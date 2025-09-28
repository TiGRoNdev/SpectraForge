/**
 * @file gaussian_optimization.cu
 * @brief CUDA kernels для оптимизации параметров 3D Gaussian Splatting
 * 
 * Реализует алгоритмы FlashGS для высокопроизводительной оптимизации
 * параметров гауссианов с использованием CUDA.
 */

#include <cuda_runtime.h>
#include <curand_kernel.h>
#include <device_launch_parameters.h>
#include <math.h>

namespace HyperEngine::CUDA {

/**
 * @brief Структура гауссиана на GPU
 */
struct GPUGaussian {
    float4 position;        // x, y, z, opacity
    float4 color;          // r, g, b, scale
    float4 rotation;       // quaternion (x, y, z, w)
    float4 covariance[2];  // 6 элементов ковариационной матрицы (3x3 symmetric)
};

/**
 * @brief Параметры оптимизации
 */
struct OptimizationParams {
    float learningRate;
    float densificationThreshold;
    float pruningThreshold;
    int maxGaussians;
    int iterationCount;
};

/**
 * @brief Kernel для инициализации гауссианов из точечного облака
 * @param points Входные точки (x, y, z, intensity)
 * @param gaussians Выходные гауссианы
 * @param numPoints Количество точек
 * @param baseScale Базовый масштаб гауссианов
 */
__global__ void initializeGaussiansFromPoints(
    const float4* points,
    GPUGaussian* gaussians,
    int numPoints,
    float baseScale
) {
    int idx = blockIdx.x * blockDim.x + threadIdx.x;
    if (idx >= numPoints) return;
    
    const float4& point = points[idx];
    GPUGaussian& gaussian = gaussians[idx];
    
    // Позиция и прозрачность
    gaussian.position = make_float4(point.x, point.y, point.z, 0.8f);
    
    // Цвет (базируется на интенсивности или случайный)
    float intensity = point.w;
    gaussian.color = make_float4(intensity, intensity, intensity, baseScale);
    
    // Единичный кватернион (нет поворота)
    gaussian.rotation = make_float4(0.0f, 0.0f, 0.0f, 1.0f);
    
    // Изотропная ковариационная матрица
    float var = baseScale * baseScale;
    gaussian.covariance[0] = make_float4(var, 0.0f, 0.0f, var);  // xx, xy, xz, yy
    gaussian.covariance[1] = make_float4(0.0f, var, 0.0f, 0.0f); // yz, zz, pad, pad
}

/**
 * @brief Kernel для вычисления градиентов позиций гауссианов
 * @param gaussians Массив гауссианов
 * @param imageTarget Целевое изображение
 * @param imageRendered Отрендеренное изображение  
 * @param gradients Выходные градиенты
 * @param width Ширина изображения
 * @param height Высота изображения
 * @param numGaussians Количество гауссианов
 */
__global__ void computePositionGradients(
    const GPUGaussian* gaussians,
    const float4* imageTarget,
    const float4* imageRendered, 
    float4* gradients,
    int width,
    int height,
    int numGaussians
) {
    int gaussianIdx = blockIdx.x * blockDim.x + threadIdx.x;
    if (gaussianIdx >= numGaussians) return;
    
    const GPUGaussian& gaussian = gaussians[gaussianIdx];
    float4 gradient = make_float4(0.0f, 0.0f, 0.0f, 0.0f);
    
    // Проекция гауссиана на экран (упрощенная)
    int centerX = (int)(gaussian.position.x * width);
    int centerY = (int)(gaussian.position.y * height);
    
    // Радиус влияния гауссиана (3 сигма)
    float scale = gaussian.color.w;
    int radius = (int)(3.0f * scale * width);
    
    // Вычисляем градиент по области влияния
    for (int dy = -radius; dy <= radius; dy++) {
        for (int dx = -radius; dx <= radius; dx++) {
            int x = centerX + dx;
            int y = centerY + dy;
            
            if (x >= 0 && x < width && y >= 0 && y < height) {
                int pixelIdx = y * width + x;
                
                // Разность между целевым и отрендеренным изображением
                float4 diff = make_float4(
                    imageTarget[pixelIdx].x - imageRendered[pixelIdx].x,
                    imageTarget[pixelIdx].y - imageRendered[pixelIdx].y,
                    imageTarget[pixelIdx].z - imageRendered[pixelIdx].z,
                    imageTarget[pixelIdx].w - imageRendered[pixelIdx].w
                );
                
                // Расстояние от центра гауссиана
                float deltaX = (float)dx / width;
                float deltaY = (float)dy / height;
                float distSq = deltaX * deltaX + deltaY * deltaY;
                
                // Гауссово влияние
                float influence = expf(-distSq / (2.0f * scale * scale));
                
                // Накапливаем градиент
                gradient.x += diff.x * influence * deltaX;
                gradient.y += diff.y * influence * deltaY;
                gradient.z += diff.z * influence;
                gradient.w += diff.w * influence;
            }
        }
    }
    
    gradients[gaussianIdx] = gradient;
}

/**
 * @brief Kernel для обновления параметров гауссианов с помощью SGD
 * @param gaussians Массив гауссианов для обновления
 * @param gradients Вычисленные градиенты
 * @param params Параметры оптимизации
 * @param numGaussians Количество гауссианов
 */
__global__ void updateGaussianParameters(
    GPUGaussian* gaussians,
    const float4* gradients,
    OptimizationParams params,
    int numGaussians
) {
    int idx = blockIdx.x * blockDim.x + threadIdx.x;
    if (idx >= numGaussians) return;
    
    GPUGaussian& gaussian = gaussians[idx];
    const float4& gradient = gradients[idx];
    
    // Обновляем позицию
    gaussian.position.x -= params.learningRate * gradient.x;
    gaussian.position.y -= params.learningRate * gradient.y;
    gaussian.position.z -= params.learningRate * gradient.z;
    
    // Обновляем прозрачность (с клампингом)
    gaussian.position.w -= params.learningRate * gradient.w;
    gaussian.position.w = fmaxf(0.0f, fminf(1.0f, gaussian.position.w));
    
    // Адаптивный контроль плотности
    float gradientMagnitude = sqrtf(gradient.x * gradient.x + 
                                  gradient.y * gradient.y + 
                                  gradient.z * gradient.z);
    
    // Увеличиваем масштаб если градиент большой (недостаточное покрытие)
    if (gradientMagnitude > params.densificationThreshold) {
        gaussian.color.w *= 1.1f; // Увеличиваем scale
    }
    
    // Уменьшаем прозрачность если гауссиан слабо влияет
    if (gradientMagnitude < params.pruningThreshold) {
        gaussian.position.w *= 0.95f; // Уменьшаем opacity
    }
}

/**
 * @brief Kernel для плотностного контроля - разделение больших гауссианов
 * @param gaussians Массив гауссианов
 * @param newGaussians Новые гауссианы от разделения
 * @param splitMask Маска гауссианов для разделения
 * @param numGaussians Количество исходных гауссианов
 * @param numNewGaussians Количество новых гауссианов
 */
__global__ void densificationSplit(
    const GPUGaussian* gaussians,
    GPUGaussian* newGaussians,
    const bool* splitMask,
    int numGaussians,
    int* numNewGaussians
) {
    int idx = blockIdx.x * blockDim.x + threadIdx.x;
    if (idx >= numGaussians || !splitMask[idx]) return;
    
    const GPUGaussian& parent = gaussians[idx];
    
    // Атомарно получаем индекс для нового гауссиана
    int newIdx = atomicAdd(numNewGaussians, 2); // Создаем 2 новых гауссиана
    
    if (newIdx + 1 < numGaussians) { // Проверяем границы
        // Создаем два новых гауссиана около родительского
        float offset = parent.color.w * 0.3f; // 30% от масштаба
        
        // Первый дочерний
        newGaussians[newIdx] = parent;
        newGaussians[newIdx].position.x -= offset;
        newGaussians[newIdx].color.w *= 0.7f; // Уменьшаем масштаб
        
        // Второй дочерний
        newGaussians[newIdx + 1] = parent;
        newGaussians[newIdx + 1].position.x += offset;
        newGaussians[newIdx + 1].color.w *= 0.7f;
    }
}

/**
 * @brief Kernel для pruning - удаление слабых гауссианов
 * @param gaussians Массив гауссианов
 * @param validMask Маска валидных гауссианов после pruning
 * @param numGaussians Количество гауссианов
 * @param minOpacity Минимальная прозрачность для сохранения
 */
__global__ void pruneWeakGaussians(
    const GPUGaussian* gaussians,
    bool* validMask,
    int numGaussians,
    float minOpacity
) {
    int idx = blockIdx.x * blockDim.x + threadIdx.x;
    if (idx >= numGaussians) return;
    
    const GPUGaussian& gaussian = gaussians[idx];
    
    // Помечаем гауссиан как валидный если его прозрачность достаточна
    validMask[idx] = (gaussian.position.w >= minOpacity);
}

/**
 * @brief Host функция для запуска оптимизации гауссианов
 */
extern "C" {
    
    /**
     * @brief Инициализация гауссианов из точечного облака
     */
    cudaError_t launchGaussianInitialization(
        const float4* d_points,
        HyperEngine::CUDA::GPUGaussian* d_gaussians,
        int numPoints,
        float baseScale,
        cudaStream_t stream = 0
    ) {
        dim3 blockSize(256);
        dim3 gridSize((numPoints + blockSize.x - 1) / blockSize.x);
        
        initializeGaussiansFromPoints<<<gridSize, blockSize, 0, stream>>>(
            d_points, d_gaussians, numPoints, baseScale
        );
        
        return cudaGetLastError();
    }
    
    /**
     * @brief Вычисление градиентов
     */
    cudaError_t launchGradientComputation(
        const HyperEngine::CUDA::GPUGaussian* d_gaussians,
        const float4* d_imageTarget,
        const float4* d_imageRendered,
        float4* d_gradients,
        int width,
        int height,
        int numGaussians,
        cudaStream_t stream = 0
    ) {
        dim3 blockSize(256);
        dim3 gridSize((numGaussians + blockSize.x - 1) / blockSize.x);
        
        computePositionGradients<<<gridSize, blockSize, 0, stream>>>(
            d_gaussians, d_imageTarget, d_imageRendered, d_gradients,
            width, height, numGaussians
        );
        
        return cudaGetLastError();
    }
    
    /**
     * @brief Обновление параметров гауссианов
     */
    cudaError_t launchParameterUpdate(
        HyperEngine::CUDA::GPUGaussian* d_gaussians,
        const float4* d_gradients,
        HyperEngine::CUDA::OptimizationParams params,
        int numGaussians,
        cudaStream_t stream = 0
    ) {
        dim3 blockSize(256);
        dim3 gridSize((numGaussians + blockSize.x - 1) / blockSize.x);
        
        updateGaussianParameters<<<gridSize, blockSize, 0, stream>>>(
            d_gaussians, d_gradients, params, numGaussians
        );
        
        return cudaGetLastError();
    }
}

} // namespace HyperEngine::CUDA
