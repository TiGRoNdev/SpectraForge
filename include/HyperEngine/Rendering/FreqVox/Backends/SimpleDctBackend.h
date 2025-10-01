/**
 * @file SimpleDctBackend.h
 * @brief Простой backend (заглушка) для DCT/IDCT без внешних зависимостей
 */

#pragma once

#include "HyperEngine/Rendering/FreqVox/FrequencyShading.h"

namespace HyperEngine::Rendering::FreqVox {

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

}  // namespace HyperEngine::Rendering::FreqVox


