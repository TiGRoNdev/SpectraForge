/**
 * @file SwapchainManager.cpp
 * @brief Implementation of SwapchainManager (P0.2)
 */

#include "SpectraForge/Rendering/Core/SwapchainManager.h"
#include <iostream>
#include <algorithm>
#include <X11/Xlib.h>
#include <vulkan/vulkan_xlib.h>

namespace SpectraForge {
namespace Rendering {
namespace Core {

SwapchainManager::SwapchainManager(vk::Instance instance,
                                   vk::PhysicalDevice physicalDevice,
                                   vk::Device device)
    : instance_(instance)
    , physicalDevice_(physicalDevice)
    , device_(device) {
}

SwapchainManager::~SwapchainManager() {
    shutdown();
}

bool SwapchainManager::createSurfaceX11(void* x11Display, void* x11Window) {
    VkXlibSurfaceCreateInfoKHR createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_XLIB_SURFACE_CREATE_INFO_KHR;
    createInfo.dpy = static_cast<Display*>(x11Display);
    createInfo.window = reinterpret_cast<Window>(x11Window);
    
    VkSurfaceKHR surface;
    VkResult result = vkCreateXlibSurfaceKHR(instance_, &createInfo, nullptr, &surface);
    
    if (result != VK_SUCCESS) {
        std::cerr << "[SwapchainManager] ❌ Failed to create Xlib surface: error " << result << "\n";
        return false;
    }
    
    surface_ = surface;
    std::cout << "[SwapchainManager] ✅ X11 Vulkan surface created\n";
    return true;
}

bool SwapchainManager::createSwapchain(uint32_t width, uint32_t height,
                                      uint32_t graphicsQueueFamily,
                                      uint32_t presentQueueFamily) {
    if (!surface_) {
        std::cerr << "[SwapchainManager] ❌ Cannot create swapchain: surface not created\n";
        return false;
    }
    
    return createSwapchainAndViews(width, height, graphicsQueueFamily, presentQueueFamily);
}

void SwapchainManager::destroySwapchain() {
    if (device_) {
        for (auto imageView : swapchainImageViews_) {
            device_.destroyImageView(imageView);
        }
        swapchainImageViews_.clear();
        
        if (swapchain_) {
            device_.destroySwapchainKHR(swapchain_);
            swapchain_ = vk::SwapchainKHR{};
        }
    }
}

void SwapchainManager::shutdown() {
    destroySwapchain();
    
    if (instance_ && surface_) {
        instance_.destroySurfaceKHR(surface_);
        surface_ = vk::SurfaceKHR{};
    }
    
    std::cout << "[SwapchainManager] ✅ Shutdown complete\n";
}

bool SwapchainManager::createSwapchainAndViews(uint32_t width, uint32_t height,
                                               uint32_t graphicsQueueFamily,
                                               uint32_t presentQueueFamily) {
    // Query surface capabilities
    auto capabilities = physicalDevice_.getSurfaceCapabilitiesKHR(surface_);
    auto formats = physicalDevice_.getSurfaceFormatsKHR(surface_);
    auto presentModes = physicalDevice_.getSurfacePresentModesKHR(surface_);
    
    if (formats.empty() || presentModes.empty()) {
        std::cerr << "[SwapchainManager] ❌ Insufficient swapchain support\n";
        return false;
    }
    
    // Choose format
    vk::SurfaceFormatKHR surfaceFormat = chooseSurfaceFormat(formats);
    swapchainFormat_ = surfaceFormat.format;
    
    // Choose present mode
    vk::PresentModeKHR presentMode = choosePresentMode(presentModes);
    
    // Choose extent
    swapchainExtent_ = chooseExtent(width, height, capabilities);
    
    uint32_t imageCount = capabilities.minImageCount + 1;
    if (capabilities.maxImageCount > 0 && imageCount > capabilities.maxImageCount) {
        imageCount = capabilities.maxImageCount;
    }
    
    vk::SwapchainCreateInfoKHR createInfo;
    createInfo.surface = surface_;
    createInfo.minImageCount = imageCount;
    createInfo.imageFormat = surfaceFormat.format;
    createInfo.imageColorSpace = surfaceFormat.colorSpace;
    createInfo.imageExtent = swapchainExtent_;
    createInfo.imageArrayLayers = 1;
    createInfo.imageUsage = vk::ImageUsageFlagBits::eColorAttachment | vk::ImageUsageFlagBits::eTransferDst;
    
    uint32_t queueFamilyIndices[] = {graphicsQueueFamily, presentQueueFamily};
    if (graphicsQueueFamily != presentQueueFamily) {
        createInfo.imageSharingMode = vk::SharingMode::eConcurrent;
        createInfo.queueFamilyIndexCount = 2;
        createInfo.pQueueFamilyIndices = queueFamilyIndices;
    } else {
        createInfo.imageSharingMode = vk::SharingMode::eExclusive;
    }
    
    createInfo.preTransform = capabilities.currentTransform;
    createInfo.compositeAlpha = vk::CompositeAlphaFlagBitsKHR::eOpaque;
    createInfo.presentMode = presentMode;
    createInfo.clipped = VK_TRUE;
    
    try {
        swapchain_ = device_.createSwapchainKHR(createInfo);
        swapchainImages_ = device_.getSwapchainImagesKHR(swapchain_);
        
        // Create image views
        swapchainImageViews_.resize(swapchainImages_.size());
        for (size_t i = 0; i < swapchainImages_.size(); i++) {
            vk::ImageViewCreateInfo viewInfo;
            viewInfo.image = swapchainImages_[i];
            viewInfo.viewType = vk::ImageViewType::e2D;
            viewInfo.format = swapchainFormat_;
            viewInfo.components.r = vk::ComponentSwizzle::eIdentity;
            viewInfo.components.g = vk::ComponentSwizzle::eIdentity;
            viewInfo.components.b = vk::ComponentSwizzle::eIdentity;
            viewInfo.components.a = vk::ComponentSwizzle::eIdentity;
            viewInfo.subresourceRange.aspectMask = vk::ImageAspectFlagBits::eColor;
            viewInfo.subresourceRange.baseMipLevel = 0;
            viewInfo.subresourceRange.levelCount = 1;
            viewInfo.subresourceRange.baseArrayLayer = 0;
            viewInfo.subresourceRange.layerCount = 1;
            
            swapchainImageViews_[i] = device_.createImageView(viewInfo);
        }
        
        std::cout << "[SwapchainManager] ✅ Swapchain created: " << swapchainExtent_.width << "x" 
                  << swapchainExtent_.height << " (" << swapchainImages_.size() << " images)\n";
        return true;
    } catch (const std::exception& e) {
        std::cerr << "[SwapchainManager] ❌ Failed to create swapchain: " << e.what() << "\n";
        return false;
    }
}

vk::SurfaceFormatKHR SwapchainManager::chooseSurfaceFormat(
    const std::vector<vk::SurfaceFormatKHR>& availableFormats) {
    for (const auto& format : availableFormats) {
        if (format.format == vk::Format::eB8G8R8A8Srgb &&
            format.colorSpace == vk::ColorSpaceKHR::eSrgbNonlinear) {
            return format;
        }
    }
    return availableFormats[0];
}

vk::PresentModeKHR SwapchainManager::choosePresentMode(
    const std::vector<vk::PresentModeKHR>& availableModes) {
    for (const auto& mode : availableModes) {
        if (mode == vk::PresentModeKHR::eMailbox) {
            return mode;
        }
    }
    return vk::PresentModeKHR::eFifo;
}

vk::Extent2D SwapchainManager::chooseExtent(uint32_t width, uint32_t height,
                                            const vk::SurfaceCapabilitiesKHR& capabilities) {
    if (capabilities.currentExtent.width != UINT32_MAX) {
        return capabilities.currentExtent;
    }
    
    vk::Extent2D extent{width, height};
    extent.width = std::clamp(extent.width,
        capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
    extent.height = std::clamp(extent.height,
        capabilities.minImageExtent.height, capabilities.maxImageExtent.height);
    return extent;
}

}  // namespace Core
}  // namespace Rendering
}  // namespace SpectraForge

