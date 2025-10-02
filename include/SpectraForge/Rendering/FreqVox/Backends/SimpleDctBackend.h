/**
 * @file SimpleDctBackend.h
 * @brief Простой backend (заглушка) для DCT/IDCT без внешних зависимостей
 */

#pragma once

#include "SpectraForge/Rendering/FreqVox/FrequencyShading.h"

namespace SpectraForge::Rendering::FreqVox::Backends {

/**
 * @brief Заглушка-бэкенд, выполняющая тождественные преобразования
 */
class SimpleDctBackend : public IFrequencyBackend {
  public:
    bool initialize(const DctBlockConfig& config) override;
    void shutdown() override;
    bool transform_forward(std::vector<float>& io_block_batched) override;
    bool transform_inverse(std::vector<float>& io_block_batched) override;

  private:
    DctBlockConfig cfg_{};
    bool initialized_ = false;
};

}  // namespace SpectraForge::Rendering::FreqVox::Backends


