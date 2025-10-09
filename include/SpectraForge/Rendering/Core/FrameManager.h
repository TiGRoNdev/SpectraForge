/**
 * @file FrameManager.h
 * @brief Frame synchronization and management (P0.2 Refactoring)
 * 
 * SOLID COMPLIANCE:
 * - SRP: Единственная ответственность - управление кадрами и синхронизацией
 * - DIP: Dependency injection для device
 */

#pragma once

#include <vulkan/vulkan.hpp>
#include <vector>
#include <cstdint>

namespace SpectraForge {
namespace Rendering {
namespace Core {

/**
 * @brief Управление frame synchronization (frames in flight)
 * 
 * Ответственность:
 * - Создание sync objects (semaphores, fences)
 * - Управление frames in flight (MAX_FRAMES_IN_FLIGHT)
 * - beginFrame() - acquire next swapchain image
 * - endFrame() - present rendered image
 * - Обработка VK_ERROR_DEVICE_LOST
 * 
 * SOLID:
 * - SRP ✅: Только frame synchronization
 * - OCP ✅: Легко расширить для triple buffering
 * - DIP ✅: Зависит от abstractions
 */
class FrameManager {
public:
    /**
     * @brief Конструктор с dependency injection
     * @param device Logical device
     */
    explicit FrameManager(vk::Device device);
    ~FrameManager();
    
    // Запрет копирования
    FrameManager(const FrameManager&) = delete;
    FrameManager& operator=(const FrameManager&) = delete;
    
    /**
     * @brief Инициализация sync objects
     * @return true если успешно
     */
    bool initialize();
    
    /**
     * @brief Очистка всех ресурсов
     */
    void shutdown();
    
    /**
     * @brief Начало кадра - ожидание fence и acquire swapchain image
     * @param swapchain Swapchain для acquire
     * @return true если успешно, false если device lost
     */
    bool beginFrame(vk::SwapchainKHR swapchain);
    
    /**
     * @brief Завершение кадра - present image
     * @param swapchain Swapchain для present
     * @param presentQueue Present queue
     * @param commandBuffer Command buffer для submit
     * @return true если успешно, false если device lost
     */
    bool endFrame(vk::SwapchainKHR swapchain, 
                 vk::Queue presentQueue,
                 vk::CommandBuffer commandBuffer);
    
    // === Getters ===
    
    /**
     * @brief Получить индекс текущего swapchain image
     */
    uint32_t getCurrentImageIndex() const { return currentImageIndex_; }
    
    /**
     * @brief Получить индекс текущего frame in flight
     */
    size_t getCurrentFrame() const { return currentFrame_; }
    
    /**
     * @brief Получить image available semaphore для текущего кадра
     */
    vk::Semaphore getImageAvailableSemaphore() const {
        if (imageAvailableSemaphores_.empty()) return vk::Semaphore{};
        return imageAvailableSemaphores_[currentFrame_];
    }
    
    /**
     * @brief Получить render finished semaphore для текущего кадра
     */
    vk::Semaphore getRenderFinishedSemaphore() const {
        if (renderFinishedSemaphores_.empty()) return vk::Semaphore{};
        return renderFinishedSemaphores_[currentFrame_];
    }
    
    /**
     * @brief Получить in-flight fence для текущего кадра
     */
    vk::Fence getInFlightFence() const {
        if (inFlightFences_.empty()) return vk::Fence{};
        return inFlightFences_[currentFrame_];
    }
    
    /**
     * @brief Проверка device lost
     */
    bool isDeviceLost() const { return deviceLost_; }
    
    /**
     * @brief Проверка инициализации
     */
    bool isInitialized() const { return initialized_; }
    
    /**
     * @brief Максимальное количество frames in flight
     */
    static constexpr int MAX_FRAMES_IN_FLIGHT = 2;

private:
    /**
     * @brief Создание synchronization objects (semaphores, fences)
     */
    bool createSyncObjects();

    // === Dependencies (injected) ===
    vk::Device device_;
    
    // === Synchronization Objects ===
    std::vector<vk::Semaphore> imageAvailableSemaphores_{};
    std::vector<vk::Semaphore> renderFinishedSemaphores_{};
    std::vector<vk::Fence> inFlightFences_{};
    
    // === State ===
    size_t currentFrame_ = 0;
    uint32_t currentImageIndex_ = 0;
    bool deviceLost_ = false;
    bool initialized_ = false;
};

}  // namespace Core
}  // namespace Rendering
}  // namespace SpectraForge

