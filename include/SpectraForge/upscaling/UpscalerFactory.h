/**
 * @file UpscalerFactory.h
 * @brief Factory for creating upscaler implementations (DLSS, FSR2, etc.)
 *
 * Implements factory pattern for runtime selection of upscaling technology
 * based on hardware capabilities or user preference.
 *
 * @author SpectraForge Core Team
 * @date 2025-10-02
 */

#pragma once

#include "SpectraForge/upscaling/Upscaler.h"
#include <memory>
#include <string>

namespace spectraforge {
namespace upscaling {

/**
 * @brief Upscaler type enum
 */
enum class UpscalerType {
    AUTO,           ///< Automatic selection (DLSS for NVIDIA, FSR2 for others)
    DLSS,           ///< NVIDIA DLSS (requires RTX GPU)
    FSR2,           ///< AMD FidelityFX Super Resolution 2 (vendor-agnostic)
    NONE            ///< No upscaling (native resolution)
};

/**
 * @brief Upscaler configuration
 */
struct UpscalerConfig {
    UpscalerType type = UpscalerType::AUTO;
    UpscaleQuality quality = UpscaleQuality::BALANCED;
    
    uint32_t inputWidth = 0;
    uint32_t inputHeight = 0;
    uint32_t outputWidth = 0;
    uint32_t outputHeight = 0;
    
    bool enableSharpening = true;
    float sharpness = 0.5f;  ///< 0.0 - 1.0
    
    bool enableReactiveMask = false;  ///< FSR2-specific
    bool enableExposure = false;       ///< Auto-exposure support
};

/**
 * @brief Factory for creating upscalers (Factory Pattern)
 * 
 * SRP: Single responsibility - upscaler creation
 * OCP: Open for extension (new upscaler types)
 * DIP: Returns abstract IUpscaler interface
 */
class UpscalerFactory {
public:
    /**
     * @brief Create upscaler based on type and GPU vendor
     * @param config Upscaler configuration
     * @param gpuVendorId Vulkan vendor ID (0 for auto-detect)
     * @return Unique pointer to upscaler, or nullptr if unavailable
     */
    static std::unique_ptr<IUpscaler> create(
        const UpscalerConfig& config,
        uint32_t gpuVendorId = 0
    );
    
    /**
     * @brief Check if upscaler type is available on current hardware
     * @param type Upscaler type
     * @param gpuVendorId Vulkan vendor ID
     * @return true if supported
     */
    static bool isAvailable(UpscalerType type, uint32_t gpuVendorId);
    
    /**
     * @brief Get recommended upscaler for GPU vendor
     * @param gpuVendorId Vulkan vendor ID
     * @return Recommended upscaler type
     */
    static UpscalerType getRecommended(uint32_t gpuVendorId);
    
    /**
     * @brief Get upscaler type name (for logging/UI)
     * @param type Upscaler type
     * @return Human-readable name
     */
    static const char* getTypeName(UpscalerType type);

private:
    /**
     * @brief Check if NVIDIA GPU
     */
    static bool isNVIDIA(uint32_t vendorId) {
        return vendorId == 0x10DE;  // NVIDIA vendor ID
    }
    
    /**
     * @brief Check if AMD GPU
     */
    static bool isAMD(uint32_t vendorId) {
        return vendorId == 0x1002;  // AMD vendor ID
    }
    
    /**
     * @brief Check if Intel GPU
     */
    static bool isIntel(uint32_t vendorId) {
        return vendorId == 0x8086;  // Intel vendor ID
    }
    
    /**
     * @brief Check if ARM Mali GPU
     */
    static bool isARM(uint32_t vendorId) {
        return vendorId == 0x13B5;  // ARM vendor ID
    }
    
    /**
     * @brief Check if Qualcomm Adreno GPU
     */
    static bool isQualcomm(uint32_t vendorId) {
        return vendorId == 0x5143;  // Qualcomm vendor ID
    }
};

/**
 * @brief No-op upscaler (native resolution, pass-through)
 */
class NativeUpscaler : public IUpscaler {
public:
    explicit NativeUpscaler(const UpscalerConfig& config);
    ~NativeUpscaler() override = default;
    
    bool initialize(
        const VulkanContext& context,
        const UpscaleConfig& config
    ) override;
    
    void execute(
        vk::CommandBuffer commandBuffer,
        const UpscaleResources& resources,
        uint32_t frameIndex,
        float jitterX,
        float jitterY
    ) override;
    
    void cleanup() override;
    
    bool resize(
        uint32_t newInputWidth,
        uint32_t newInputHeight,
        uint32_t newOutputWidth,
        uint32_t newOutputHeight
    ) override;
    
    void getJitterOffset(uint32_t frameIndex, float& outX, float& outY) const override;
    
    const char* getName() const override { return "Native (No Upscaling)"; }
    bool isInitialized() const override { return initialized_; }

private:
    bool initialized_ = false;
    uint32_t inputWidth_ = 0;
    uint32_t inputHeight_ = 0;
};

} // namespace upscaling
} // namespace spectraforge

