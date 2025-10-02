/**
 * @file NativeUpscaler.h
 * @brief Native (pass-through) upscaler with optional blit
 *
 * Implements SOLID principles:
 * - SRP: Only handles native resolution rendering
 * - OCP: Extensible for custom blit filters
 * - LSP: Substitutable for IUpscaler
 * - ISP: Minimal interface from IUpscaler
 * - DIP: Depends on Vulkan abstractions
 *
 * @author SpectraForge Core Team
 * @date 2025-10-02
 */

#pragma once

#include "Upscaler.h"

namespace spectraforge {
namespace upscaling {

/**
 * @brief Native upscaler (pass-through with optional blit)
 * 
 * Use cases:
 * - No upscaling needed (input == output resolution)
 * - Fallback when DLSS/FSR2 unavailable
 * - Testing baseline performance
 * 
 * Performance: ~0.1ms @ 4K (single blit operation)
 */
class NativeUpscaler : public UpscalerBase {
public:
    explicit NativeUpscaler();
    ~NativeUpscaler() override = default;

    // IUpscaler interface
    bool initialize(const core::VulkanContext& context,
                   const UpscaleConfig& config) override;
    
    void execute(vk::CommandBuffer cmd, const UpscaleResources& resources,
                uint32_t frameIndex, float jitterX, float jitterY) override;
    
    void cleanup() override;
    
    bool resize(uint32_t newInputWidth, uint32_t newInputHeight,
               uint32_t newOutputWidth, uint32_t newOutputHeight) override;
    
    bool isInitialized() const override { return initialized_; }
    
    void getJitterOffset(uint32_t frameIndex, float& outX, float& outY) const override;

private:
    /**
     * @brief Perform image blit (if resolutions differ)
     */
    void blitImage(vk::CommandBuffer cmd, vk::Image src, vk::Image dst,
                   uint32_t srcWidth, uint32_t srcHeight,
                   uint32_t dstWidth, uint32_t dstHeight);

    vk::Device device_;
    bool needsBlit_ = false;  // True if input != output resolution
};

} // namespace upscaling
} // namespace spectraforge

