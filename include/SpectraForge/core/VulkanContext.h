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
 * @brief Concrete Vulkan context implementation
 */
class VulkanContextImpl : public VulkanContext {
public:
    VulkanContextImpl() = default;
    ~VulkanContextImpl() override = default;

    // Implementation will be in VulkanContext.cpp
    vk::Instance getInstance() const override;
    vk::PhysicalDevice getPhysicalDevice() const override;
    vk::Device getDevice() const override;
    vk::Queue getGraphicsQueue() const override;
    vk::Queue getComputeQueue() const override;
    vk::CommandPool getCommandPool() const override;
    vk::PhysicalDeviceProperties getPhysicalDeviceProperties() const override;
    vk::PhysicalDeviceMemoryProperties getMemoryProperties() const override;
    bool isFeatureAvailable(const char* featureName) const override;

private:
    // TODO: Add actual Vulkan handles
    vk::Instance instance_;
    vk::PhysicalDevice physicalDevice_;
    vk::Device device_;
    vk::Queue graphicsQueue_;
    vk::Queue computeQueue_;
    vk::CommandPool commandPool_;
};

} // namespace spectraforge

