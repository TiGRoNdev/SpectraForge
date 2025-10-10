/**
 * @file FrameManager.h
 * @brief Frame synchronization and management (P0.2 Refactoring)
 * 
 * SOLID COMPLIANCE:
 * - SRP: Единственная ответственность - управление кадрами и синхронизацией
 * - DIP: Dependency injection для device
 */

#pragma once

#include <memory>
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
class IFrameManager {
  public:
    virtual ~IFrameManager() = default;

    virtual bool initialize() = 0;
    virtual void shutdown() = 0;
    virtual bool beginFrame(vk::SwapchainKHR swapchain) = 0;
    virtual bool endFrame(vk::SwapchainKHR swapchain,
                          vk::Queue presentQueue,
                          vk::CommandBuffer commandBuffer) = 0;
    virtual uint32_t getCurrentImageIndex() const = 0;
    virtual size_t getCurrentFrame() const = 0;
    virtual vk::Semaphore getImageAvailableSemaphore() const = 0;
    virtual vk::Semaphore getRenderFinishedSemaphore() const = 0;
    virtual vk::Fence getInFlightFence() const = 0;
    virtual bool isDeviceLost() const = 0;
    virtual bool isInitialized() const = 0;
};

class IFrameManagerFactory {
  public:
    virtual ~IFrameManagerFactory() = default;
    virtual std::shared_ptr<IFrameManager> create(vk::Device device) = 0;
};

class FrameManager : public IFrameManager {
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
    bool initialize() override;
    
    /**
     * @brief Очистка всех ресурсов
     */
    void shutdown() override;
    
    /**
     * @brief Начало кадра - ожидание fence и acquire swapchain image
     * @param swapchain Swapchain для acquire
     * @return true если успешно, false если device lost
     */
    bool beginFrame(vk::SwapchainKHR swapchain) override;
    
    /**
     * @brief Завершение кадра - present image
     * @param swapchain Swapchain для present
     * @param presentQueue Present queue
     * @param commandBuffer Command buffer для submit
     * @return true если успешно, false если device lost
     */
    bool endFrame(vk::SwapchainKHR swapchain,
                 vk::Queue presentQueue,
                 vk::CommandBuffer commandBuffer) override;
    
    // === Getters ===
    
    /**
     * @brief Получить индекс текущего swapchain image
     */
    uint32_t getCurrentImageIndex() const override { return currentImageIndex_; }
    
    /**
     * @brief Получить индекс текущего frame in flight
     */
    size_t getCurrentFrame() const override { return currentFrame_; }
    
    /**
     * @brief Получить image available semaphore для текущего кадра
     */
    vk::Semaphore getImageAvailableSemaphore() const override {
        if (imageAvailableSemaphores_.empty()) return vk::Semaphore{};
        return imageAvailableSemaphores_[currentFrame_];
    }
    
    /**
     * @brief Получить render finished semaphore для текущего кадра
     */
    vk::Semaphore getRenderFinishedSemaphore() const override {
        if (renderFinishedSemaphores_.empty()) return vk::Semaphore{};
        return renderFinishedSemaphores_[currentFrame_];
    }
    
    /**
     * @brief Получить in-flight fence для текущего кадра
     */
    vk::Fence getInFlightFence() const override {
        if (inFlightFences_.empty()) return vk::Fence{};
        return inFlightFences_[currentFrame_];
    }
    
    /**
     * @brief Проверка device lost
     */
    bool isDeviceLost() const override { return deviceLost_; }
    
    /**
     * @brief Проверка инициализации
     */
    bool isInitialized() const override { return initialized_; }
    
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

class FrameManagerFactory : public IFrameManagerFactory {
  public:
    std::shared_ptr<IFrameManager> create(vk::Device device) override {
        return std::make_shared<FrameManager>(device);
    }
};

}  // namespace Core
}  // namespace Rendering
}  // namespace SpectraForge

