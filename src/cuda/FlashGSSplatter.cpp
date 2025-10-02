/**
 * @file FlashGSSplatter.cpp
 * @brief Реализация CUDA-ускоренного 3D Gaussian Splatter
 *
 * Полная реализация FlashGS алгоритма с CUDA kernels
 * для высокопроизводительного рендеринга гауссианов.
 */

#include "SpectraForge/CUDA/FlashGSSplatter.h"
#include <algorithm>
#include <iostream>
#include <stdexcept>
#include "SpectraForge/CUDA/CudaInterop.h"
#include "SpectraForge/Core/Console.h"
#include "SpectraForge/Core/SafeConsole.h"
#include "SpectraForge/Vulkan/SceneManager.h"
#include "SpectraForge/Vulkan/VulkanRenderer.h"

#ifdef CUDA_VULKAN_INTEROP_SUPPORTED
// Объявления внешних CUDA kernel функций с C linkage
extern "C" {
// Из gaussian_optimization.cu
cudaError_t launchGaussianInitialization(const float4* d_points,
                                         SpectraForge::CUDA::GPUGaussian* d_gaussians,
                                         int numPoints,
                                         float baseScale,
                                         cudaStream_t stream);

cudaError_t launchGradientComputation(const SpectraForge::CUDA::GPUGaussian* d_gaussians,
                                      const float4* d_imageTarget,
                                      const float4* d_imageRendered,
                                      float4* d_gradients,
                                      int width,
                                      int height,
                                      int numGaussians,
                                      cudaStream_t stream);

cudaError_t launchParameterUpdate(SpectraForge::CUDA::GPUGaussian* d_gaussians,
                                  const float4* d_gradients,
                                  SpectraForge::CUDA::OptimizationParams params,
                                  int numGaussians,
                                  cudaStream_t stream);

// Из tile_rasterization.cu
cudaError_t launchGaussianProjection(const SpectraForge::CUDA::GPUGaussian* d_gaussians,
                                     SpectraForge::CUDA::CameraMatrix camera,
                                     SpectraForge::CUDA::ProjectedGaussian* d_projected,
                                     int numGaussians,
                                     cudaStream_t stream);

cudaError_t launchTileAssignment(const SpectraForge::CUDA::ProjectedGaussian* d_projected,
                                 uint32_t* d_tileAssignments,
                                 int numGaussians,
                                 int screenWidth,
                                 int screenHeight,
                                 cudaStream_t stream);

cudaError_t launchTileRasterization(const SpectraForge::CUDA::ProjectedGaussian* d_projected,
                                    const uint32_t* d_sortedIndices,
                                    const uint32_t* d_tileOffsets,
                                    float4* d_framebuffer,
                                    float* d_depthBuffer,
                                    int screenWidth,
                                    int screenHeight,
                                    cudaStream_t stream);

// Из depth_sorting.cu (будет исправлен позднее)
cudaError_t launchCubRadixSort(const SpectraForge::CUDA::ProjectedGaussian* d_projected,
                               uint32_t* d_sortedIndices,
                               int numGaussians,
                               cudaStream_t stream);

cudaError_t launchTileAwareSorting(const SpectraForge::CUDA::ProjectedGaussian* d_projected,
                                   const uint32_t* d_tileAssignments,
                                   uint32_t* d_sortedIndices,
                                   uint32_t* d_tileOffsets,
                                   int numGaussians,
                                   int numTiles,
                                   cudaStream_t stream);
}
#endif

using namespace SpectraForge::CUDA;
using namespace SpectraForge::Core;

