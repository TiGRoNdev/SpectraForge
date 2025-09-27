#pragma once

#include <memory>
#include <vector>

// Forward declarations
namespace Engine3D::Vulkan {
    struct Gaussians;
    struct PrimaryImage;
    struct MultiViewImages;
}

namespace Engine3D {
    struct CameraParams;
}

namespace Engine3D::CUDA {

/**
 * @brief Параметры гауссианов для рендеринга
 */
struct GaussianParams {
    float* positions;      // Позиции гауссианов (x, y, z, w)
    float* covariances;    // Ковариационные матрицы
    float* opacities;      // Прозрачности
    float* colors;         // Цвета (RGB)
    uint32_t count;        // Количество гауссианов
};

/**
 * @brief Tile-based растеризатор для гауссианов
 */
class TileBasedRasterizer {
public:
    TileBasedRasterizer();
    ~TileBasedRasterizer();
    
    bool init(uint32_t width, uint32_t height, uint32_t tileSize = 16);
    void shutdown();
    
    void rasterize(const GaussianParams& params, const CameraParams& camera);
    
private:
    uint32_t width, height, tileSize;
    bool initialized = false;
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
     * @return true если инициализация успешна
     */
    bool init();
    
    /**
     * @brief Завершение работы
     */
    void shutdown();
    
    /**
     * @brief Оптимизация гауссианов из мульти-вид изображений
     * @param images Изображения с разных ракурсов
     */
    void optimizeGaussians(const Vulkan::MultiViewImages& images);
    
    /**
     * @brief Растеризация гауссианов
     * @param params Параметры камеры
     * @return Первичное изображение
     */
    Vulkan::PrimaryImage rasterizeGaussians(const CameraParams& params);
    
    /**
     * @brief Получение текущих параметров гауссианов
     * @return Параметры гауссианов
     */
    const GaussianParams& getGaussianParams() const { return params; }
    
    /**
     * @brief Проверка инициализации
     * @return true если инициализирован
     */
    bool isInitialized() const { return initialized; }

private:
    GaussianParams params;
    std::unique_ptr<TileBasedRasterizer> rasterizer;
    
    // CUDA ресурсы (будут добавлены на этапе 3)
    void* cudaStream = nullptr;  // cudaStream_t
    
    bool initialized = false;
    
    /**
     * @brief Инициализация CUDA ресурсов
     */
    bool initCUDA();
    
    /**
     * @brief Освобождение CUDA ресурсов
     */
    void cleanupCUDA();
    
    // Запрет копирования
    FlashGSSplatter(const FlashGSSplatter&) = delete;
    FlashGSSplatter& operator=(const FlashGSSplatter&) = delete;
};

} // namespace Engine3D::CUDA