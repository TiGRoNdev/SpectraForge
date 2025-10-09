/**
 * @file SwapchainManager.h
 * @brief Swapchain and surface management (P0.2 Refactoring)
 * 
 * SOLID COMPLIANCE:
 * - SRP: Единственная ответственность - управление swapchain и surface
 * - DIP: Инжектируются зависимости (instance, device, physicalDevice)
 */

#pragma once

#include <vulkan/vulkan.hpp>
#include <vector>
#include <cstdint>

namespace SpectraForge {
namespace Rendering {
namespace Core {

/**
 * @brief Управление Vulkan swapchain и surface
 * 
 * Ответственность:
 * - Создание X11 surface
 * - Создание swapchain с оптимальными настройками
 * - Управление swapchain images и image views
 * - Пересоздание swapchain при resize
 * 
 * SOLID:
 * - SRP ✅: Только swapchain/surface management
 * - OCP ✅: Легко расширить для других платформ (Wayland, Win32)
 * - DIP ✅: Зависит от abstractions (vk::Instance, vk::Device)
 */
class SwapchainManager {
public:
    /**
     * @brief Конструктор с dependency injection
     * @param instance Vulkan instance
     * @param physicalDevice Физическое устройство
     * @param device Logical device
     */
    SwapchainManager(vk::Instance instance, 
                     vk::PhysicalDevice physicalDevice, 
                     vk::Device device);
    ~SwapchainManager();
    
    // Запрет копирования
    SwapchainManager(const SwapchainManager&) = delete;
    SwapchainManager& operator=(const SwapchainManager&) = delete;
    
    /**
     * @brief Создание X11 surface
     * @param x11Display Указатель на Display (X11)
     * @param x11Window Указатель на Window (X11)
     * @return true если успешно
     */
    bool createSurfaceX11(void* x11Display, void* x11Window);
    
    /**
     * @brief Создание swapchain с optimal settings
     * @param width Ширина окна
     * @param height Высота окна
     * @param graphicsQueueFamily Индекс graphics queue family
     * @param presentQueueFamily Индекс present queue family
     * @return true если успешно
     */
    bool createSwapchain(uint32_t width, uint32_t height,
                        uint32_t graphicsQueueFamily,
                        uint32_t presentQueueFamily);
    
    /**
     * @brief Уничтожение swapchain и views (для пересоздания)
     */
    void destroySwapchain();
    
    /**
     * @brief Полная очистка всех ресурсов
     */
    void shutdown();
    
    // === Getters ===
    
    /**
     * @brief Получить surface
     */
    vk::SurfaceKHR getSurface() const { return surface_; }
    
    /**
     * @brief Получить swapchain
     */
    vk::SwapchainKHR getSwapchain() const { return swapchain_; }
    
    /**
     * @brief Получить swapchain images
     */
    const std::vector<vk::Image>& getSwapchainImages() const { 
        return swapchainImages_; 
    }
    
    /**
     * @brief Получить swapchain image views
     */
    const std::vector<vk::ImageView>& getSwapchainImageViews() const { 
        return swapchainImageViews_; 
    }
    
    /**
     * @brief Получить формат swapchain
     */
    vk::Format getSwapchainFormat() const { return swapchainFormat_; }
    
    /**
     * @brief Получить размер swapchain
     */
    vk::Extent2D getSwapchainExtent() const { return swapchainExtent_; }
    
    /**
     * @brief Проверка инициализации
     */
    bool isInitialized() const { return swapchain_ != vk::SwapchainKHR{}; }

private:
    /**
     * @brief Создание swapchain и image views
     */
    bool createSwapchainAndViews(uint32_t width, uint32_t height,
                                 uint32_t graphicsQueueFamily,
                                 uint32_t presentQueueFamily);
    
    /**
     * @brief Выбор оптимального surface format
     */
    vk::SurfaceFormatKHR chooseSurfaceFormat(
        const std::vector<vk::SurfaceFormatKHR>& availableFormats);
    
    /**
     * @brief Выбор оптимального present mode
     */
    vk::PresentModeKHR choosePresentMode(
        const std::vector<vk::PresentModeKHR>& availableModes);
    
    /**
     * @brief Выбор оптимального extent
     */
    vk::Extent2D chooseExtent(uint32_t width, uint32_t height,
                             const vk::SurfaceCapabilitiesKHR& capabilities);

    // === Dependencies (injected) ===
    vk::Instance instance_;
    vk::PhysicalDevice physicalDevice_;
    vk::Device device_;
    
    // === Swapchain Resources ===
    vk::SurfaceKHR surface_{};
    vk::SwapchainKHR swapchain_{};
    std::vector<vk::Image> swapchainImages_{};
    std::vector<vk::ImageView> swapchainImageViews_{};
    vk::Format swapchainFormat_ = vk::Format::eB8G8R8A8Unorm;
    vk::Extent2D swapchainExtent_{1920, 1080};
};

}  // namespace Core
}  // namespace Rendering
}  // namespace SpectraForge

