/**
 * @file FrequencyShading.h
 * @brief Абстракция частотного шейдинга (DCT/FFT) для FreqVox
 */

#pragma once

#include <cstdint>
#include <vector>
#include <memory>
#include "SpectraForge/Rendering/FreqVox/FreqVoxTypes.h"

namespace SpectraForge::Rendering::FreqVox {

/**
 * @brief Интерфейс бэкенда FFT/DCT
 */
class IFrequencyBackend {
  public:
    virtual ~IFrequencyBackend() = default;
    virtual bool initialize(const DctBlockConfig& config) = 0;
    virtual void shutdown() = 0;
    virtual bool transform_forward(std::vector<float>& io_block_batched) = 0;  ///< DCT/FFT
    virtual bool transform_inverse(std::vector<float>& io_block_batched) = 0;  ///< IDCT/IFFT
};

/**
 * @brief Высокоуровневый оркестратор частотного шейдинга
 * 
 * Реализует frequency-domain shading согласно FreqVox Math.md раздел 2:
 * 
 * S̃_i[u,v] = L̃_i[u,v] ⊙ M̃[u,v]
 * 
 * Где:
 * - L̃_i[u,v] - incident lighting в частотной области (после DCT)
 * - M̃[u,v] - material BRDF response в частотной области
 * - ⊙ - element-wise multiplication (convolution theorem)
 * - S̃_i[u,v] - результат shading в частотной области
 * 
 * Затем inverse DCT для получения финального S_i[p,q] в spatial domain.
 */
class FrequencyShadingPipeline {
  public:
    explicit FrequencyShadingPipeline(std::unique_ptr<IFrequencyBackend> backend)
        : backend_(std::move(backend)) {}

    /**
     * @brief Инициализация pipeline
     * @param config Конфигурация блоков DCT
     * @return true если успешно
     */
    bool initialize(const DctBlockConfig& config) {
        config_ = config;
        return backend_ && backend_->initialize(config_);
    }

    /**
     * @brief Завершение работы
     */
    void shutdown() {
        if (backend_) backend_->shutdown();
    }

    /**
     * @brief Frequency-domain shading для полного изображения (high-level API)
     * @param image_buffer RGB изображение [width * height * 3], будет изменен in-place
     * @param width Ширина изображения в пикселях
     * @param height Высота изображения в пикселях
     * @param material_freq Material BRDF в частотной области [blockSize²]
     * @return true если успешно
     * 
     * Автоматически разбивает изображение на блоки 8×8, применяет
     * frequency-domain shading к каждому блоку, и собирает результат обратно.
     */
    bool shade_image(std::vector<float>& image_buffer,
                     uint32_t width,
                     uint32_t height,
                     const std::vector<float>& material_freq);

    /**
     * @brief Frequency-domain shading для батча блоков (low-level API)
     * @param lighting_blocks Lighting data [batch][blockSize²], будет изменен in-place
     * @param material_freq Material BRDF в частотной области [blockSize²]
     * @return true если успешно
     * 
     * Алгоритм:
     * 1. DCT(lighting) → L̃
     * 2. L̃ ⊙ M̃ (element-wise multiply)
     * 3. IDCT(L̃ ⊙ M̃) → S (shaded result)
     */
    bool shade_blocks(std::vector<float>& lighting_blocks,
                     const std::vector<float>& material_freq);

    /**
     * @brief Precompute material BRDF в частотной области
     * @param material_spatial Material BRDF в spatial domain [blockSize²]
     * @param material_freq Output: материал в частотной области
     * @return true если успешно
     * 
     * Это нужно вызвать один раз для каждого уникального материала,
     * затем использовать результат в shade_blocks().
     */
    bool precompute_material(const std::vector<float>& material_spatial,
                            std::vector<float>& material_freq);

  private:
    DctBlockConfig config_{};
    std::unique_ptr<IFrequencyBackend> backend_;

    /**
     * @brief Element-wise multiply двух массивов
     * @param a Первый массив (будет изменен in-place)
     * @param b Второй массив
     * @return true если успешно
     */
    bool elementwise_multiply(std::vector<float>& a, const std::vector<float>& b);
};

}  // namespace SpectraForge::Rendering::FreqVox


