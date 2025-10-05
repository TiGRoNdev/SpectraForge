/**
 * @file CpuDct2Backend.cpp
 * @brief CPU-based DCT-II implementation
 * 
 * Implements the exact mathematical formulas from FreqVox Renderer Math.md:
 * 
 * Forward DCT-II:
 *   M̃[u,v] = Σ_{p=0}^{P-1} Σ_{q=0}^{Q-1} M[p,q] cos(πu(2p+1)/(2P)) cos(πv(2q+1)/(2Q))
 * 
 * Inverse DCT-II:
 *   M[p,q] = Σ_{u=0}^{P-1} Σ_{v=0}^{Q-1} α(u)α(v) M̃[u,v] cos(πu(2p+1)/(2P)) cos(πv(2q+1)/(2Q))
 * 
 * where α(0) = 1/√N, α(k) = √(2/N) for k > 0
 */

#include "SpectraForge/Rendering/FreqVox/Backends/CpuDct2Backend.h"
#include "SpectraForge/Core/SafeConsole.h"
#include <cmath>
#include <algorithm>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

namespace SpectraForge::Rendering::FreqVox::Backends {

bool CpuDct2Backend::initialize(const DctBlockConfig& config) {
    if (initialized_) {
        SAFE_WARNING("[CpuDct2Backend] Already initialized");
        return true;
    }

    cfg_ = config;

    // Validate configuration
    if (cfg_.blockSize == 0 || cfg_.batchCount == 0) {
        SAFE_ERROR("[CpuDct2Backend] Invalid configuration");
        return false;
    }

    // Precompute cosine tables for efficiency
    precomputeCosineTables();

    initialized_ = true;
    SAFE_PRINT_LINE("[CpuDct2Backend] Initialized with blockSize=" + 
                    std::to_string(cfg_.blockSize) + ", batchCount=" + 
                    std::to_string(cfg_.batchCount));
    return true;
}

void CpuDct2Backend::shutdown() {
    if (!initialized_) return;

    forwardCosTable_.clear();
    inverseCosTable_.clear();
    initialized_ = false;

    SAFE_PRINT_LINE("[CpuDct2Backend] Shutdown complete");
}

void CpuDct2Backend::precomputeCosineTables() {
    const int N = cfg_.blockSize;
    
    // Allocate tables: [u][p] for u,p ∈ [0, N-1]
    forwardCosTable_.resize(N * N);
    inverseCosTable_.resize(N * N);

    // Precompute cos(πu(2p+1)/(2N)) for all u, p
    for (int u = 0; u < N; ++u) {
        for (int p = 0; p < N; ++p) {
            float cosValue = std::cos(M_PI * u * (2.0f * p + 1.0f) / (2.0f * N));
            forwardCosTable_[u * N + p] = cosValue;

            // For inverse: apply scaling factors α(u)
            // α(0) = 1/√N, α(k) = √(2/N) for k > 0
            float alpha = (u == 0) ? (1.0f / std::sqrt(static_cast<float>(N)))
                                   : std::sqrt(2.0f / static_cast<float>(N));
            inverseCosTable_[u * N + p] = alpha * cosValue;
        }
    }
}

bool CpuDct2Backend::transform_forward(std::vector<float>& io_block_batched) {
    if (!initialized_) {
        SAFE_ERROR("[CpuDct2Backend] Not initialized");
        return false;
    }

    const int N = cfg_.blockSize;
    const int blockSize = N * N;
    const size_t expectedSize = cfg_.batchCount * blockSize;

    if (io_block_batched.size() != expectedSize) {
        SAFE_ERROR("[CpuDct2Backend] Invalid input size for forward DCT");
        return false;
    }

    // Temporary buffer for output
    std::vector<float> outputBuffer(blockSize);

    // Process each block in batch
    for (uint32_t batch = 0; batch < cfg_.batchCount; ++batch) {
        const float* inputBlock = io_block_batched.data() + batch * blockSize;
        float* outputBlock = io_block_batched.data() + batch * blockSize;

        // Perform 2D DCT-II forward transform
        dct2_forward_2d(inputBlock, outputBuffer.data(), N);

        // Copy result back
        std::copy(outputBuffer.begin(), outputBuffer.end(), outputBlock);
    }

    return true;
}

bool CpuDct2Backend::transform_inverse(std::vector<float>& io_block_batched) {
    if (!initialized_) {
        SAFE_ERROR("[CpuDct2Backend] Not initialized");
        return false;
    }

    const int N = cfg_.blockSize;
    const int blockSize = N * N;
    const size_t expectedSize = cfg_.batchCount * blockSize;

    if (io_block_batched.size() != expectedSize) {
        SAFE_ERROR("[CpuDct2Backend] Invalid input size for inverse DCT");
        return false;
    }

    // Temporary buffer for output
    std::vector<float> outputBuffer(blockSize);

    // Process each block in batch
    for (uint32_t batch = 0; batch < cfg_.batchCount; ++batch) {
        const float* inputBlock = io_block_batched.data() + batch * blockSize;
        float* outputBlock = io_block_batched.data() + batch * blockSize;

        // Perform 2D DCT-II inverse transform
        dct2_inverse_2d(inputBlock, outputBuffer.data(), N);

        // Copy result back
        std::copy(outputBuffer.begin(), outputBuffer.end(), outputBlock);
    }

    return true;
}

void CpuDct2Backend::dct2_forward_2d(const float* block, float* output, int N) {
    // Forward 2D DCT-II:
    // M̃[u,v] = Σ_{p=0}^{N-1} Σ_{q=0}^{N-1} M[p,q] cos(πu(2p+1)/(2N)) cos(πv(2q+1)/(2N))

    for (int u = 0; u < N; ++u) {
        for (int v = 0; v < N; ++v) {
            float sum = 0.0f;

            // Sum over all spatial positions (p, q)
            for (int p = 0; p < N; ++p) {
                for (int q = 0; q < N; ++q) {
                    float spatialValue = block[p * N + q];
                    float cosU = getCosine(u, p, N);
                    float cosV = getCosine(v, q, N);
                    sum += spatialValue * cosU * cosV;
                }
            }

            output[u * N + v] = sum;
        }
    }
}

void CpuDct2Backend::dct2_inverse_2d(const float* block, float* output, int N) {
    // Inverse 2D DCT-II:
    // M[p,q] = Σ_{u=0}^{N-1} Σ_{v=0}^{N-1} α(u)α(v) M̃[u,v] cos(πu(2p+1)/(2N)) cos(πv(2q+1)/(2N))
    // 
    // where α(0) = 1/√N, α(k) = √(2/N) for k > 0

    for (int p = 0; p < N; ++p) {
        for (int q = 0; q < N; ++q) {
            float sum = 0.0f;

            // Sum over all frequency components (u, v)
            for (int u = 0; u < N; ++u) {
                for (int v = 0; v < N; ++v) {
                    float freqValue = block[u * N + v];
                    
                    // Use precomputed scaled cosines (includes α factors)
                    float scaledCosU = getScaledCosine(u, p, N);
                    float scaledCosV = getScaledCosine(v, q, N);
                    
                    sum += freqValue * scaledCosU * scaledCosV;
                }
            }

            output[p * N + q] = sum;
        }
    }
}

} // namespace SpectraForge::Rendering::FreqVox::Backends

