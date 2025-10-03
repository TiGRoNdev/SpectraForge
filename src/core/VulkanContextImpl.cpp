/**
 * @file VulkanContextImpl.cpp
 * @brief Full implementation of Vulkan context initialization and management
 *
 * Implements device selection, queue creation, and extension/layer management.
 * Follows Vulkan best practices for modern GPU programming.
 *
 * @author SpectraForge Core Team
 * @date 2025-10-02
 */

#ifndef VULKAN_HPP_DISPATCH_LOADER_DYNAMIC
#define VULKAN_HPP_DISPATCH_LOADER_DYNAMIC 1
#endif
#include <vulkan/vulkan.hpp>
#include "SpectraForge/core/VulkanContext.h"
#include "SpectraForge/core/VMAMemoryManager.h"
#include <iostream>
#include <set>
#include <map>
#include <stdexcept>
#include <vector>
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <string>
#include <cstring>
#include <cstdlib>

// Fallback defines for platform surface extension names when headers don't provide them
#ifndef VK_KHR_WAYLAND_SURFACE_EXTENSION_NAME
#define VK_KHR_WAYLAND_SURFACE_EXTENSION_NAME "VK_KHR_wayland_surface"
#endif
#ifndef VK_KHR_XCB_SURFACE_EXTENSION_NAME
#define VK_KHR_XCB_SURFACE_EXTENSION_NAME "VK_KHR_xcb_surface"
#endif
#ifndef VK_KHR_XLIB_SURFACE_EXTENSION_NAME
#define VK_KHR_XLIB_SURFACE_EXTENSION_NAME "VK_KHR_xlib_surface"
#endif
#ifndef VK_KHR_WIN32_SURFACE_EXTENSION_NAME
#define VK_KHR_WIN32_SURFACE_EXTENSION_NAME "VK_KHR_win32_surface"
#endif

namespace spectraforge {

// ============================================================================
// Helper Functions
// ============================================================================

namespace {

/**
 * @brief Check if validation layers are available
 */
bool checkValidationLayerSupport(const std::vector<const char*>& validationLayers) {
    auto availableLayers = vk::enumerateInstanceLayerProperties();
    
    for (const char* layerName : validationLayers) {
        bool layerFound = false;
        
        for (const auto& layerProperties : availableLayers) {
            if (strcmp(layerName, layerProperties.layerName) == 0) {
                layerFound = true;
                break;
            }
        }
        
        if (!layerFound) {
            return false;
        }
    }
    
    return true;
}

/**
 * @brief Rate physical device suitability
 * @return Score (higher is better), 0 means unsuitable
 */
uint32_t rateDeviceSuitability(vk::PhysicalDevice device) {
    auto properties = device.getProperties();
    auto features = device.getFeatures();
    
    uint32_t score = 0;
    
    // Discrete GPU has huge advantage
    if (properties.deviceType == vk::PhysicalDeviceType::eDiscreteGpu) {
        score += 10000;
    } else if (properties.deviceType == vk::PhysicalDeviceType::eIntegratedGpu) {
        score += 5000;
    }
    
    // Maximum texture size
    score += properties.limits.maxImageDimension2D;
    
    // Required features
    if (!features.geometryShader) {
        return 0;  // Must have geometry shader
    }
    
    if (!features.samplerAnisotropy) {
        score /= 2;  // Prefer anisotropic filtering
    }
    
    return score;
}

/**
 * @brief Check if device supports required extensions
 */
bool checkDeviceExtensionSupport(vk::PhysicalDevice device, const std::vector<const char*>& extensions) {
    auto availableExtensions = device.enumerateDeviceExtensionProperties();
    
    std::set<std::string> requiredExtensions(extensions.begin(), extensions.end());
    
    for (const auto& extension : availableExtensions) {
        requiredExtensions.erase(extension.extensionName);
    }
    
    return requiredExtensions.empty();
}

/**
 * @brief Find queue families
 */
struct QueueFamilyIndices {
    std::optional<uint32_t> graphicsFamily;
    std::optional<uint32_t> computeFamily;
    std::optional<uint32_t> transferFamily;
    
