/**
 * @file FrequencyShading.h
 * @brief Абстракция частотного шейдинга (DCT/FFT) для FreqVox
 */

#pragma once

#include <cstdint>
#include <vector>
#include <memory>
#include "HyperEngine/Rendering/FreqVox/FreqVoxTypes.h"

namespace HyperEngine::Rendering::FreqVox {

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
 */
class FrequencyShadingPipeline {
  public:
    explicit FrequencyShadingPipeline(std::unique_ptr<IFrequencyBackend> backend)
        : backend_(std::move(backend)) {}

    bool initialize(const DctBlockConfig& config) {
        config_ = config;
        return backend_ && backend_->initialize(config_);
    }

    void shutdown() {
        if (backend_) backend_->shutdown();
    }

    bool shade_blocks(std::vector<float>& block_data) {
        // ẼL ⊙ ẼM можно добавить позже; пока заглушка: forward -> inverse
        if (!backend_) return false;
        if (!backend_->transform_forward(block_data)) return false;
        return backend_->transform_inverse(block_data);
    }

  private:
    DctBlockConfig config_{};
    std::unique_ptr<IFrequencyBackend> backend_;
};

}  // namespace HyperEngine::Rendering::FreqVox


