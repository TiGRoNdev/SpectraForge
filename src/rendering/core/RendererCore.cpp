/**
 * @file RendererCore.cpp
 * @brief Implementation of RendererCore (P0.2 Refactoring)
 * 
 * SOLID COMPLIANCE:
 * - SRP ✅: Только инициализация Vulkan core
 * - RAII ✅: Автоматическая очистка в деструкторе
 */

#include "SpectraForge/Rendering/Core/RendererCore.h"
#include <iostream>
#include <vector>
#include <set>
#include <algorithm>
#include <X11/Xlib.h>
#include <vulkan/vulkan_xlib.h>

namespace SpectraForge {
namespace Rendering {
namespace Core {

RendererCore::~RendererCore() {
    shutdown();
}

bool RendererCore::initialize() {
    if (initialized_) {
        std::cout << "[RendererCore] Already initialized\n";
        return true;
    }
    
    std::cout << "[RendererCore] Initializing Vulkan core...\n";
    
    if (!createInstance()) {
        std::cerr << "[RendererCore] ❌ Failed to create Vulkan instance\n";
        return false;
    }
    
    if (!pickPhysicalDevice()) {
        std::cerr << "[RendererCore] ❌ Failed to pick physical device\n";
        return false;
    }
    
    initialized_ = true;
    std::cout << "[RendererCore] ✅ Vulkan core initialized\n";
    return true;
}

bool RendererCore::createLogicalDeviceWithSurface(vk::SurfaceKHR surface) {
    if (!initialized_) {
        std::cerr << "[RendererCore] ❌ Cannot create logical device: not initialized\n";
        return false;
    }
    
    if (!createLogicalDevice(surface)) {
        std::cerr << "[RendererCore] ❌ Failed to create logical device\n";
        return false;
    }
    
    if (!createAllocator()) {
        std::cerr << "[RendererCore] ❌ Failed to create VMA allocator\n";
        return false;
    }
    
    std::cout << "[RendererCore] ✅ Logical device and allocator created\n";
    return true;
}

void RendererCore::shutdown() {
    if (!initialized_) {
        return;
    }
    
    std::cout << "[RendererCore] Shutting down...\n";
    
    // Cleanup VMA allocator
    if (allocator_ != VK_NULL_HANDLE) {
        vmaDestroyAllocator(allocator_);
        allocator_ = VK_NULL_HANDLE;
    }
    
    // Cleanup logical device
    if (device_) {
        device_.destroy();
        device_ = vk::Device{};
    }
    
    // Cleanup instance
    if (instance_) {
        instance_.destroy();
        instance_ = vk::Instance{};
    }
    
    initialized_ = false;
    std::cout << "[RendererCore] ✅ Shutdown complete\n";
}

bool RendererCore::createInstance() {
    vk::ApplicationInfo appInfo;
    appInfo.pApplicationName = "SpectraForge";
    appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.pEngineName = "SpectraForge Engine";
    appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.apiVersion = VK_API_VERSION_1_3;
    
    // Required extensions
    std::vector<const char*> extensions = {
        VK_KHR_SURFACE_EXTENSION_NAME,
        VK_KHR_XLIB_SURFACE_EXTENSION_NAME,
        VK_EXT_DEBUG_UTILS_EXTENSION_NAME
    };
    
    // Validation layers
    std::vector<const char*> validationLayers = {
        "VK_LAYER_KHRONOS_validation"
    };
    
    vk::InstanceCreateInfo createInfo;
    createInfo.pApplicationInfo = &appInfo;
    createInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
    createInfo.ppEnabledExtensionNames = extensions.data();
    createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
    createInfo.ppEnabledLayerNames = validationLayers.data();
    
    try {
        instance_ = vk::createInstance(createInfo);
        std::cout << "[RendererCore] ✅ Vulkan instance created with validation layers\n";
        return true;
    } catch (const std::exception& e) {
        std::cerr << "[RendererCore] ❌ Failed to create instance: " << e.what() << "\n";
        return false;
    }
}

bool RendererCore::pickPhysicalDevice() {
    auto devices = instance_.enumeratePhysicalDevices();
    
    if (devices.empty()) {
        std::cerr << "[RendererCore] ❌ No Vulkan devices found\n";
        return false;
    }
    
    // Pick the first discrete GPU or fallback to first device
    for (const auto& device : devices) {
        auto props = device.getProperties();
        if (props.deviceType == vk::PhysicalDeviceType::eDiscreteGpu) {
            physicalDevice_ = device;
            std::cout << "[RendererCore] ✅ Selected GPU: " << props.deviceName << "\n";
            return true;
        }
    }
    
    physicalDevice_ = devices[0];
    auto props = physicalDevice_.getProperties();
    std::cout << "[RendererCore] ✅ Selected device: " << props.deviceName << "\n";
    return true;
}

bool RendererCore::createLogicalDevice(vk::SurfaceKHR surface) {
    // Find queue families
    auto queueFamilies = physicalDevice_.getQueueFamilyProperties();
    
    uint32_t graphicsFamily = UINT32_MAX;
    uint32_t presentFamily = UINT32_MAX;
    
    for (uint32_t i = 0; i < queueFamilies.size(); i++) {
        if (queueFamilies[i].queueFlags & vk::QueueFlagBits::eGraphics) {
            graphicsFamily = i;
        }
        
        // Check present support
        if (surface) {
            VkBool32 presentSupport = false;
            vkGetPhysicalDeviceSurfaceSupportKHR(physicalDevice_, i, surface, &presentSupport);
            if (presentSupport) {
                presentFamily = i;
            }
        } else {
            // Если surface не передан (тесты), используем graphics queue
            presentFamily = graphicsFamily;
        }
        
        if (graphicsFamily != UINT32_MAX && presentFamily != UINT32_MAX) {
            break;
        }
    }
    
    if (graphicsFamily == UINT32_MAX || presentFamily == UINT32_MAX) {
        std::cerr << "[RendererCore] ❌ Failed to find suitable queue families\n";
        return false;
    }
    
    graphicsQueueFamily_ = graphicsFamily;
    presentQueueFamily_ = presentFamily;
    
    // Create queues
    std::set<uint32_t> uniqueQueueFamilies = {graphicsFamily, presentFamily};
    std::vector<vk::DeviceQueueCreateInfo> queueCreateInfos;
    
    float queuePriority = 1.0f;
    for (uint32_t queueFamily : uniqueQueueFamilies) {
        vk::DeviceQueueCreateInfo queueCreateInfo;
        queueCreateInfo.queueFamilyIndex = queueFamily;
        queueCreateInfo.queueCount = 1;
        queueCreateInfo.pQueuePriorities = &queuePriority;
        queueCreateInfos.push_back(queueCreateInfo);
    }
    
    // Device features
    vk::PhysicalDeviceFeatures deviceFeatures;
    
    // Device extensions
    std::vector<const char*> deviceExtensions = {
        VK_KHR_SWAPCHAIN_EXTENSION_NAME
    };
    
    vk::DeviceCreateInfo createInfo;
    createInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
    createInfo.pQueueCreateInfos = queueCreateInfos.data();
    createInfo.pEnabledFeatures = &deviceFeatures;
    createInfo.enabledExtensionCount = static_cast<uint32_t>(deviceExtensions.size());
    createInfo.ppEnabledExtensionNames = deviceExtensions.data();
    
    try {
        device_ = physicalDevice_.createDevice(createInfo);
        graphicsQueue_ = device_.getQueue(graphicsFamily, 0);
        presentQueue_ = device_.getQueue(presentFamily, 0);
        std::cout << "[RendererCore] ✅ Logical device created\n";
        return true;
    } catch (const std::exception& e) {
        std::cerr << "[RendererCore] ❌ Failed to create logical device: " << e.what() << "\n";
        return false;
    }
}

bool RendererCore::createAllocator() {
    VmaAllocatorCreateInfo allocatorInfo{};
    allocatorInfo.vulkanApiVersion = VK_API_VERSION_1_3;
    allocatorInfo.physicalDevice = physicalDevice_;
    allocatorInfo.device = device_;
    allocatorInfo.instance = instance_;
    
    if (vmaCreateAllocator(&allocatorInfo, &allocator_) != VK_SUCCESS) {
        std::cerr << "[RendererCore] ❌ Failed to create VMA allocator\n";
        return false;
    }
    
    std::cout << "[RendererCore] ✅ VMA allocator created\n";
    return true;
}

}  // namespace Core
}  // namespace Rendering
}  // namespace SpectraForge

