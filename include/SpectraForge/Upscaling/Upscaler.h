/**
 * @file Upscaler.h
 * @brief Abstraction for AI upscaling (DLSS/FSR2)
 *
 * Implements SOLID principles:
 * - SRP: Each upscaler handles specific vendor tech
 * - OCP: Open for extension (new upscalers)
 * - LSP: All upscalers substitutable
 * - ISP: Minimal interface
 * - DIP: Factory returns abstractions
 *
 * @author SpectraForge Core Team
 * @date 2025-10-02
 */

#pragma once

#include <vulkan/vulkan.hpp>
#include <memory>
#include <string>

namespace spectraforge {

// Forward declaration
class VulkanContext;

namespace upscaling {

/**
 * @brief Upscaling quality preset
 */
enum class UpscaleQuality {
    PERFORMANCE,    // Lowest input resolution, highest perf
    BALANCED,       // Balance quality/perf
    QUALITY,        // Higher input resolution
    ULTRA_QUALITY   // Highest quality, lowest perf boost
};

/**
 * @brief Upscaling configuration
 */
struct UpscaleConfig {
    uint32_t inputWidth;
    uint32_t inputHeight;
    uint32_t outputWidth;
    uint32_t outputHeight;
    UpscaleQuality quality = UpscaleQuality::BALANCED;
    bool enableSharpening = true;
    float sharpness = 0.5f;  // [0, 1]
};

/**
 * @brief Upscaling input/output
 */
struct UpscaleResources {
    vk::Image inputColor;
    vk::ImageView inputColorView;
    vk::Image inputDepth;
    vk::ImageView inputDepthView;
    vk::Image inputMotionVectors;
    vk::ImageView inputMotionVectorsView;
    vk::Image outputColor;
    vk::ImageView outputColorView;
};

/**
 * @brief Base interface for upscalers (ISP: minimal interface)
 */
class IUpscaler {
public:
    virtual ~IUpscaler() = default;

    /**
     * @brief Initialize upscaler
     */
    virtual bool initialize(const VulkanContext& context,
                           const UpscaleConfig& config) = 0;

    /**
     * @brief Execute upscaling
     */
    virtual void execute(vk::CommandBuffer cmd, const UpscaleResources& resources,
                        uint32_t frameIndex, float jitterX, float jitterY) = 0;

    /**
     * @brief Cleanup resources
     */
    virtual void cleanup() = 0;

    /**
     * @brief Resize (for window resize events)
     */
    virtual bool resize(uint32_t newInputWidth, uint32_t newInputHeight,
                       uint32_t newOutputWidth, uint32_t newOutputHeight) = 0;

    /**
     * @brief Get upscaler name
     */
    virtual const char* getName() const = 0;

    /**
     * @brief Check if initialized
     */
    virtual bool isInitialized() const = 0;

    /**
     * @brief Get recommended jitter sequence for TAA
     */
    virtual void getJitterOffset(uint32_t frameIndex, float& outX, float& outY) const = 0;
};

/**
 * @brief Base implementation with common functionality
 */
class UpscalerBase : public IUpscaler {
public:
    explicit UpscalerBase(std::string name);
    virtual ~UpscalerBase() override = default;

    // Disable copy (RAII)
    UpscalerBase(const UpscalerBase&) = delete;
    UpscalerBase& operator=(const UpscalerBase&) = delete;

    // Enable move
    UpscalerBase(UpscalerBase&&) noexcept = default;
    UpscalerBase& operator=(UpscalerBase&&) noexcept = default;

    const char* getName() const override { return name_.c_str(); }

protected:
    /**
     * @brief Halton sequence for jitter (common for TAA)
     */
    static void haltonSequence(uint32_t index, uint32_t base, float& out);

    std::string name_;
    UpscaleConfig config_;
    bool initialized_ = false;
};

/**
 * @brief Null upscaler (deprecated - use NativeUpscaler)
 */
class NullUpscaler : public UpscalerBase {
public:
    NullUpscaler();
    
    bool initialize(const VulkanContext& context,
                   const UpscaleConfig& config) override;
    void execute(vk::CommandBuffer cmd, const UpscaleResources& resources,
                uint32_t frameIndex, float jitterX, float jitterY) override;
    void cleanup() override;
    bool resize(uint32_t newInputWidth, uint32_t newInputHeight,
               uint32_t newOutputWidth, uint32_t newOutputHeight) override;
    bool isInitialized() const override { return initialized_; }
    void getJitterOffset(uint32_t frameIndex, float& outX, float& outY) const override;
};

} // namespace upscaling
} // namespace spectraforge

