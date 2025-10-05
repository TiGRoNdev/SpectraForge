/**
 * @file VulkanValidation.h
 * @brief Управление Vulkan validation layers и debug messenger
 */

#pragma once

#include <vulkan/vulkan.hpp>
#include <vector>
#include <string>
#include <functional>

namespace SpectraForge {
namespace Vulkan {

/**
 * @brief Класс для управления Vulkan validation layers и debug messenger
 */
class VulkanValidation {
public:
    VulkanValidation() = default;
    ~VulkanValidation() = default;

    /**
     * @brief Проверка поддержки validation layers
     * @return true если все запрошенные layers поддерживаются
     */
    static bool checkValidationLayerSupport();

    /**
     * @brief Получение списка требуемых validation layers
     * @return Вектор имен validation layers
     */
    static std::vector<const char*> getRequiredValidationLayers();

    /**
     * @brief Создание debug messenger для instance
     * @param instance Vulkan instance
     * @return Созданный debug messenger
     */
    static vk::DebugUtilsMessengerEXT createDebugMessenger(vk::Instance instance);

    /**
     * @brief Установка callback функции для debug сообщений
     * @param callback Функция обратного вызова
     */
    static void setDebugCallback(std::function<void(const std::string&)> callback);

    /**
     * @brief Заполнение DebugUtilsMessengerCreateInfoEXT структуры
     * @param createInfo Структура для заполнения
     */
    static void populateDebugMessengerCreateInfo(vk::DebugUtilsMessengerCreateInfoEXT& createInfo);

    /**
     * @brief Уничтожение debug messenger
     * @param instance Vulkan instance
     * @param debugMessenger Debug messenger для уничтожения
     */
    static void destroyDebugMessenger(vk::Instance instance, vk::DebugUtilsMessengerEXT debugMessenger);

private:
    /**
     * @brief Debug callback функция (статическая)
     */
    static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
        VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
        VkDebugUtilsMessageTypeFlagsEXT messageType,
        const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
        void* pUserData);

    /**
     * @brief Создание debug utils messenger (внутренняя функция)
     */
    static VkResult createDebugUtilsMessengerEXT(
        VkInstance instance,
        const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo,
        const VkAllocationCallbacks* pAllocator,
        VkDebugUtilsMessengerEXT* pDebugMessenger);

    /**
     * @brief Уничтожение debug utils messenger (внутренняя функция)
     */
    static void destroyDebugUtilsMessengerEXT(
        VkInstance instance,
        VkDebugUtilsMessengerEXT debugMessenger,
        const VkAllocationCallbacks* pAllocator);

private:
    static std::function<void(const std::string&)> debugCallbackFunction_;
};

} // namespace Vulkan
} // namespace SpectraForge