    bool isComplete() const {
        return graphicsFamily.has_value() && computeFamily.has_value();
    }
};

QueueFamilyIndices findQueueFamilies(vk::PhysicalDevice device) {
    QueueFamilyIndices indices;
    
    auto queueFamilies = device.getQueueFamilyProperties();
    
    uint32_t i = 0;
    for (const auto& queueFamily : queueFamilies) {
        // Graphics queue
        if (queueFamily.queueFlags & vk::QueueFlagBits::eGraphics) {
            indices.graphicsFamily = i;
        }
        
        // Dedicated compute queue (prefer separate from graphics)
        if ((queueFamily.queueFlags & vk::QueueFlagBits::eCompute) &&
            !(queueFamily.queueFlags & vk::QueueFlagBits::eGraphics)) {
            indices.computeFamily = i;
        }
        
        // Transfer queue
        if (queueFamily.queueFlags & vk::QueueFlagBits::eTransfer) {
            indices.transferFamily = i;
        }
        
        i++;
    }
    
    // Fallback: use graphics queue for compute if no dedicated queue
    if (!indices.computeFamily.has_value() && indices.graphicsFamily.has_value()) {
        indices.computeFamily = indices.graphicsFamily;
    }
    
    return indices;
}

} // anonymous namespace

// ============================================================================
// VulkanContextImpl Implementation
// ============================================================================

class VulkanContextImpl : public VulkanContext {
public:
    VulkanContextImpl() = default;
    ~VulkanContextImpl() override {
        cleanup();
    }
    
    bool initialize(bool enableValidation = true) {
        if (initialized_) {
            std::cerr << "VulkanContext already initialized\n";
            return true;
        }
        
        try {
            enableValidation_ = enableValidation;
            
            createInstance();
            pickPhysicalDevice();
            createLogicalDevice();
            initializeVMA();
            
            initialized_ = true;
            std::cout << "VulkanContext initialized successfully\n";
            return true;
            
        } catch (const std::exception& e) {
            std::cerr << "VulkanContext initialization failed: " << e.what() << "\n";
            cleanup();
            return false;
        }
    }
    
    void cleanup() {
        if (!initialized_) {
            return;
        }
        
        // Cleanup VMA first
        core::VMAMemoryManager::getInstance().cleanup();
        
        // Destroy logical device
        if (device_) {
            device_.destroy();
            device_ = nullptr;
        }
        
        // Destroy instance
        if (instance_) {
            instance_.destroy();
            instance_ = nullptr;
        }
        
        initialized_ = false;
        std::cout << "VulkanContext cleaned up\n";
    }
    
    // Interface implementation
    vk::Instance getInstance() const override { return instance_; }
    vk::PhysicalDevice getPhysicalDevice() const override { return physicalDevice_; }
    vk::Device getDevice() const override { return device_; }
    vk::Queue getGraphicsQueue() const override { return graphicsQueue_; }
    vk::Queue getComputeQueue() const override { return computeQueue_; }
    vk::CommandPool getCommandPool() const override { return commandPool_; }
    
    vk::PhysicalDeviceProperties getPhysicalDeviceProperties() const override {
        return physicalDevice_.getProperties();
    }
    
    vk::PhysicalDeviceMemoryProperties getMemoryProperties() const override {
        return physicalDevice_.getMemoryProperties();
    }
    
    bool isFeatureAvailable(const char* featureName) const override {
        // Check extensions
        auto extensions = physicalDevice_.enumerateDeviceExtensionProperties();
        for (const auto& ext : extensions) {
            if (strcmp(ext.extensionName, featureName) == 0) {
                return true;
            }
        }
        return false;
    }

private:
    void createInstance() {
        // Инициализируем глобальные функции до любых вызовов vk::*
        VULKAN_HPP_DEFAULT_DISPATCHER.init(vkGetInstanceProcAddr);

        vk::ApplicationInfo appInfo;
        appInfo.pApplicationName = "SpectraForge";
        appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
        appInfo.pEngineName = "SpectraForge Engine";
        appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
        appInfo.apiVersion = VK_API_VERSION_1_3;
        
        vk::InstanceCreateInfo createInfo;
        createInfo.pApplicationInfo = &appInfo;
        
        // Extensions - используем GLFW для получения правильных расширений
        std::vector<const char*> extensions = {
            VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME
        };

        // Получаем требуемые расширения от GLFW (включает VK_KHR_surface и платформо-специфичные)
        uint32_t glfwExtensionCount = 0;
        const char** glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);
        
