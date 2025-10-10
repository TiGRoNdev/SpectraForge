/**
 * @file VulkanValidation.cpp
 * @brief Реализация управления Vulkan validation layers
 */

#include "SpectraForge/Vulkan/VulkanValidation.h"
#include <iostream>
#include <cstring>

namespace SpectraForge {
namespace Vulkan {

// Статическая переменная для callback функции
std::function<void(const std::string&)> VulkanValidation::debugCallbackFunction_ = nullptr;

bool VulkanValidation::checkValidationLayerSupport() {
    // Получаем список доступных layers
    uint32_t layerCount;
    vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

    std::vector<VkLayerProperties> availableLayers(layerCount);
    vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

    // Получаем список требуемых layers
    auto requiredLayers = getRequiredValidationLayers();

    // Проверяем что все требуемые layers доступны
    for (const char* layerName : requiredLayers) {
        bool layerFound = false;

        for (const auto& layerProperties : availableLayers) {
            if (strcmp(layerName, layerProperties.layerName) == 0) {
                layerFound = true;
                break;
            }
        }

        if (!layerFound) {
            std::cerr << "❌ Validation layer не найден: " << layerName << std::endl;
            return false;
        }
    }

    std::cout << "✅ Все validation layers поддерживаются" << std::endl;
    return true;
}

std::vector<const char*> VulkanValidation::getRequiredValidationLayers() {
    return {
        "VK_LAYER_KHRONOS_validation"
    };
}

vk::DebugUtilsMessengerEXT VulkanValidation::createDebugMessenger(vk::Instance instance) {
    vk::DebugUtilsMessengerCreateInfoEXT createInfo;
    populateDebugMessengerCreateInfo(createInfo);

    VkDebugUtilsMessengerEXT debugMessenger;
    VkResult result = createDebugUtilsMessengerEXT(
        static_cast<VkInstance>(instance),
        reinterpret_cast<const VkDebugUtilsMessengerCreateInfoEXT*>(&createInfo),
        nullptr,
        &debugMessenger
    );

    if (result != VK_SUCCESS) {
        std::cerr << "❌ Не удалось создать debug messenger: " << result << std::endl;
        return nullptr;
    }

    std::cout << "✅ Debug messenger создан успешно" << std::endl;
    return static_cast<vk::DebugUtilsMessengerEXT>(debugMessenger);
}

void VulkanValidation::setDebugCallback(std::function<void(const std::string&)> callback) {
    debugCallbackFunction_ = callback;
}

void VulkanValidation::populateDebugMessengerCreateInfo(vk::DebugUtilsMessengerCreateInfoEXT& createInfo) {
    // Используем C API для создания, так как C++ wrapper имеет несовместимую сигнатуру callback
    VkDebugUtilsMessengerCreateInfoEXT cCreateInfo = {};
    cCreateInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
    cCreateInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT |
                                  VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
                                  VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
    cCreateInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
                              VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
                              VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
    cCreateInfo.pfnUserCallback = debugCallback;
    cCreateInfo.pUserData = nullptr;
    
    // Копируем данные в C++ структуру
    createInfo = *reinterpret_cast<vk::DebugUtilsMessengerCreateInfoEXT*>(&cCreateInfo);
}

void VulkanValidation::destroyDebugMessenger(vk::Instance instance, vk::DebugUtilsMessengerEXT debugMessenger) {
    if (debugMessenger) {
        destroyDebugUtilsMessengerEXT(
            static_cast<VkInstance>(instance),
            static_cast<VkDebugUtilsMessengerEXT>(debugMessenger),
            nullptr
        );
        std::cout << "✅ Debug messenger уничтожен" << std::endl;
    }
}

VKAPI_ATTR VkBool32 VKAPI_CALL VulkanValidation::debugCallback(
    VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
    VkDebugUtilsMessageTypeFlagsEXT messageType,
    const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
    void* pUserData) {

    if (debugCallbackFunction_) {
        debugCallbackFunction_(pCallbackData->pMessage);
    } else {
        // Если callback не установлен, выводим в stderr
        std::cerr << "Validation layer: " << pCallbackData->pMessage << std::endl;
    }

    return VK_FALSE;
}

VkResult VulkanValidation::createDebugUtilsMessengerEXT(
    VkInstance instance,
    const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo,
    const VkAllocationCallbacks* pAllocator,
    VkDebugUtilsMessengerEXT* pDebugMessenger) {

    auto func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
    if (func != nullptr) {
        return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
    } else {
        return VK_ERROR_EXTENSION_NOT_PRESENT;
    }
}

void VulkanValidation::destroyDebugUtilsMessengerEXT(
    VkInstance instance,
    VkDebugUtilsMessengerEXT debugMessenger,
    const VkAllocationCallbacks* pAllocator) {

    auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");
    if (func != nullptr) {
        func(instance, debugMessenger, pAllocator);
    }
}

} // namespace Vulkan
} // namespace SpectraForge
