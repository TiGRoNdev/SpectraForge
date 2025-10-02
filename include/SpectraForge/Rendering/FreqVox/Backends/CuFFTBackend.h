/**
 * @file CuFFTBackend.h
 * @brief CUDA cuFFT backend for frequency-domain transformations
 */

#pragma once

#include "SpectraForge/Rendering/FreqVox/FrequencyShading.h"
#include <vector>
#include <memory>

#ifdef HYPERENGINE_CUDA_AVAILABLE
#include <cufft.h>
#include <cuda_runtime.h>
#endif

namespace SpectraForge::Rendering::FreqVox::Backends {

/**
 * @brief cuFFT-based backend for DCT/FFT transformations
 * 
 * Uses NVIDIA cuFFT library for batched 2D DCT operations on GPU.
 * Requires CUDA support at compile time.
 */
class CuFFTBackend : public IFrequencyBackend {
public:
    CuFFTBackend();
    ~CuFFTBackend() override;

    bool initialize(const DctBlockConfig& config) override;
    void shutdown() override;
    bool transform_forward(std::vector<float>& io_block_batched) override;
    bool transform_inverse(std::vector<float>& io_block_batched) override;

private:
    DctBlockConfig cfg_;
    bool initialized_ = false;

#ifdef HYPERENGINE_CUDA_AVAILABLE
    cufftHandle plan_forward_;
    cufftHandle plan_inverse_;
    cudaStream_t stream_;
    float* d_buffer_ = nullptr;
    size_t buffer_size_ = 0;

    bool createPlans();
    void destroyPlans();
    bool setupDeviceMemory(size_t required_size);
#endif

    // Запрет копирования
    CuFFTBackend(const CuFFTBackend&) = delete;
    CuFFTBackend& operator=(const CuFFTBackend&) = delete;
};

} // namespace SpectraForge::Rendering::FreqVox::Backends