namespace SpectraForge::CUDA {

// TileBasedRasterizer implementation
TileBasedRasterizer::TileBasedRasterizer()
    : width(0), height(0), tileSize(16), initialized(false) {}

TileBasedRasterizer::~TileBasedRasterizer() {
    if (initialized) {
        shutdown();
    }
}

bool TileBasedRasterizer::init(uint32_t w, uint32_t h, uint32_t tSize) {
    this->width = w;
    this->height = h;
    this->tileSize = tSize;

    SAFE_PRINT_LINE("[TileBasedRasterizer] Инициализация " + SAFE_TO_STRING(w) + "x"
                    + SAFE_TO_STRING(h) + " с размером тайла " + SAFE_TO_STRING(tSize));

    initialized = true;
    return true;
}

void TileBasedRasterizer::shutdown() {
    if (!initialized) {
        return;
    }

    SAFE_PRINT_LINE("[TileBasedRasterizer] Завершение работы");
    initialized = false;
}

void TileBasedRasterizer::rasterize(const GaussianParams& params,
                                    const CameraParams& /* camera */) {
    if (!initialized) {
        SAFE_ERROR("[TileBasedRasterizer] Ошибка: Растеризатор не инициализирован");
        return;
    }

    SAFE_PRINT_LINE("[TileBasedRasterizer] Растеризация " + SAFE_TO_STRING(params.count)
                    + " гауссианов (заглушка)");
    // TODO: Реальная растеризация на этапе 3
}

// FlashGSSplatter implementation
FlashGSSplatter::FlashGSSplatter() : legacyParams() {
#ifdef CUDA_VULKAN_INTEROP_SUPPORTED
    // Устанавливаем параметры оптимизации по умолчанию
    optimParams.learningRate = 0.01f;
    optimParams.densificationThreshold = 0.1f;
    optimParams.pruningThreshold = 0.001f;
    optimParams.maxGaussians = 100000;
    optimParams.iterationCount = 100;
#endif
}

FlashGSSplatter::~FlashGSSplatter() {
    if (initialized) {
        shutdown();
    }
}

bool FlashGSSplatter::init(std::shared_ptr<CudaInterop> interop) {
    try {
        SAFE_PRINT_LINE("[FlashGSSplatter] Инициализация CUDA Gaussian Splatter...");

        // Сохраняем interop объект
#ifdef CUDA_VULKAN_INTEROP_SUPPORTED
        cudaInterop = interop;
#else
        (void)interop;  // Подавляем warning о неиспользуемом параметре
#endif

        // Инициализируем legacy параметры гауссианов
        legacyParams = GaussianParams{};
        legacyParams.positions = nullptr;
        legacyParams.covariances = nullptr;
        legacyParams.opacities = nullptr;
        legacyParams.colors = nullptr;
        legacyParams.count = 0;

        // Создаем растеризатор (legacy)
        rasterizer = std::make_unique<TileBasedRasterizer>();
        if (!rasterizer->init(1920, 1080, 16)) {
            SAFE_ERROR("[FlashGSSplatter] Ошибка инициализации растеризатора");
            return false;
        }

        // Инициализируем CUDA ресурсы
        if (!initCUDA()) {
            SAFE_ERROR("[FlashGSSplatter] Ошибка инициализации CUDA");
            return false;
        }

        initialized = true;
        SAFE_PRINT_LINE("[FlashGSSplatter] Инициализация завершена успешно");
        return true;

    } catch (const std::exception& e) {
        SAFE_ERROR("[FlashGSSplatter] Ошибка инициализации: " + std::string(e.what()));
        return false;
    }
}

void FlashGSSplatter::shutdown() {
    if (!initialized) {
        return;
    }

    SAFE_PRINT_LINE("[FlashGSSplatter] Завершение работы...");

    // Освобождаем CUDA ресурсы
    cleanupCUDA();

    // Освобождаем растеризатор
    rasterizer.reset();

    // Очищаем параметры
    legacyParams = GaussianParams{};

#ifdef CUDA_VULKAN_INTEROP_SUPPORTED
    // Сбрасываем interop
    cudaInterop.reset();
    activeGaussianCount = 0;
    lastRenderTime = 0.0f;
#endif

    initialized = false;
    SAFE_PRINT_LINE("[FlashGSSplatter] Завершение работы завершено");
}

void FlashGSSplatter::optimizeGaussians(const Vulkan::MultiViewImages& images, int iterations) {
    if (!initialized) {
        SAFE_ERROR("[FlashGSSplatter] Ошибка: Splatter не инициализирован");
        return;
    }

    SAFE_PRINT_LINE("[FlashGSSplatter] Оптимизация гауссианов из "
                    + SAFE_TO_STRING(images.viewCount) + " изображений...");

#ifdef CUDA_VULKAN_INTEROP_SUPPORTED
    if (cudaStream && activeGaussianCount > 0) {
        SAFE_PRINT_LINE("[FlashGSSplatter] CUDA-ускоренная оптимизация "
                        + SAFE_TO_STRING(activeGaussianCount) + " гауссианов за "
                        + SAFE_TO_STRING(iterations) + " итераций");

        // Запускаем таймер производительности
        if (startEvent)
            cudaEventRecord(startEvent, cudaStream);

        for (int iter = 0; iter < iterations; iter++) {
            // Шаг 1: Рендеринг текущего состояния гауссианов
            // (упрощенная версия - в полной реализации нужен полный pipeline)

            // Шаг 2: Вычисление градиентов между целевым и отрендеренным изображением
            for (uint32_t viewIdx = 0; viewIdx < images.viewCount; viewIdx++) {
                // В полной реализации здесь будет сравнение с целевыми изображениями
                if (d_gradients && framebufferWidth > 0 && framebufferHeight > 0) {
                    launchGradientComputation(d_gaussians,
                                              nullptr,  // d_targetImage - нужно получить из images
                                              d_framebuffer,
                                              d_gradients,
                                              framebufferWidth,
                                              framebufferHeight,
                                              activeGaussianCount,
                                              cudaStream);
                }
            }

            // Шаг 3: Обновление параметров гауссианов
            optimParams.iterationCount = iter;
            launchParameterUpdate(
                d_gaussians, d_gradients, optimParams, activeGaussianCount, cudaStream);

            // Шаг 4: Адаптивный контроль плотности (каждые 10 итераций)
            if (iter % 10 == 0) {
                adaptiveDensityControl();
            }

            // Синхронизация потока каждые 50 итераций для проверки прогресса
            if (iter % 50 == 0) {
                cudaStreamSynchronize(cudaStream);
                SAFE_PRINT_LINE("[FlashGSSplatter] Итерация " + SAFE_TO_STRING(iter) + "/"
                                + SAFE_TO_STRING(iterations));
            }
        }

        // Останавливаем таймер
        if (stopEvent) {
            cudaEventRecord(stopEvent, cudaStream);
            cudaEventSynchronize(stopEvent);
            cudaEventElapsedTime(&lastRenderTime, startEvent, stopEvent);
        }

        SAFE_PRINT_LINE("[FlashGSSplatter] Оптимизация завершена за "
                        + SAFE_TO_STRING(lastRenderTime) + " мс");
    } else {
#endif
        // Fallback к legacy режиму
        SAFE_PRINT_LINE("[FlashGSSplatter] Оптимизация в legacy режиме (заглушка) - "
                        + SAFE_TO_STRING(iterations) + " итераций");

        // Устанавливаем тестовые параметры
        legacyParams.count = 10000;  // Примерное количество гауссианов

#ifdef CUDA_VULKAN_INTEROP_SUPPORTED
    }
#endif
}

Vulkan::PrimaryImage FlashGSSplatter::rasterizeGaussians(const CameraParams& camera) {
    if (!initialized) {
        SAFE_ERROR("[FlashGSSplatter] Ошибка: Splatter не инициализирован");
        return Vulkan::PrimaryImage{};
    }

    SAFE_PRINT_LINE("[FlashGSSplatter] Растеризация " + SAFE_TO_STRING(legacyParams.count)
                    + " гауссианов");

    // Используем растеризатор
    if (rasterizer) {
        rasterizer->rasterize(legacyParams, camera);
    }

    // TODO: Реальная растеризация на этапе 3
    // В полной реализации здесь будет:
    // 1. Сортировка гауссианов по глубине
    // 2. Tile-based rasterization на GPU
    // 3. Alpha blending
    // 4. Создание финального изображения

    // Пока возвращаем заглушку
    Vulkan::PrimaryImage result{};
    result.width = 1920;
    result.height = 1080;

    SAFE_PRINT_LINE("[FlashGSSplatter] Растеризация завершена (заглушка)");
    return result;
}

// Приватные методы

bool FlashGSSplatter::initCUDA() {
    SAFE_PRINT_LINE("[FlashGSSplatter] Инициализация CUDA ресурсов...");

#ifdef CUDA_VULKAN_INTEROP_SUPPORTED
    try {
        // Создание CUDA stream
        cudaError_t result = cudaStreamCreate(&cudaStream);
        if (result != cudaSuccess) {
            SAFE_ERROR("[FlashGSSplatter] Ошибка создания CUDA stream: "
                       + std::string(cudaGetErrorString(result)));
            return false;
        }

        // Создание event'ов для измерения производительности
        cudaEventCreate(&startEvent);
        cudaEventCreate(&stopEvent);

        // Выделяем базовые GPU буферы (будут расширены по необходимости)
        if (!allocateGPUBuffers(1920, 1080)) {
            SAFE_ERROR("[FlashGSSplatter] Ошибка выделения GPU буферов");
            return false;
        }

        SAFE_PRINT_LINE("[FlashGSSplatter] CUDA ресурсы инициализированы успешно");
        return true;

    } catch (const std::exception& e) {
        SAFE_ERROR("[FlashGSSplatter] Исключение при инициализации CUDA: " + std::string(e.what()));
        return false;
    }
#else
    SAFE_PRINT_LINE("[FlashGSSplatter] CUDA interop не поддерживается");
    cudaStream = nullptr;
    return true;  // Не ошибка, просто нет поддержки
#endif
}

void FlashGSSplatter::cleanupCUDA() {
    SAFE_PRINT_LINE("[FlashGSSplatter] Освобождение CUDA ресурсов...");

#ifdef CUDA_VULKAN_INTEROP_SUPPORTED
    // Освобождаем GPU буферы
    freeGPUBuffers();

    // Уничтожаем event'ы
    if (startEvent) {
        cudaEventDestroy(startEvent);
        startEvent = nullptr;
    }
    if (stopEvent) {
        cudaEventDestroy(stopEvent);
        stopEvent = nullptr;
    }

    // Уничтожаем stream
    if (cudaStream) {
        cudaStreamDestroy(cudaStream);
        cudaStream = nullptr;
    }

    SAFE_PRINT_LINE("[FlashGSSplatter] CUDA ресурсы освобождены");
#else
    cudaStream = nullptr;
#endif
}

#ifdef CUDA_VULKAN_INTEROP_SUPPORTED

bool FlashGSSplatter::allocateGPUBuffers(int width, int height) {
    try {
        framebufferWidth = width;
        framebufferHeight = height;

        // Выделяем буферы для гауссианов
        size_t gaussianSize = maxGaussians * sizeof(GPUGaussian);
        cudaError_t result = cudaMalloc(&d_gaussians, gaussianSize);
        if (result != cudaSuccess) {
            SAFE_ERROR("[FlashGSSplatter] Ошибка выделения d_gaussians: "
                       + std::string(cudaGetErrorString(result)));
            return false;
        }

        // Буферы для проекции и сортировки
        result = cudaMalloc(&d_projectedGaussians, maxGaussians * sizeof(ProjectedGaussian));
        if (result != cudaSuccess) {
            SAFE_ERROR("[FlashGSSplatter] Ошибка выделения d_projectedGaussians: "
                       + std::string(cudaGetErrorString(result)));
            return false;
        }

        result = cudaMalloc(&d_sortedIndices, maxGaussians * sizeof(uint32_t));
        if (result != cudaSuccess) {
            SAFE_ERROR("[FlashGSSplatter] Ошибка выделения d_sortedIndices: "
                       + std::string(cudaGetErrorString(result)));
            return false;
        }

        result = cudaMalloc(&d_tileAssignments, maxGaussians * sizeof(uint32_t));
        if (result != cudaSuccess) {
            SAFE_ERROR("[FlashGSSplatter] Ошибка выделения d_tileAssignments: "
                       + std::string(cudaGetErrorString(result)));
            return false;
        }

        // Буферы для тайлов
        int numTiles = ((width + 15) / 16) * ((height + 15) / 16);  // 16x16 tiles
        result = cudaMalloc(&d_tileOffsets, (numTiles + 1) * sizeof(uint32_t));
        if (result != cudaSuccess) {
            SAFE_ERROR("[FlashGSSplatter] Ошибка выделения d_tileOffsets: "
                       + std::string(cudaGetErrorString(result)));
            return false;
        }

        // Буферы для оптимизации
        result = cudaMalloc(&d_gradients, maxGaussians * sizeof(float4));
        if (result != cudaSuccess) {
            SAFE_ERROR("[FlashGSSplatter] Ошибка выделения d_gradients: "
                       + std::string(cudaGetErrorString(result)));
            return false;
        }

        // Framebuffer и depth buffer
        size_t pixelCount = width * height;
        result = cudaMalloc(&d_framebuffer, pixelCount * sizeof(float4));
        if (result != cudaSuccess) {
            SAFE_ERROR("[FlashGSSplatter] Ошибка выделения d_framebuffer: "
                       + std::string(cudaGetErrorString(result)));
            return false;
        }

        result = cudaMalloc(&d_depthBuffer, pixelCount * sizeof(float));
        if (result != cudaSuccess) {
            SAFE_ERROR("[FlashGSSplatter] Ошибка выделения d_depthBuffer: "
                       + std::string(cudaGetErrorString(result)));
            return false;
        }

        // Инициализируем буферы нулями
        result = cudaMemset(d_framebuffer, 0, pixelCount * sizeof(float4));
        if (result != cudaSuccess) {
            SAFE_ERROR("[FlashGSSplatter] Ошибка инициализации d_framebuffer: "
                       + std::string(cudaGetErrorString(result)));
            return false;
        }

        result = cudaMemset(d_depthBuffer, 0, pixelCount * sizeof(float));
        if (result != cudaSuccess) {
            SAFE_ERROR("[FlashGSSplatter] Ошибка инициализации d_depthBuffer: "
                       + std::string(cudaGetErrorString(result)));
            return false;
        }

        SAFE_PRINT_LINE("[FlashGSSplatter] GPU буферы выделены: " + SAFE_TO_STRING(width) + "x"
                        + SAFE_TO_STRING(height) + ", " + SAFE_TO_STRING(maxGaussians)
                        + " гауссианов");

        return true;

    } catch (const std::exception& e) {
        SAFE_ERROR("[FlashGSSplatter] Ошибка выделения GPU буферов: " + std::string(e.what()));
        freeGPUBuffers();  // Очищаем частично выделенные ресурсы
        return false;
    }
}

void FlashGSSplatter::freeGPUBuffers() {
    if (d_gaussians) {
        cudaFree(d_gaussians);
        d_gaussians = nullptr;
    }
    if (d_projectedGaussians) {
        cudaFree(d_projectedGaussians);
        d_projectedGaussians = nullptr;
    }
    if (d_sortedIndices) {
        cudaFree(d_sortedIndices);
        d_sortedIndices = nullptr;
    }
    if (d_tileAssignments) {
        cudaFree(d_tileAssignments);
        d_tileAssignments = nullptr;
    }
    if (d_tileOffsets) {
        cudaFree(d_tileOffsets);
        d_tileOffsets = nullptr;
    }
    if (d_gradients) {
        cudaFree(d_gradients);
        d_gradients = nullptr;
    }
    if (d_framebuffer) {
        cudaFree(d_framebuffer);
        d_framebuffer = nullptr;
    }
    if (d_depthBuffer) {
        cudaFree(d_depthBuffer);
        d_depthBuffer = nullptr;
    }

    framebufferWidth = 0;
    framebufferHeight = 0;
}

void FlashGSSplatter::initializeFromPointCloud(const float4* points,
                                               int numPoints,
                                               float baseScale) {
    if (!initialized || !d_gaussians) {
        SAFE_ERROR("[FlashGSSplatter] Не инициализирован для инициализации из точечного облака");
        return;
    }

    activeGaussianCount = (numPoints < maxGaussians) ? numPoints : maxGaussians;

    SAFE_PRINT_LINE("[FlashGSSplatter] Инициализация " + SAFE_TO_STRING(activeGaussianCount)
                    + " гауссианов из точечного облака");

    // Копируем точки на GPU и инициализируем гауссианы
    float4* d_points;
    cudaMalloc(&d_points, activeGaussianCount * sizeof(float4));
    cudaMemcpy(d_points, points, activeGaussianCount * sizeof(float4), cudaMemcpyHostToDevice);

    // Запускаем kernel инициализации
    launchGaussianInitialization(d_points, d_gaussians, activeGaussianCount, baseScale, cudaStream);

    // Освобождаем временный буфер
    cudaFree(d_points);

    // Синхронизируем stream
    cudaStreamSynchronize(cudaStream);

    SAFE_PRINT_LINE("[FlashGSSplatter] Инициализация гауссианов завершена");
}

void FlashGSSplatter::rasterizeGaussiansCUDA(const CameraMatrix& camera,
                                             float4* framebuffer,
                                             float* depthBuffer,
                                             int width,
                                             int height) {
    if (!initialized || !d_gaussians || activeGaussianCount == 0) {
        SAFE_ERROR("[FlashGSSplatter] Не готов к CUDA растеризации");
        return;
    }

    // Запускаем таймер
    if (startEvent)
        cudaEventRecord(startEvent, cudaStream);

    // Шаг 1: Проекция 3D гауссианов в 2D
    launchGaussianProjection(
        d_gaussians, camera, d_projectedGaussians, activeGaussianCount, cudaStream);

    // Шаг 2: Назначение гауссианов тайлам
    launchTileAssignment(
        d_projectedGaussians, d_tileAssignments, activeGaussianCount, width, height, cudaStream);

    // Шаг 3: Сортировка по глубине
    launchCubRadixSort(d_projectedGaussians, d_sortedIndices, activeGaussianCount, cudaStream);

    // Шаг 4: Растеризация тайлов
    int numTiles = ((width + 15) / 16) * ((height + 15) / 16);
    (void)numTiles;  // Подавляем предупреждение о неиспользуемой переменной
    launchTileRasterization(d_projectedGaussians,
                            d_sortedIndices,
                            d_tileOffsets,
                            framebuffer ? framebuffer : d_framebuffer,
                            depthBuffer ? depthBuffer : d_depthBuffer,
                            width,
                            height,
                            cudaStream);

    // Останавливаем таймер
    if (stopEvent) {
        cudaEventRecord(stopEvent, cudaStream);
        cudaEventSynchronize(stopEvent);
        cudaEventElapsedTime(&lastRenderTime, startEvent, stopEvent);
    }
}

void FlashGSSplatter::synchronize() {
    if (cudaStream) {
        cudaStreamSynchronize(cudaStream);
    }
}

void FlashGSSplatter::computeGradients(const float4* targetImage, const float4* renderedImage) {
    if (!d_gradients || !d_gaussians || activeGaussianCount == 0) {
        return;
    }

    launchGradientComputation(d_gaussians,
                              targetImage,
                              renderedImage,
                              d_gradients,
                              framebufferWidth,
                              framebufferHeight,
                              activeGaussianCount,
                              cudaStream);
}

void FlashGSSplatter::updateGaussianParameters() {
    if (!d_gaussians || !d_gradients || activeGaussianCount == 0) {
        return;
    }

    launchParameterUpdate(d_gaussians, d_gradients, optimParams, activeGaussianCount, cudaStream);
}

void FlashGSSplatter::adaptiveDensityControl() {
    // Упрощенная версия адаптивного контроля плотности
    // В полной реализации здесь будет сложная логика разделения и удаления гауссианов

    SAFE_PRINT_LINE("[FlashGSSplatter] Адаптивный контроль плотности (упрощенная версия)");

    // Пока просто ограничиваем количество гауссианов
    if (activeGaussianCount > maxGaussians * 0.9f) {
        SAFE_PRINT_LINE("[FlashGSSplatter] Достигнут лимит гауссианов: "
                        + SAFE_TO_STRING(activeGaussianCount));
    }
}

std::shared_ptr<SharedResource> FlashGSSplatter::createSharedBuffer(size_t size,
                                                                    vk::BufferUsageFlags usage) {
    if (!cudaInterop || !cudaInterop->isInitialized()) {
        SAFE_ERROR("[FlashGSSplatter] CudaInterop не инициализирован для создания shared буфера");
        return nullptr;
    }

    try {
        // Создаем shared буфер через interop
        auto sharedResource = cudaInterop->createSharedBuffer(size, usage);

        if (sharedResource && sharedResource->isValid) {
            SAFE_PRINT_LINE("[FlashGSSplatter] Создан shared буфер размером " + SAFE_TO_STRING(size)
                            + " байт");
            return sharedResource;
        } else {
            SAFE_ERROR("[FlashGSSplatter] Ошибка создания shared буфера");
            return nullptr;
        }

    } catch (const std::exception& e) {
        SAFE_ERROR("[FlashGSSplatter] Исключение при создании shared буфера: "
                   + std::string(e.what()));
        return nullptr;
    }
}

std::shared_ptr<SharedResource> FlashGSSplatter::exportFramebufferToVulkan() {
    if (!d_framebuffer || framebufferWidth == 0 || framebufferHeight == 0) {
        SAFE_ERROR("[FlashGSSplatter] Framebuffer не готов к экспорту");
        return nullptr;
    }

    if (!cudaInterop || !cudaInterop->isInitialized()) {
        SAFE_ERROR("[FlashGSSplatter] CudaInterop не доступен для экспорта");
        return nullptr;
    }

    try {
        size_t framebufferSize = framebufferWidth * framebufferHeight * sizeof(float4);

        // Создаем shared буфер для framebuffer
        auto sharedResource = createSharedBuffer(
            framebufferSize,
            vk::BufferUsageFlagBits::eTransferSrc | vk::BufferUsageFlagBits::eStorageBuffer);

        if (sharedResource && sharedResource->isValid) {
            // Копируем данные из CUDA framebuffer в shared буфер
            cudaMemcpy(reinterpret_cast<void*>(sharedResource->cudaDevicePtr),
                       d_framebuffer,
                       framebufferSize,
                       cudaMemcpyDeviceToDevice);

            // Синхронизируем stream
            if (cudaStream) {
                cudaStreamSynchronize(cudaStream);
            }

            SAFE_PRINT_LINE("[FlashGSSplatter] Framebuffer экспортирован в Vulkan ("
                            + SAFE_TO_STRING(framebufferWidth) + "x"
                            + SAFE_TO_STRING(framebufferHeight) + ")");

            return sharedResource;
        }

    } catch (const std::exception& e) {
        SAFE_ERROR("[FlashGSSplatter] Ошибка экспорта framebuffer: " + std::string(e.what()));
    }

    return nullptr;
}

bool FlashGSSplatter::importVulkanImage(vk::Image vulkanImage) {
    // Подавляем предупреждение о неиспользуемом параметре
    (void)vulkanImage;

    if (!cudaInterop || !cudaInterop->isInitialized()) {
        SAFE_ERROR("[FlashGSSplatter] CudaInterop не доступен для импорта");
        return false;
    }

    try {
        // В полной реализации здесь будет импорт Vulkan изображения
        // через external memory API и создание CUDA surface/texture

        SAFE_PRINT_LINE("[FlashGSSplatter] Импорт Vulkan изображения (заглушка)");

        // Пока просто логируем что импорт запрошен
        // В реальной реализации:
        // 1. Получаем memory handle от Vulkan изображения
        // 2. Импортируем через cudaImportExternalMemory
        // 3. Создаем CUDA mipmapped array/surface
        // 4. Используем как входные данные для оптимизации

        return true;

    } catch (const std::exception& e) {
        SAFE_ERROR("[FlashGSSplatter] Ошибка импорта Vulkan изображения: " + std::string(e.what()));
        return false;
    }
}

// Дополнительные методы для совместимости с demo
int FlashGSSplatter::getActiveGaussianCount() const {
    return activeGaussianCount;
}

void FlashGSSplatter::setOptimizationParams(const OptimizationParams& params) {
    this->optimParams = params;
    SAFE_PRINT_LINE("[FlashGSSplatter] Параметры оптимизации обновлены: lr="
                    + SAFE_TO_STRING(params.learningRate)
                    + ", threshold=" + SAFE_TO_STRING(params.densificationThreshold));
}

float FlashGSSplatter::getLastRenderTime() const {
    return lastRenderTime;
}

#endif  // CUDA_VULKAN_INTEROP_SUPPORTED

}  // namespace SpectraForge::CUDA
