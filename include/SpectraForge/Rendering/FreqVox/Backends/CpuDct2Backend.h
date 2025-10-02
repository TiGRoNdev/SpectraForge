/**
 * @file CpuDct2Backend.h
 * @brief CPU-based DCT-II implementation according to FreqVox Math.md
 * 
 * Implements the exact DCT-II formula:
 * M̃[u,v] = Σ_{p=0}^{P-1} Σ_{q=0}^{Q-1} M[p,q] cos(πu(2p+1)/(2P)) cos(πv(2q+1)/(2Q))
 */

#pragma once

#include "SpectraForge/Rendering/FreqVox/FrequencyShading.h"
#include <vector>
#include <cmath>

namespace SpectraForge::Rendering::FreqVox::Backends {

/**
 * @brief Type-II DCT backend (CPU implementation)
 * 
 * Implements forward and inverse DCT-II transforms as specified in
 * "Detailed Mathematical Framework for AFS-NVR" section 2.
 */
class CpuDct2Backend : public IFrequencyBackend {
public:
    CpuDct2Backend() = default;
    ~CpuDct2Backend() override = default;

    /**
     * @brief Initialize DCT-II backend
     * @param config Block configuration (blockSize, batchCount)
     * @return true if successful
     */
    bool initialize(const DctBlockConfig& config) override;

    /**
     * @brief Cleanup resources
     */
    void shutdown() override;

    /**
     * @brief Forward DCT-II transform (spatial → frequency)
     * 
     * Implements: M̃[u,v] = Σ_{p,q} M[p,q] cos(πu(2p+1)/(2P)) cos(πv(2q+1)/(2Q))
     * 
     * @param io_block_batched Input/output data [batch][blockSize*blockSize]
     * @return true if successful
     */
    bool transform_forward(std::vector<float>& io_block_batched) override;

    /**
     * @brief Inverse DCT-II transform (frequency → spatial)
     * 
     * Implements: M[p,q] = Σ_{u,v} α(u)α(v) M̃[u,v] cos(πu(2p+1)/(2P)) cos(πv(2q+1)/(2Q))
     * where α(u) = 1/√N for u=0, α(u) = √(2/N) for u>0
     * 
     * @param io_block_batched Input/output data [batch][blockSize*blockSize]
     * @return true if successful
     */
    bool transform_inverse(std::vector<float>& io_block_batched) override;

private:
    DctBlockConfig cfg_{};
    bool initialized_ = false;

    // Precomputed cosine tables for efficiency
    std::vector<float> forwardCosTable_;   // cos(πu(2p+1)/(2N))
    std::vector<float> inverseCosTable_;   // Same, but with scaling factors
    
    /**
     * @brief Precompute cosine lookup tables
     */
    void precomputeCosineTables();

    /**
     * @brief 2D DCT-II forward transform for a single block
     * @param block Input spatial data [N×N]
     * @param output Output frequency data [N×N]
     * @param N Block size
     */
    void dct2_forward_2d(const float* block, float* output, int N);

    /**
     * @brief 2D DCT-II inverse transform for a single block
     * @param block Input frequency data [N×N]
     * @param output Output spatial data [N×N]
     * @param N Block size
     */
    void dct2_inverse_2d(const float* block, float* output, int N);

    /**
     * @brief Get cosine value from precomputed table
     * @param u Frequency index
     * @param p Spatial index
     * @param N Block size
     * @return cos(πu(2p+1)/(2N))
     */
    inline float getCosine(int u, int p, int N) const {
        return forwardCosTable_[u * N + p];
    }

    /**
     * @brief Get scaled cosine for inverse transform
     * @param u Frequency index
     * @param p Spatial index
     * @param N Block size
     * @return α(u) * cos(πu(2p+1)/(2N))
     */
    inline float getScaledCosine(int u, int p, int N) const {
        return inverseCosTable_[u * N + p];
    }
};

} // namespace SpectraForge::Rendering::FreqVox::Backends

