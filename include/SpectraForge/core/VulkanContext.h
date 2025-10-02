/**
 * @file VulkanContext.h
 * @brief Abstraction for Vulkan device and context management
 *
 * Provides interface for passes to access Vulkan resources (DIP).
 * SRP: Single responsibility - Vulkan context management
 *
 * @author SpectraForge Core Team
 * @date 2025-10-02
 */

#pragma once

#include <vulkan/vulkan.hpp>
#include <memory>

namespace spectraforge {

/**
 * @brief Vulkan context interface (DIP: passes depend on this abstraction)
 * 
 * Following ISP: minimal interface for render passes
 */
class VulkanContext {
public:
    virtual ~VulkanContext() = default;

    /**
     * @brief Get Vulkan instance
     */
    virtual vk::Instance getInstance() const = 0;

    /**
     * @brief Get physical device
     */
    virtual vk::PhysicalDevice getPhysicalDevice() const = 0;

    /**
     * @brief Get logical device
     */
    virtual vk::Device getDevice() const = 0;

    /**
     * @brief Get graphics queue
     */
    virtual vk::Queue getGraphicsQueue() const = 0;

    /**
     * @brief Get compute queue (may be same as graphics)
     */
    virtual vk::Queue getComputeQueue() const = 0;

    /**
     * @brief Get command pool
     */
    virtual vk::CommandPool getCommandPool() const = 0;

    /**
     * @brief Get physical device properties
     */
    virtual vk::PhysicalDeviceProperties getPhysicalDeviceProperties() const = 0;

    /**
     * @brief Get physical device memory properties
     */
    virtual vk::PhysicalDeviceMemoryProperties getMemoryProperties() const = 0;

    /**
     * @brief Check if feature is available
     */
    virtual bool isFeatureAvailable(const char* featureName) const = 0;
};

/**
 * @brief Factory function to create VulkanContext
 * @param enableValidation Enable Vulkan validation layers (default: true)
 * @return Initialized VulkanContext, or nullptr on failure
 */
std::unique_ptr<VulkanContext> createVulkanContext(bool enableValidation = true);

} // namespace spectraforge