        if (glfwExtensions && glfwExtensionCount > 0) {
            for (uint32_t i = 0; i < glfwExtensionCount; ++i) {
                extensions.push_back(glfwExtensions[i]);
            }
            std::cout << "GLFW предоставил " << glfwExtensionCount << " Vulkan расширений\n";
        } else {
            throw std::runtime_error(
                "GLFW не смог предоставить расширения Vulkan! "
                "Убедитесь, что:\n"
                "  1) GLFW скомпилирован с поддержкой Vulkan\n"
                "  2) Установлены драйверы Vulkan (пакет vulkan-loader или mesa-vulkan-drivers)\n"
                "  3) Окно создано с GLFW_CLIENT_API = GLFW_NO_API\n"
                "  4) glfwVulkanSupported() возвращает true"
            );
        }
        
        createInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
        createInfo.ppEnabledExtensionNames = extensions.data();
        
        // Validation layers
        std::vector<const char*> validationLayers;
        if (enableValidation_) {
            validationLayers = { "VK_LAYER_KHRONOS_validation" };
            
            if (!checkValidationLayerSupport(validationLayers)) {
                std::cerr << "Validation layers requested but not available\n";
                enableValidation_ = false;
            } else {
                createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
                createInfo.ppEnabledLayerNames = validationLayers.data();
            }
        }
        
        instance_ = vk::createInstance(createInfo);

        std::cout << "Vulkan instance created (API 1.3)\n";
        // Initialize dispatcher with instance
        VULKAN_HPP_DEFAULT_DISPATCHER.init(instance_);
    }
    
    void pickPhysicalDevice() {
        auto devices = instance_.enumeratePhysicalDevices();
        
        if (devices.empty()) {
            throw std::runtime_error("No Vulkan-capable GPUs found");
        }
        
        // Rate all devices
        std::multimap<uint32_t, vk::PhysicalDevice> candidates;
        
        for (const auto& device : devices) {
            uint32_t score = rateDeviceSuitability(device);
            if (score > 0) {
                candidates.insert(std::make_pair(score, device));
            }
        }
        
        if (candidates.empty()) {
            throw std::runtime_error("No suitable GPU found");
        }
        
        // Pick best device
        physicalDevice_ = candidates.rbegin()->second;
        
        auto properties = physicalDevice_.getProperties();
        std::cout << "Selected GPU: " << properties.deviceName << "\n";
        std::cout << "Driver Version: " << properties.driverVersion << "\n";
        std::cout << "API Version: " << VK_VERSION_MAJOR(properties.apiVersion) << "."
                  << VK_VERSION_MINOR(properties.apiVersion) << "."
                  << VK_VERSION_PATCH(properties.apiVersion) << "\n";
    }
    
