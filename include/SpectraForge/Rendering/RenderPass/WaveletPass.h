/**
 * @file WaveletPass.h
 * @brief Wavelet Lifting Transform pass for Hybrid DWT + FreGS pipeline
 *
 * Implements 2D Daubechies-4 lifting scheme with Vulkan subgroups.
 * SRP: Single responsibility - wavelet decomposition only
 * DIP: Depends on IRenderPass abstraction
 *
 * Performance: O(N) complexity, ~0.9ms @ 8K
 *
 * @see shaders/WaveletLifting.comp
 * @author SpectraForge Core Team
 * @date 2025-10-02
 */

#pragma once

#include "RenderPass.h"
#include "SpectraForge/core/VMAMemoryManager.h"
#include <vulkan/vulkan.hpp>
#include <array>

namespace spectraforge {
namespace rendering {

/**
 * @brief Configuration for wavelet pass
 */
struct WaveletPassConfig {
    uint32_t inputWidth;
    uint32_t inputHeight;
    float threshold = 0.01f;        // Sparsity threshold
    uint32_t foveationLevel = 0;    // 0 = full spectrum
    bool enableProfiling = false;
};

/**
 * @brief Wavelet subband outputs
 */
struct WaveletSubbands {
    // Vulkan images (extracted from VMA wrappers for descriptor binding)
    vk::Image imageLL;  // Low-Low (approximation)
    vk::Image imageLH;  // Low-High (horizontal details)
    vk::Image imageHL;  // High-Low (vertical details)
    vk::Image imageHH;  // High-High (diagonal details)
    
    vk::ImageView viewLL;
    vk::ImageView viewLH;
    vk::ImageView viewHL;
    vk::ImageView viewHH;
    
    // VMA RAII wrappers (automatic memory management)
    core::VMAImage vmaImageLL;
    core::VMAImage vmaImageLH;
    core::VMAImage vmaImageHL;
    core::VMAImage vmaImageHH;
};

/**
 * @brief Wavelet Lifting Transform render pass
 * 
 * Following SOLID:
 * - SRP: Only wavelet decomposition
 * - OCP: Can be extended for different wavelet types
 * - LSP: Substitutable for IRenderPass
 */
class WaveletPass : public RenderPassBase {
public:
    explicit WaveletPass(const WaveletPassConfig& config);
    ~WaveletPass() override;

    // IRenderPass interface
    bool initialize(const VulkanContext& context) override;
    void execute(vk::CommandBuffer commandBuffer, uint32_t frameIndex) override;
    void cleanup() override;

    /**
     * @brief Set input image for decomposition
     */
    void setInputImage(vk::Image image, vk::ImageView view);

    /**
     * @brief Get output subbands
     */
    const WaveletSubbands& getSubbands() const { return subbands_; }

    /**
     * @brief Update configuration (e.g., foveation level)
     */
    void updateConfig(const WaveletPassConfig& config);

private:
    /**
     * @brief Create output subband images
     */
    bool createSubbandImages(const VulkanContext& context);

    /**
     * @brief Create descriptor sets for compute
     */
    bool createDescriptorSets(const VulkanContext& context);

    /**
     * @brief Load and compile shader
     */
    std::vector<uint32_t> loadShaderSPIRV() const;

    /**
     * @brief Ensure input image/view exist (create dummy if not provided)
     */
    void ensureInputImage(const VulkanContext& context);

    WaveletPassConfig config_;
    WaveletSubbands subbands_;
    
    // Input
    vk::Image inputImage_;
    vk::ImageView inputView_;
    // Dummy input storage (for RAII when we create placeholder)
    core::VMAImage vmaInputImage_;

    // Vulkan context (stored for cleanup)
    const VulkanContext* context_ = nullptr;
    
    // Descriptors
    vk::DescriptorPool descriptorPool_;
    std::vector<vk::DescriptorSet> descriptorSets_;  // Per frame
    
    // Push constants
    struct PushConstants {
        uint32_t inputWidth;
        uint32_t inputHeight;
        float threshold;
        uint32_t foveationLevel;
    } pushConstants_;

    // Chosen image format for subbands/input
    vk::Format subbandFormat_ = vk::Format::eUndefined;
};

} // namespace rendering
} // namespace spectraforge

