/**
 * @file tile_rasterization.cu
 * @brief CUDA kernels для tile-based растеризации гауссианов
 *
 * Реализует высокопроизводительную растеризацию 3D Gaussian Splatting
 * с использованием тайлового подхода для эффективного рендеринга.
 */

#include <cooperative_groups.h>
#include <cuda_runtime.h>
#include <device_launch_parameters.h>
#include <math.h>
#include "HyperEngine/CUDA/FlashGSSplatter.h"

namespace Engine3D::CUDA {

/**
 * @brief Параметры камеры для проекции
 */
/**
 * @brief Структура гауссиана на GPU
 */
struct GPUGaussian {
    float4 position;  // x, y, z, opacity
    float4 color;     // r, g, b, scale
    float4 rotation;  // quaternion (x, y, z, w)
    float4 covariance[2];  // 6 элементов ковариационной матрицы (3x3 symmetric)
};

/**
 * @brief Параметры камеры для проекции
 */
struct CameraMatrix {
    float viewMatrix[16];       // View matrix (4x4)
    float projMatrix[16];       // Projection matrix (4x4)
    float viewProjMatrix[16];   // View-Projection matrix (4x4)
    int width, height;          // Разрешение
    float nearPlane, farPlane;  // Near/far planes
};

/**
 * @brief Проецированный гауссиан на экране
 */
struct ProjectedGaussian {
    float2 center;        // Центр на экране (пиксели)
    float2 axis1, axis2;  // Главные оси эллипса
    float depth;          // Глубина для сортировки
    float4 color;         // Цвет и прозрачность
    float radius;         // Радиус влияния (пиксели)
    uint32_t gaussianID;  // ID исходного гауссиана
};

/**
 * @brief Тайл для растеризации
 */
struct Tile {
    uint32_t gaussianCount;     // Количество гауссианов в тайле
    uint32_t gaussianIDs[256];  // ID гауссианов (максимум 256 на тайл)
};

/**
 * @brief Константы тайловой растеризации
 */
constexpr int TILE_SIZE = 16;                // Размер тайла в пикселях
constexpr int MAX_GAUSSIANS_PER_TILE = 256;  // Максимум гауссианов на тайл
constexpr int SHARED_MEMORY_SIZE = 1024;     // Размер shared memory

/**
 * @brief Kernel для проекции 3D гауссианов в 2D
 * @param gaussians Входные 3D гауссианы
 * @param camera Параметры камеры
 * @param projected Выходные проецированные гауссианы
 * @param numGaussians Количество гауссианов
 */
__global__ void projectGaussians(const GPUGaussian* gaussians,
                                 CameraMatrix camera,
                                 ProjectedGaussian* projected,
                                 int numGaussians) {
    int idx = blockIdx.x * blockDim.x + threadIdx.x;
    if (idx >= numGaussians)
        return;

    const GPUGaussian& gaussian = gaussians[idx];
    ProjectedGaussian& proj = projected[idx];

    // Применяем view-projection matrix к позиции
    float4 pos = gaussian.position;
    float4 projPos;

    // Умножение на view-projection matrix (упрощенная версия)
    projPos.x = camera.viewProjMatrix[0] * pos.x + camera.viewProjMatrix[4] * pos.y
                + camera.viewProjMatrix[8] * pos.z + camera.viewProjMatrix[12];
    projPos.y = camera.viewProjMatrix[1] * pos.x + camera.viewProjMatrix[5] * pos.y
                + camera.viewProjMatrix[9] * pos.z + camera.viewProjMatrix[13];
    projPos.z = camera.viewProjMatrix[2] * pos.x + camera.viewProjMatrix[6] * pos.y
                + camera.viewProjMatrix[10] * pos.z + camera.viewProjMatrix[14];
    projPos.w = camera.viewProjMatrix[3] * pos.x + camera.viewProjMatrix[7] * pos.y
                + camera.viewProjMatrix[11] * pos.z + camera.viewProjMatrix[15];

    // Проверка на clipping
    if (projPos.w <= 0.0f) {
        proj.radius = -1.0f;  // Маркер невалидного гауссиана
        return;
    }

    // Perspective divide
    float invW = 1.0f / projPos.w;
    float ndcX = projPos.x * invW;
    float ndcY = projPos.y * invW;

    // Преобразование в экранные координаты
    proj.center.x = (ndcX * 0.5f + 0.5f) * camera.width;
    proj.center.y = (1.0f - (ndcY * 0.5f + 0.5f)) * camera.height;
    proj.depth = projPos.z * invW;

    // Вычисляем проекцию ковариационной матрицы на экран
    float scale = gaussian.color.w;

    // Упрощенная проекция - изотропный гауссиан
    float screenScale = scale * camera.width / projPos.w;
    proj.axis1 = make_float2(screenScale, 0.0f);
    proj.axis2 = make_float2(0.0f, screenScale);
    proj.radius = 3.0f * screenScale;  // 3 сигма

    // Копируем цвет и прозрачность
    proj.color = gaussian.color;
    proj.color.w = gaussian.position.w;  // opacity
    proj.gaussianID = idx;
}

/**
 * @brief Kernel для определения принадлежности гауссианов к тайлам
 * @param projected Проецированные гауссианы
 * @param tileAssignments Выходные назначения тайлов
 * @param numGaussians Количество гауссианов
 * @param screenWidth Ширина экрана
 * @param screenHeight Высота экрана
 */
__global__ void assignGaussiansToTiles(const ProjectedGaussian* projected,
                                       uint32_t* tileAssignments,
                                       int numGaussians,
                                       int screenWidth,
                                       int screenHeight) {
    int idx = blockIdx.x * blockDim.x + threadIdx.x;
    if (idx >= numGaussians)
        return;

    const ProjectedGaussian& proj = projected[idx];

    // Пропускаем невалидные гауссианы
    if (proj.radius < 0.0f) {
        tileAssignments[idx] = 0xFFFFFFFF;  // Невалидный тайл
        return;
    }

    // Вычисляем границы влияния гауссиана
    int minTileX = max(0, (int)((proj.center.x - proj.radius) / TILE_SIZE));
    int maxTileX = min((screenWidth + TILE_SIZE - 1) / TILE_SIZE - 1,
                       (int)((proj.center.x + proj.radius) / TILE_SIZE));
    int minTileY = max(0, (int)((proj.center.y - proj.radius) / TILE_SIZE));
    int maxTileY = min((screenHeight + TILE_SIZE - 1) / TILE_SIZE - 1,
                       (int)((proj.center.y + proj.radius) / TILE_SIZE));

    // Записываем все тайлы, которые покрывает гауссиан
    for (int ty = minTileY; ty <= maxTileY; ty++) {
        for (int tx = minTileX; tx <= maxTileX; tx++) {
            int tileID = ty * ((screenWidth + TILE_SIZE - 1) / TILE_SIZE) + tx;

            // В реальной реализации здесь нужен атомарный append в список тайла
            // Для упрощения записываем только один тайл
            if (tx == minTileX && ty == minTileY) {
                tileAssignments[idx] = tileID;
            }
        }
    }
}

/**
 * @brief Kernel для сортировки гауссианов по глубине внутри тайлов
 * @param projected Проецированные гауссианы
 * @param sortedIndices Отсортированные индексы гауссианов
 * @param tileOffsets Смещения для каждого тайла
 * @param numTiles Количество тайлов
 */
__global__ void sortGaussiansByDepth(const ProjectedGaussian* projected,
                                     uint32_t* sortedIndices,
                                     uint32_t* tileOffsets,
                                     int numTiles) {
    int tileID = blockIdx.x;
    if (tileID >= numTiles)
        return;

    // Получаем диапазон гауссианов для этого тайла
    uint32_t start = tileOffsets[tileID];
    uint32_t end = (tileID + 1 < numTiles) ? tileOffsets[tileID + 1] : start;
    uint32_t count = end - start;

    if (count == 0)
        return;

    // Простая сортировка вставками в shared memory
    extern __shared__ uint32_t sharedIndices[];

    // Загружаем индексы в shared memory
    for (int i = threadIdx.x; i < count; i += blockDim.x) {
        if (start + i < end) {
            sharedIndices[i] = sortedIndices[start + i];
        }
    }
    __syncthreads();

    // Сортировка по глубине (insertion sort для малых массивов)
    if (threadIdx.x == 0) {
        for (int i = 1; i < count; i++) {
            uint32_t key = sharedIndices[i];
            float keyDepth = projected[key].depth;

            int j = i - 1;
            while (j >= 0 && projected[sharedIndices[j]].depth > keyDepth) {
                sharedIndices[j + 1] = sharedIndices[j];
                j--;
            }
            sharedIndices[j + 1] = key;
        }
    }
    __syncthreads();

    // Записываем отсортированный массив обратно
    for (int i = threadIdx.x; i < count; i += blockDim.x) {
        if (start + i < end) {
            sortedIndices[start + i] = sharedIndices[i];
        }
    }
}

/**
 * @brief Kernel для растеризации тайлов
 * @param projected Проецированные гауссианы
 * @param sortedIndices Отсортированные индексы
 * @param tileOffsets Смещения тайлов
 * @param framebuffer Выходной framebuffer (RGBA)
 * @param depthBuffer Буфер глубины
 * @param screenWidth Ширина экрана
 * @param screenHeight Высота экрана
 */
__global__ void rasterizeTiles(const ProjectedGaussian* projected,
                               const uint32_t* sortedIndices,
                               const uint32_t* tileOffsets,
                               float4* framebuffer,
                               float* depthBuffer,
                               int screenWidth,
                               int screenHeight) {
    // Определяем тайл и пиксель внутри тайла
    int tileX = blockIdx.x;
    int tileY = blockIdx.y;
    int pixelX = threadIdx.x;
    int pixelY = threadIdx.y;

    // Проверяем границы
    int globalX = tileX * TILE_SIZE + pixelX;
    int globalY = tileY * TILE_SIZE + pixelY;

    if (globalX >= screenWidth || globalY >= screenHeight)
        return;

    int pixelIdx = globalY * screenWidth + globalX;
    int tileID = tileY * ((screenWidth + TILE_SIZE - 1) / TILE_SIZE) + tileX;

    // Получаем список гауссианов для этого тайла
    uint32_t start = tileOffsets[tileID];
    uint32_t end = tileOffsets[tileID + 1];

    // Инициализируем цвет пикселя
    float4 pixelColor = make_float4(0.0f, 0.0f, 0.0f, 0.0f);
    float transmittance = 1.0f;

    // Позиция пикселя на экране
    float2 pixelPos = make_float2(globalX + 0.5f, globalY + 0.5f);

    // Проходим по всем гауссианам в тайле (от близких к дальним)
    for (uint32_t i = start; i < end && transmittance > 0.001f; i++) {
        uint32_t gaussianIdx = sortedIndices[i];
        const ProjectedGaussian& proj = projected[gaussianIdx];

        // Вычисляем расстояние от центра гауссиана
        float2 delta = make_float2(pixelPos.x - proj.center.x, pixelPos.y - proj.center.y);

        // Вычисляем влияние гауссиана в этой точке
        float distSq = delta.x * delta.x + delta.y * delta.y;
        float radiusSq = proj.radius * proj.radius;

        if (distSq > radiusSq)
            continue;  // Пиксель вне влияния

        // Гауссово затухание
        float sigma = proj.radius / 3.0f;  // 3-sigma rule
        float alpha = proj.color.w * expf(-distSq / (2.0f * sigma * sigma));

        if (alpha < 0.001f)
            continue;  // Слишком слабое влияние

        // Alpha blending
        float weight = alpha * transmittance;
        pixelColor.x += proj.color.x * weight;
        pixelColor.y += proj.color.y * weight;
        pixelColor.z += proj.color.z * weight;
        pixelColor.w += weight;

        // Обновляем прозрачность
        transmittance *= (1.0f - alpha);
    }

    // Записываем финальный цвет
    framebuffer[pixelIdx] = pixelColor;
    depthBuffer[pixelIdx] = 1.0f - transmittance;  // Depth as accumulated alpha
}

/**
 * @brief Host функции для запуска tile rasterization kernels
 */
extern "C" {
/**
 * @brief Проекция гауссианов на экран
 */
cudaError_t launchGaussianProjection(const GPUGaussian* d_gaussians,
                                     CameraMatrix camera,
                                     ProjectedGaussian* d_projected,
                                     int numGaussians,
                                     cudaStream_t stream = 0) {
    dim3 blockSize(256);
    dim3 gridSize((numGaussians + blockSize.x - 1) / blockSize.x);

    projectGaussians<<<gridSize, blockSize, 0, stream>>>(
        d_gaussians, camera, d_projected, numGaussians);

    return cudaGetLastError();
}

/**
 * @brief Назначение гауссианов в тайлы
 */
cudaError_t launchTileAssignment(const ProjectedGaussian* d_projected,
                                 uint32_t* d_tileAssignments,
                                 int numGaussians,
                                 int screenWidth,
                                 int screenHeight,
                                 cudaStream_t stream = 0) {
    dim3 blockSize(256);
    dim3 gridSize((numGaussians + blockSize.x - 1) / blockSize.x);

    assignGaussiansToTiles<<<gridSize, blockSize, 0, stream>>>(
        d_projected, d_tileAssignments, numGaussians, screenWidth, screenHeight);

    return cudaGetLastError();
}

/**
 * @brief Tile-based растеризация
 */
cudaError_t launchTileRasterization(const ProjectedGaussian* d_projected,
                                    const uint32_t* d_sortedIndices,
                                    const uint32_t* d_tileOffsets,
                                    float4* d_framebuffer,
                                    float* d_depthBuffer,
                                    int screenWidth,
                                    int screenHeight,
                                    cudaStream_t stream = 0) {
    dim3 blockSize(TILE_SIZE, TILE_SIZE);
    dim3 gridSize((screenWidth + TILE_SIZE - 1) / TILE_SIZE,
                  (screenHeight + TILE_SIZE - 1) / TILE_SIZE);

    rasterizeTiles<<<gridSize, blockSize, 0, stream>>>(d_projected,
                                                       d_sortedIndices,
                                                       d_tileOffsets,
                                                       d_framebuffer,
                                                       d_depthBuffer,
                                                       screenWidth,
                                                       screenHeight);

    return cudaGetLastError();
}

}  // extern "C"

// Временная заглушка удалена - теперь используется реальная CUB сортировка из depth_sorting.cu

}  // namespace Engine3D::CUDA
