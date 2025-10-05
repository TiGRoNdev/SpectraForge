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
 * @brief Debug callback для получения сообщений от validation layers
 * @note Используем Vulkan-hpp типы для совместимости с vk::DebugUtilsMessengerCreateInfoEXT
 */
static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
    VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
    VkDebugUtilsMessageTypeFlagsEXT messageType,
    const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
    void* pUserData) {
    
    (void)pUserData;  // Неиспользуемый параметр
    
    // Формируем префикс в зависимости от типа сообщения
    std::string prefix = "[Vulkan";
    if (messageType & VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT) {
        prefix += " Validation";
    }
    if (messageType & VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT) {
        prefix += " Performance";
    }
    prefix += "]";
    
    // Выводим в зависимости от уровня важности
    if (messageSeverity >= VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT) {
        std::cerr << prefix << " ❌ ERROR: " << pCallbackData->pMessage << "\n";
    } else if (messageSeverity >= VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT) {
        std::cout << prefix << " ⚠️  WARNING: " << pCallbackData->pMessage << "\n";
    } else if (messageSeverity >= VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT) {
        std::cout << prefix << " ℹ️  INFO: " << pCallbackData->pMessage << "\n";
    } else {
        std::cout << prefix << " 🔍 VERBOSE: " << pCallbackData->pMessage << "\n";
    }
    
    return VK_FALSE;
}

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
            std::cout << "[VulkanContext] ℹ️ Уже очищено, пропускаем\n";
            return;
        }

        std::cout << "[VulkanContext] 🔄 Начинаем очистку ресурсов...\n";

        // КРИТИЧНО: Ожидаем завершения всех операций GPU перед уничтожением ресурсов
        if (device_) {
            std::cout << "[VulkanContext] ⏳ Синхронизация с GPU (ожидание до 5 сек)...\n";
            try {
                auto startTime = std::chrono::steady_clock::now();
                device_.waitIdle();
                auto endTime = std::chrono::steady_clock::now();
                auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime);
                std::cout << "[VulkanContext] ✅ GPU синхронизирован за " << duration.count() << " мс\n";
            } catch (const std::exception& e) {
                std::cerr << "[VulkanContext] ❌ Ошибка при синхронизации с GPU: " << e.what() << "\n";
                std::cerr << "[VulkanContext] ⚠️ Продолжаем очистку несмотря на ошибку синхронизации\n";
            }
        }

        // Cleanup VMA first (может содержать GPU ресурсы)
        std::cout << "[VulkanContext] 🧹 Очистка VMA memory manager...\n";
        try {
            core::VMAMemoryManager::getInstance().cleanup();
            std::cout << "[VulkanContext] ✅ VMA очищен\n";
        } catch (const std::exception& e) {
            std::cerr << "[VulkanContext] ❌ Ошибка при очистке VMA: " << e.what() << "\n";
        }

        // Destroy logical device
        if (device_) {
            std::cout << "[VulkanContext] 🗑️ Уничтожение логического устройства...\n";
            try {
                device_.destroy();
                device_ = nullptr;
                std::cout << "[VulkanContext] ✅ Логическое устройство уничтожено\n";
            } catch (const std::exception& e) {
                std::cerr << "[VulkanContext] ❌ Ошибка при уничтожении устройства: " << e.what() << "\n";
            }
        }

        // Destroy debug messenger перед instance (используем C API)
        if (debugMessenger_ && instance_) {
            std::cout << "[VulkanContext] 🗑️ Уничтожение debug messenger...\n";
            try {
                auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(
                    instance_, "vkDestroyDebugUtilsMessengerEXT");
                if (func != nullptr) {
                    func(instance_, debugMessenger_, nullptr);
                }
                debugMessenger_ = nullptr;
                std::cout << "[VulkanContext] ✅ Debug messenger уничтожен\n";
            } catch (const std::exception& e) {
                std::cerr << "[VulkanContext] ❌ Ошибка при уничтожении debug messenger: " << e.what() << "\n";
            }
        }
        
        // Destroy instance (последний шаг)
        if (instance_) {
            std::cout << "[VulkanContext] 🗑️ Уничтожение экземпляра Vulkan...\n";
            try {
                instance_.destroy();
                instance_ = nullptr;
                std::cout << "[VulkanContext] ✅ Экземпляр Vulkan уничтожен\n";
            } catch (const std::exception& e) {
                std::cerr << "[VulkanContext] ❌ Ошибка при уничтожении экземпляра: " << e.what() << "\n";
            }
        }

        initialized_ = false;
        std::cout << "[VulkanContext] ✅ Очистка завершена успешно\n";
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
        
        // Добавляем debug utils extension для validation layers
        if (enableValidation_) {
            extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
            std::cout << "[VulkanContext] 🔍 Добавлено расширение: " << VK_EXT_DEBUG_UTILS_EXTENSION_NAME << std::endl;
        }

        // Получаем требуемые расширения от GLFW (включает VK_KHR_surface и платформо-специфичные)
        // ПРИМЕЧАНИЕ: GLFW должен быть уже инициализирован Window::initializeSystem()
        std::cout << "[VulkanContext] 🔍 Запрос расширений Vulkan от GLFW..." << std::endl;
        
        // Дополнительная отладка: проверяем состояние GLFW
        std::cout << "[VulkanContext] 🔍 Проверка glfwVulkanSupported() перед запросом расширений..." << std::endl;
        int vulkanSupported = glfwVulkanSupported();
        std::cout << "[VulkanContext] glfwVulkanSupported() = " << (vulkanSupported ? "TRUE" : "FALSE") << std::endl;
        
        uint32_t glfwExtensionCount = 0;
        const char** glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);
        
        std::cout << "[VulkanContext] glfwGetRequiredInstanceExtensions() вернул: " 
                  << (glfwExtensions ? "VALID pointer" : "NULL")
                  << ", count = " << glfwExtensionCount << std::endl;
        
        if (glfwExtensions && glfwExtensionCount > 0) {
            std::cout << "[VulkanContext] ✅ GLFW предоставил " << glfwExtensionCount << " Vulkan расширений:" << std::endl;
            for (uint32_t i = 0; i < glfwExtensionCount; ++i) {
                extensions.push_back(glfwExtensions[i]);
                std::cout << "    - " << glfwExtensions[i] << std::endl;
            }
        } else {
            throw std::runtime_error(
                "glfwGetRequiredInstanceExtensions() вернул NULL!\n"
                "  Это критическая ошибка GLFW-Vulkan интеграции.\n"
                "  Убедитесь, что:\n"
                "  1) GLFW инициализирован через glfwInit()\n"
                "  2) glfwVulkanSupported() возвращает TRUE\n"
                "  3) Окно (если создано) использует GLFW_CLIENT_API = GLFW_NO_API\n"
                "  4) Установлены Vulkan драйверы и loader\n\n"
                "  Текущий статус:\n"
                "    glfwVulkanSupported() = " + 
                std::string(glfwVulkanSupported() ? "TRUE" : "FALSE")
            );
        }
        
        createInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
        createInfo.ppEnabledExtensionNames = extensions.data();
        
        // Validation layers - безопасная проверка и включение только доступных слоев
        std::vector<const char*> validationLayers;
        if (enableValidation_) {
            // Получаем все доступные слои валидации
            auto availableLayers = vk::enumerateInstanceLayerProperties();
            std::cout << "[VulkanContext] 🔍 Доступные validation layers (" << availableLayers.size() << "):" << std::endl;
            for (const auto& layer : availableLayers) {
                std::cout << "  ✓ " << layer.layerName << std::endl;
            }

            // Список желаемых слоев (будут включены только если доступны)
            const std::vector<const char*> requestedLayers = {
                "VK_LAYER_KHRONOS_validation",    // Основной слой валидации
            };

            std::cout << "[VulkanContext] 🔍 Проверка запрошенных слоёв..." << std::endl;
            for (const char* requestedLayer : requestedLayers) {
                bool found = false;
                for (const auto& availableLayer : availableLayers) {
                    if (strcmp(requestedLayer, availableLayer.layerName) == 0) {
                        validationLayers.push_back(requestedLayer);
                        found = true;
                        std::cout << "  ✅ " << requestedLayer << " - ДОСТУПЕН" << std::endl;
                        break;
                    }
                }
                if (!found) {
                    std::cout << "  ⚠️  " << requestedLayer << " - НЕДОСТУПЕН (пропускаем)" << std::endl;
                }
            }

            if (validationLayers.empty()) {
                std::cout << "[VulkanContext] ⚠️  Validation layers запрошены, но ни один не доступен" << std::endl;
                std::cout << "[VulkanContext] ℹ️  Продолжаем без validation layers" << std::endl;
                enableValidation_ = false;
            } else {
                std::cout << "[VulkanContext] ✅ Включаем " << validationLayers.size() << " validation layer(s)" << std::endl;
                createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
                createInfo.ppEnabledLayerNames = validationLayers.data();
            }
        } else {
            std::cout << "[VulkanContext] ℹ️  Validation layers отключены пользователем" << std::endl;
        }
        
        instance_ = vk::createInstance(createInfo);

        std::cout << "Vulkan instance created (API 1.3)\n";
        // Initialize dispatcher with instance
        VULKAN_HPP_DEFAULT_DISPATCHER.init(instance_);
        
        // Создаём debug messenger если validation layers включены (используем C API напрямую)
        if (enableValidation_ && !validationLayers.empty()) {
            VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo{};
            debugCreateInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
            debugCreateInfo.pNext = nullptr;
            debugCreateInfo.flags = 0;
            debugCreateInfo.messageSeverity = 
                VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT |
                VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT |
                VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
                VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
            debugCreateInfo.messageType = 
                VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
                VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
                VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
            debugCreateInfo.pfnUserCallback = debugCallback;
            debugCreateInfo.pUserData = nullptr;
            
            // Используем C API для создания messenger, затем оборачиваем в vk::DebugUtilsMessengerEXT
            VkDebugUtilsMessengerEXT rawMessenger;
            auto func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(
                instance_, "vkCreateDebugUtilsMessengerEXT");
            if (func != nullptr && func(instance_, &debugCreateInfo, nullptr, &rawMessenger) == VK_SUCCESS) {
                debugMessenger_ = vk::DebugUtilsMessengerEXT(rawMessenger);
                std::cout << "[VulkanContext] ✅ Debug messenger создан - validation layers активны!\n";
            } else {
                std::cerr << "[VulkanContext] ⚠️  Не удалось создать debug messenger\n";
            }
        }
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
        // Включаем robust буферный доступ (если поддерживается) для предотвращения OOB → DeviceLost
        deviceFeatures.robustBufferAccess = supported.robustBufferAccess;
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
        // Включаем робастность доступа к изображениям через core 1.3
        features13.robustImageAccess = VK_TRUE;
        
        // Vulkan 1.2 features
        vk::PhysicalDeviceVulkan12Features features12{};
        features12.bufferDeviceAddress = VK_TRUE;
        features12.descriptorIndexing = VK_TRUE;
        // Enable fp16 arithmetic if supported to satisfy SPIR-V Capability Float16
        features12.shaderFloat16 = supported12.shaderFloat16;
        features12.shaderInt8 = supported12.shaderInt8;
        features13.pNext = &features12;

        // Опционально: VK_EXT_robustness2 (если доступно) для усиленной защиты доступа к буферам/изображениям
        bool hasRobust2 = false;
        {
            auto extProps = physicalDevice_.enumerateDeviceExtensionProperties();
            for (const auto& ep : extProps) {
                if (std::string(ep.extensionName) == std::string(VK_EXT_ROBUSTNESS_2_EXTENSION_NAME)) {
                    hasRobust2 = true;
                    break;
                }
            }
        }
        vk::PhysicalDeviceRobustness2FeaturesEXT robust2{};
        if (hasRobust2) {
            robust2.robustBufferAccess2 = VK_TRUE;
            robust2.robustImageAccess2 = VK_TRUE;
            // nullDescriptor оставим по умолчанию (false) чтобы не менять семантику биндингов
            features12.pNext = &robust2;
        }

        // Не добавляем VkPhysicalDeviceImageRobustnessFeatures в pNext при наличии Vulkan13 (см. VUID-06532)
        
        // Device extensions
        std::vector<const char*> deviceExtensions = {
            VK_KHR_SWAPCHAIN_EXTENSION_NAME
        };
        if (hasRobust2) {
            deviceExtensions.push_back(VK_EXT_ROBUSTNESS_2_EXTENSION_NAME);
        }
        // VK_EXT_image_robustness не требуется при core 1.3 robustImageAccess
        
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
    vk::DebugUtilsMessengerEXT debugMessenger_;
    
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

