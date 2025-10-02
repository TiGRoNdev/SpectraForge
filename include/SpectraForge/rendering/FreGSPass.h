/**
 * @file FreGSPass.h
 * @brief Frequency-Encoded Gaussian Splatting render pass
 *
 * Implements analytic frequency-domain Gaussian splatting on wavelet subbands.
 * SRP: Single responsibility - frequency-domain accumulation
 * DIP: Depends on IRenderPass and WaveletSubbands abstractions
 *
 * Performance: O(M+P) complexity, no global atomics
 *
 * @see shaders/GaussFreqSplat.comp
 * @author SpectraForge Core Team
 * @date 2025-10-02
 */

#pragma once

#include "RenderPass.h"
#include "WaveletPass.h"
#include <vulkan/vulkan.hpp>
#include <glm/glm.hpp>
#include <vector>

namespace spectraforge {
namespace rendering {

/**
 * @brief Single Gaussian splat representation
 */
struct GaussianSplat {
    glm::vec4 positionAndScale;  // xyz = position μ, w = scale σ
    glm::vec4 colorAndWeight;    // rgb = color, a = weight
};

/**
 * @brief Configuration for FreGS pass
 */
struct FreGSPassConfig {
    uint32_t outputWidth;
    uint32_t outputHeight;
    float freqScale = 1.0f;
    uint32_t subbandLevel = 0;
    float foveaRadius = 0.1f;      // Normalized [0, 1]
    glm::vec2 foveaCenter = {0.5f, 0.5f};
    uint32_t maxGaussians = 1024;
    bool enableFoveation = true;
};

/**
 * @brief Frequency-Encoded Gaussian Splatting pass
 * 
 * Following SOLID:
 * - SRP: Only frequency-domain splatting
 * - OCP: Can be extended for different kernel types
 * - DIP: Depends on WaveletSubbands abstraction
 */
class FreGSPass : public RenderPassBase {
public:
    explicit FreGSPass(const FreGSPassConfig& config);
    ~FreGSPass() override;

    // IRenderPass interface
    bool initialize(const VulkanContext& context) override;
    void execute(vk::CommandBuffer commandBuffer, uint32_t frameIndex) override;
    void cleanup() override;

    /**
     * @brief Set input wavelet subbands
     */
    void setInputSubbands(const WaveletSubbands& subbands);

    /**
     * @brief Upload Gaussian splats to GPU
     */
    void uploadGaussians(const std::vector<GaussianSplat>& gaussians);

    /**
     * @brief Update foveation parameters (eye tracking)
     */
    void updateFoveation(glm::vec2 gazePoint, float radius);

    /**
     * @brief Get output accumulator image
     */
    vk::ImageView getOutputView() const { return outputView_; }

private:
    /**
     * @brief Create output accumulator image
     */
    bool createOutputImage(const VulkanContext& context);

    /**
     * @brief Create Gaussian storage buffer
     */
    bool createGaussianBuffer(const VulkanContext& context);

    /**
     * @brief Create descriptor sets
     */
    bool createDescriptorSets(const VulkanContext& context);

    /**
     * @brief Load shader SPIR-V
     */
    std::vector<uint32_t> loadShaderSPIRV() const;

    FreGSPassConfig config_;
    
    // Input subbands
    const WaveletSubbands* inputSubbands_ = nullptr;
    
    // Output accumulator
    vk::Image outputImage_;
    vk::ImageView outputView_;
    vk::DeviceMemory outputMemory_;
    
    // Gaussian buffer
    vk::Buffer gaussianBuffer_;
    vk::DeviceMemory gaussianMemory_;
    uint32_t gaussianCount_ = 0;
    
    // Descriptors
    vk::DescriptorPool descriptorPool_;
    std::vector<vk::DescriptorSet> descriptorSets_;
    
    // Push constants
    struct PushConstants {
        uint32_t outputWidth;
        uint32_t outputHeight;
        float freqScale;
        uint32_t subbandLevel;
        float foveaRadius;
        glm::vec2 foveaCenter;
        uint32_t maxGaussians;
    } pushConstants_;
};

} // namespace rendering
} // namespace spectraforge