    void createLogicalDevice() {
        QueueFamilyIndices indices = findQueueFamilies(physicalDevice_);
        
        if (!indices.isComplete()) {
            throw std::runtime_error("Required queue families not available");
        }
        
        // Queue create infos
        std::vector<vk::DeviceQueueCreateInfo> queueCreateInfos;
        std::set<uint32_t> uniqueQueueFamilies = {
            indices.graphicsFamily.value(),
            indices.computeFamily.value()
        };
        
        float queuePriority = 1.0f;
        for (uint32_t queueFamily : uniqueQueueFamilies) {
            vk::DeviceQueueCreateInfo queueCreateInfo;
            queueCreateInfo.queueFamilyIndex = queueFamily;
            queueCreateInfo.queueCount = 1;
            queueCreateInfo.pQueuePriorities = &queuePriority;
            queueCreateInfos.push_back(queueCreateInfo);
        }
        
        // Device features
        vk::PhysicalDeviceFeatures supported = physicalDevice_.getFeatures();
        vk::PhysicalDeviceFeatures deviceFeatures{};
        deviceFeatures.samplerAnisotropy = supported.samplerAnisotropy;
        deviceFeatures.geometryShader = supported.geometryShader;
        // Enable extended storage image formats only if supported
        deviceFeatures.shaderStorageImageExtendedFormats = supported.shaderStorageImageExtendedFormats;

        // Query supported Vulkan 1.2 features (for shaderFloat16)
        vk::PhysicalDeviceFeatures2 features2{};
        vk::PhysicalDeviceVulkan12Features supported12{};
        features2.pNext = &supported12;
        physicalDevice_.getFeatures2(&features2);

        // Vulkan 1.3 features
        vk::PhysicalDeviceVulkan13Features features13;
        features13.synchronization2 = VK_TRUE;
        features13.dynamicRendering = VK_TRUE;
        
        // Vulkan 1.2 features
        vk::PhysicalDeviceVulkan12Features features12{};
        features12.bufferDeviceAddress = VK_TRUE;
        features12.descriptorIndexing = VK_TRUE;
        // Enable fp16 arithmetic if supported to satisfy SPIR-V Capability Float16
        features12.shaderFloat16 = supported12.shaderFloat16;
        features12.shaderInt8 = supported12.shaderInt8;
        features13.pNext = &features12;
        
        // Device extensions
        std::vector<const char*> deviceExtensions = {
            VK_KHR_SWAPCHAIN_EXTENSION_NAME
        };
        
        // Device create info
        vk::DeviceCreateInfo createInfo;
        createInfo.pNext = &features13;
        createInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
        createInfo.pQueueCreateInfos = queueCreateInfos.data();
        createInfo.pEnabledFeatures = &deviceFeatures;
        createInfo.enabledExtensionCount = static_cast<uint32_t>(deviceExtensions.size());
        createInfo.ppEnabledExtensionNames = deviceExtensions.data();
        
        device_ = physicalDevice_.createDevice(createInfo);
        
        // Get queues
        graphicsQueue_ = device_.getQueue(indices.graphicsFamily.value(), 0);
        computeQueue_ = device_.getQueue(indices.computeFamily.value(), 0);
        
        graphicsQueueFamily_ = indices.graphicsFamily.value();
        computeQueueFamily_ = indices.computeFamily.value();
        
        // Create command pool
        vk::CommandPoolCreateInfo poolInfo;
        poolInfo.flags = vk::CommandPoolCreateFlagBits::eResetCommandBuffer;
        poolInfo.queueFamilyIndex = indices.graphicsFamily.value();
        
        commandPool_ = device_.createCommandPool(poolInfo);
        
        std::cout << "Logical device created\n";
        std::cout << "Graphics Queue Family: " << graphicsQueueFamily_ << "\n";
        std::cout << "Compute Queue Family: " << computeQueueFamily_ << "\n";
        // Initialize dispatcher with device
        VULKAN_HPP_DEFAULT_DISPATCHER.init(device_);
    }
    
    void initializeVMA() {
        core::VMAMemoryManager::getInstance().initialize(
            instance_,
            physicalDevice_,
            device_,
            VK_API_VERSION_1_3
        );
    }
    
    vk::Instance instance_;
    vk::PhysicalDevice physicalDevice_;
    vk::Device device_;
    vk::Queue graphicsQueue_;
    vk::Queue computeQueue_;
    vk::CommandPool commandPool_;
    
    uint32_t graphicsQueueFamily_ = 0;
    uint32_t computeQueueFamily_ = 0;
    
    bool initialized_ = false;
    bool enableValidation_ = true;
};

// ============================================================================
// Factory Function
// ============================================================================

std::unique_ptr<VulkanContext> createVulkanContext(bool enableValidation) {
    auto context = std::make_unique<VulkanContextImpl>();
    if (!context->initialize(enableValidation)) {
        return nullptr;
    }
    return context;
}

} // namespace spectraforge

