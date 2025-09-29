#pragma once

#include <memory>
#include <vector>

#ifdef CUDA_VULKAN_INTEROP_SUPPORTED
#include <cuda.h>
#include <cuda_runtime.h>
#include <vulkan/vulkan.hpp>
#endif

// Forward declarations
namespace HyperEngine::Vulkan {
struct Gaussians;
struct MultiViewImages;
struct PrimaryImage;
}  // namespace HyperEngine::Vulkan

namespace HyperEngine {
struct CameraParams;
}

namespace HyperEngine::CUDA {
class CudaInterop;
struct SharedResource;
}  // namespace HyperEngine::CUDA

namespace HyperEngine::CUDA {

#ifdef CUDA_VULKAN_INTEROP_SUPPORTED

/**
 * @brief Структура гауссиана на GPU (соответствует gaussian_optimization.cu)
 */
struct GPUGaussian {
    float4 position;       // x, y, z, opacity
    float4 color;          // r, g, b, scale
    float4 rotation;       // quaternion (x, y, z, w)
    float4 covariance[2];  // 6 элементов ковариационной матрицы (3x3 symmetric)
};

/**
 * @brief Проецированный гауссиан на экране (соответствует tile_rasterization.cu)
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
 * @brief Параметры камеры для проекции (соответствует tile_rasterization.cu)
 */
struct CameraMatrix {
    float viewMatrix[16];       // View matrix (4x4)
    float projMatrix[16];       // Projection matrix (4x4)
    float viewProjMatrix[16];   // View-Projection matrix (4x4)
    int width, height;          // Разрешение
    float nearPlane, farPlane;  // Near/far planes
};

/**
 * @brief Параметры оптимизации (соответствует gaussian_optimization.cu)
 */
struct OptimizationParams {
    float learningRate;
    float densificationThreshold;
    float pruningThreshold;
    int maxGaussians;
    int iterationCount;
};

#endif  // CUDA_VULKAN_INTEROP_SUPPORTED

/**
 * @brief Параметры гауссианов для рендеринга (legacy API)
 */
struct GaussianParams {
    float* positions;    // Позиции гауссианов (x, y, z, w)
    float* covariances;  // Ковариационные матрицы
    float* opacities;    // Прозрачности
    float* colors;       // Цвета (RGB)
    uint32_t count;      // Количество гауссианов
};

/**
 * @brief Tile-based растеризатор для гауссианов
 */
class TileBasedRasterizer {
  public:
    TileBasedRasterizer();
    ~TileBasedRasterizer();

    bool init(uint32_t w, uint32_t h, uint32_t tSize = 16);
    void shutdown();

    void rasterize(const GaussianParams& params, const CameraParams& camera);

  private:
    uint32_t width, height, tileSize;
    bool initialized;
};

/**
 * @brief CUDA-ускоренный 3D Gaussian Splatter
 *
 * Реализует FlashGS алгоритм для быстрого рендеринга гауссианов
 * с использованием CUDA для оптимизации производительности.
 */
class FlashGSSplatter {
  public:
    /**
     * @brief Конструктор
     */
    FlashGSSplatter();

    /**
     * @brief Деструктор
     */
    ~FlashGSSplatter();

    /**
     * @brief Инициализация splatter'а
     * @param interop CUDA-Vulkan interop объект (опционально)
     * @return true если инициализация успешна
     */
    bool init(std::shared_ptr<CudaInterop> interop = nullptr);

    /**
     * @brief Завершение работы
     */
    void shutdown();

    /**
     * @brief Оптимизация гауссианов из мульти-вид изображений
     * @param images Изображения с разных ракурсов
     * @param iterations Количество итераций оптимизации
     */
    void optimizeGaussians(const Vulkan::MultiViewImages& images, int iterations = 100);

    /**
     * @brief Растеризация гауссианов
     * @param params Параметры камеры
     * @return Первичное изображение
     */
    Vulkan::PrimaryImage rasterizeGaussians(const CameraParams& camera);

#ifdef CUDA_VULKAN_INTEROP_SUPPORTED
    /**
     * @brief CUDA-ускоренная растеризация с tile-based алгоритмом
     * @param camera Матрица камеры
     * @param framebuffer Выходной буфер кадра
     * @param depthBuffer Буфер глубины
     * @param width Ширина изображения
     * @param height Высота изображения
     */
    void rasterizeGaussiansCUDA(const CameraMatrix& camera,
                                float4* framebuffer,
                                float* depthBuffer,
                                int width,
                                int height);

