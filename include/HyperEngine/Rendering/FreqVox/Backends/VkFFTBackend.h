/**
 * @file VkFFTBackend.h
 * @brief VkFFT backend for frequency-domain transformations (cross-platform)
 */

#pragma once

#include "HyperEngine/Rendering/FreqVox/FrequencyShading.h"
#include <vector>
#include <memory>

namespace HyperEngine::Rendering::FreqVox::Backends {

/**
 * @brief VkFFT-based backend for DCT/FFT transformations
 * 
 * Uses VkFFT library for Vulkan-accelerated FFT/DCT operations.
 * Cross-platform alternative to cuFFT.
 */
class VkFFTBackend : public IFrequencyBackend {
public:
    VkFFTBackend();
    ~VkFFTBackend() override;

    bool initialize(const DctBlockConfig& config) override;
    void shutdown() override;
    bool transform_forward(std::vector<float>& io_block_batched) override;
    bool transform_inverse(std::vector<float>& io_block_batched) override;
    std::string getName() const override { return "VkFFTBackend"; }

private:
    DctBlockConfig cfg_;
    bool initialized_ = false;

    // VkFFT integration будет добавлена позже
    // Сейчас - заглушка для компиляции

    // Запрет копирования
    VkFFTBackend(const VkFFTBackend&) = delete;
    VkFFTBackend& operator=(const VkFFTBackend&) = delete;
};

} // namespace HyperEngine::Rendering::FreqVox::Backends

