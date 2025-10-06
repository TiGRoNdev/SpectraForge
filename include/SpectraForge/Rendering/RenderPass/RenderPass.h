/**
 * @file RenderPass.h
 * @brief Base interface for render passes in Hybrid DWT + FreGS pipeline
 *
 * Implements SOLID principles:
 * - SRP: Each pass has single responsibility
 * - OCP: Open for extension via inheritance
 * - LSP: Derived passes can substitute base
 * - ISP: Minimal interface (no fat interfaces)
 * - DIP: Depends on abstractions (VulkanContext interface)
 *
 * @author SpectraForge Core Team
 * @date 2025-10-02
 */

#pragma once

#include <vulkan/vulkan.hpp>
#include <memory>
#include <string>

namespace spectraforge {

// Forward declaration from parent namespace
class VulkanContext;

namespace rendering {

/**
 * @brief Forward declarations
 */
class ResourceManager;
// VulkanContext is in parent namespace spectraforge

/**
 * @brief Execution statistics for performance monitoring
 */
struct PassStatistics {
    double executionTimeMs;
    uint64_t memoryUsedBytes;
    uint32_t dispatchCount;
};

/**
 * @brief Base interface for all render passes
 * 
 * Following ISP (Interface Segregation Principle): minimal, focused interface
 */
class IRenderPass {
public:
    virtual ~IRenderPass() = default;

    /**
     * @brief Initialize pass resources (pipelines, descriptors, etc.)
     * @param context Vulkan context (DIP: depends on abstraction)
     * @return true on success
     */
    virtual bool initialize(const VulkanContext& context) = 0;

    /**
     * @brief Execute the pass
     * @param commandBuffer Command buffer to record commands
     * @param frameIndex Current frame index (for double/triple buffering)
     */
    virtual void execute(vk::CommandBuffer commandBuffer, uint32_t frameIndex) = 0;

    /**
     * @brief Cleanup resources
     */
    virtual void cleanup() = 0;

    /**
     * @brief Get pass name for debugging
     */
    virtual const char* getName() const = 0;

    /**
     * @brief Get execution statistics (optional, for profiling)
     */
    virtual PassStatistics getStatistics() const {
        return PassStatistics{0.0, 0, 0};
    }

    /**
     * @brief Check if pass is initialized
     */
    virtual bool isInitialized() const = 0;
};

/**
 * @brief Base implementation with common functionality
 * 
 * Following SRP: handles common pass operations
 * Following RAII: automatic resource management
 */
class RenderPassBase : public IRenderPass {
public:
    explicit RenderPassBase(std::string name);
    virtual ~RenderPassBase() override;

    // Disable copy (RAII: unique ownership)
    RenderPassBase(const RenderPassBase&) = delete;
    RenderPassBase& operator=(const RenderPassBase&) = delete;

    // Enable move (modern C++)
    RenderPassBase(RenderPassBase&&) noexcept = default;
    RenderPassBase& operator=(RenderPassBase&&) noexcept = default;

    const char* getName() const override { return name_.c_str(); }
    bool isInitialized() const override { return initialized_; }
    PassStatistics getStatistics() const override { return statistics_; }

protected:
    /**
     * @brief Create compute pipeline from SPIR-V
     */
    vk::Pipeline createComputePipeline(
        const VulkanContext& context,
        const std::vector<uint32_t>& spirvCode,
        vk::PipelineLayout layout
    );

    /**
     * @brief Create descriptor set layout
     */
    vk::DescriptorSetLayout createDescriptorSetLayout(
        const VulkanContext& context,
        const std::vector<vk::DescriptorSetLayoutBinding>& bindings
    );

    /**
     * @brief Record timestamp for profiling
     */
    void recordTimestamp(vk::CommandBuffer cmd, vk::PipelineStageFlagBits stage);

    std::string name_;
    bool initialized_ = false;
    PassStatistics statistics_;
    
    // Vulkan resources (unique ownership via RAII)
    vk::Pipeline pipeline_;
    vk::PipelineLayout pipelineLayout_;
    vk::DescriptorSetLayout descriptorSetLayout_;
};

/**
 * @brief Factory for creating render passes
 * 
 * Following DIP: factory returns interfaces, not concrete types
 * Following OCP: can be extended with new pass types
 */
class RenderPassFactory {
public:
    enum class PassType {
        WAVELET_LIFTING,
        FREQUENCY_GAUSSIAN_SPLAT,
        FOVEATION_ALIGN,
        TEMPORAL_REPROJECTION,
        UPSCALING_DLSS,
        UPSCALING_FSR2
    };

    /**
     * @brief Create a render pass by type
     * @return Unique pointer to interface (DIP)
     */
    static std::unique_ptr<IRenderPass> createPass(PassType type);
};

} // namespace rendering
} // namespace spectraforge