    /**
     * @brief Инициализация гауссианов из точечного облака
     * @param points Массив точек (x, y, z, intensity)
     * @param numPoints Количество точек
     * @param baseScale Базовый масштаб гауссианов
     */
    void initializeFromPointCloud(const float4* points, int numPoints, float baseScale = 0.01f);

    /**
     * @brief Получение GPU данных гауссианов
     * @return Указатель на GPU массив гауссианов
     */
    GPUGaussian* getGPUGaussians() const;

    /**
     * @brief Получение количества активных гауссианов
     * @return Количество гауссианов
     */
    int getActiveGaussianCount() const;

    /**
     * @brief Установка параметров оптимизации
     * @param params Параметры оптимизации
     */
    void setOptimizationParams(const OptimizationParams& params);

    /**
     * @brief Синхронизация CUDA stream
     */
    void synchronize();

    /**
     * @brief Получение статистики производительности
     * @return Время последнего рендера в миллисекундах
     */
    float getLastRenderTime() const;

    /**
     * @brief Создание shared буфера с Vulkan
     * @param size Размер буфера
     * @param usage Флаги использования Vulkan
     * @return Shared ресурс или nullptr при ошибке
     */
    std::shared_ptr<SharedResource> createSharedBuffer(
        size_t size,
        vk::BufferUsageFlags usage = vk::BufferUsageFlagBits::eStorageBuffer);

    /**
     * @brief Экспорт framebuffer в Vulkan для дальнейшего использования
     * @return Shared ресурс с framebuffer данными
     */
    std::shared_ptr<SharedResource> exportFramebufferToVulkan();

    /**
     * @brief Импорт Vulkan изображения как входные данные
     * @param vulkanImage Vulkan изображение
     * @return true если импорт успешен
     */
    bool importVulkanImage(vk::Image vulkanImage);
#endif

    /**
     * @brief Получение текущих параметров гауссианов (legacy API)
     * @return Параметры гауссианов
     */
    const GaussianParams& getGaussianParams() const { return legacyParams; }

    /**
     * @brief Проверка инициализации
     * @return true если инициализирован
     */
    bool isInitialized() const { return initialized; }

  private:
    // Legacy API
    GaussianParams legacyParams;
    std::unique_ptr<TileBasedRasterizer> rasterizer;

#ifdef CUDA_VULKAN_INTEROP_SUPPORTED
    // CUDA ресурсы
    cudaStream_t cudaStream = nullptr;

    // GPU буферы
    GPUGaussian* d_gaussians = nullptr;
    ProjectedGaussian* d_projectedGaussians = nullptr;
    uint32_t* d_sortedIndices = nullptr;
    uint32_t* d_tileAssignments = nullptr;
    uint32_t* d_tileOffsets = nullptr;
    float4* d_gradients = nullptr;

    // Framebuffer данные
    float4* d_framebuffer = nullptr;
    float* d_depthBuffer = nullptr;
    int framebufferWidth = 0;
    int framebufferHeight = 0;

    // Параметры
    OptimizationParams optimParams;
    int maxGaussians = 100000;
    int activeGaussianCount = 0;

    // Производительность
    cudaEvent_t startEvent = nullptr;
    cudaEvent_t stopEvent = nullptr;
    float lastRenderTime = 0.0f;

    // Interop
    std::shared_ptr<CudaInterop> cudaInterop;
#else
    void* cudaStream = nullptr;  // Заглушка для совместимости
#endif

    bool initialized = false;

    /**
     * @brief Инициализация CUDA ресурсов
     */
    bool initCUDA();

    /**
     * @brief Освобождение CUDA ресурсов
     */
    void cleanupCUDA();

#ifdef CUDA_VULKAN_INTEROP_SUPPORTED
    /**
     * @brief Выделение GPU буферов
     * @param width Ширина framebuffer
     * @param height Высота framebuffer
     */
    bool allocateGPUBuffers(int width, int height);

    /**
     * @brief Освобождение GPU буферов
     */
    void freeGPUBuffers();

    /**
     * @brief Вычисление градиентов для оптимизации
     * @param targetImage Целевое изображение
     * @param renderedImage Отрендеренное изображение
     */
    void computeGradients(const float4* targetImage, const float4* renderedImage);

    /**
     * @brief Обновление параметров гауссианов
     */
    void updateGaussianParameters();

    /**
     * @brief Адаптивный контроль плотности
     */
    void adaptiveDensityControl();
#endif

    // Запрет копирования
    FlashGSSplatter(const FlashGSSplatter&) = delete;
    FlashGSSplatter& operator=(const FlashGSSplatter&) = delete;
};

}  // namespace HyperEngine::CUDA
