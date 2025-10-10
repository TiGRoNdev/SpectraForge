/**
 * @file RendererCore.h
 * @brief Core Vulkan initialization and device management (P0.2 Refactoring)
 * 
 * SOLID COMPLIANCE:
 * - SRP: Единственная ответственность - инициализация Vulkan core (instance, device, allocator)
 * - DIP: Будет использовать DI контейнер после P0.6
 */

#pragma once

#include <vulkan/vulkan.hpp>
#include <vk_mem_alloc.h>
#include <cstdint>

namespace SpectraForge {
namespace Rendering {
namespace Core {

class IRendererCore {
  public:
    virtual ~IRendererCore() = default;

    virtual bool initialize() = 0;
    virtual bool createLogicalDeviceWithSurface(vk::SurfaceKHR surface) = 0;
    virtual void shutdown() = 0;
    virtual vk::Instance getInstance() const = 0;
    virtual vk::PhysicalDevice getPhysicalDevice() const = 0;
    virtual vk::Device getDevice() const = 0;
    virtual vk::Queue getGraphicsQueue() const = 0;
    virtual vk::Queue getPresentQueue() const = 0;
    virtual VmaAllocator getAllocator() const = 0;
    virtual uint32_t getGraphicsQueueFamily() const = 0;
    virtual uint32_t getPresentQueueFamily() const = 0;
    virtual bool isInitialized() const = 0;
};

/**
 * @brief Управление Vulkan core компонентами (instance, device, allocator)
 */
class RendererCore : public IRendererCore {
public:
    RendererCore() = default;
    ~RendererCore();
    
    // Запрет копирования (Vulkan objects не копируются)
    RendererCore(const RendererCore&) = delete;
    RendererCore& operator=(const RendererCore&) = delete;
    
    /**
     * @brief Инициализация Vulkan instance
     * @return true если успешно
     */
    bool initialize() override;
    
    /**
     * @brief Создание logical device после создания surface
     * @param surface Vulkan surface для проверки present support
     * @return true если успешно
     */
    bool createLogicalDeviceWithSurface(vk::SurfaceKHR surface) override;
    
    /**
     * @brief Полная очистка всех Vulkan ресурсов
     */
    void shutdown() override;
    
    // === Getters ===
    
    /**
     * @brief Получить Vulkan instance
     */
    vk::Instance getInstance() const override { return instance_; }
    
    /**
     * @brief Получить физическое устройство (GPU)
     */
    vk::PhysicalDevice getPhysicalDevice() const override { return physicalDevice_; }
    
    /**
     * @brief Получить logical device
     */
    vk::Device getDevice() const override { return device_; }
    
    /**
     * @brief Получить graphics queue
     */
    vk::Queue getGraphicsQueue() const override { return graphicsQueue_; }
    
    /**
     * @brief Получить present queue
     */
    vk::Queue getPresentQueue() const override { return presentQueue_; }
    
    /**
     * @brief Получить VMA allocator
     */
    VmaAllocator getAllocator() const override { return allocator_; }
    
    /**
     * @brief Получить индекс graphics queue family
     */
    uint32_t getGraphicsQueueFamily() const override { return graphicsQueueFamily_; }
    
    /**
     * @brief Получить индекс present queue family
     */
    uint32_t getPresentQueueFamily() const override { return presentQueueFamily_; }
    
    /**
     * @brief Проверка инициализации
     */
    bool isInitialized() const override { return initialized_; }

private:
    /**
     * @brief Создание Vulkan instance с validation layers
     */
    bool createInstance();
    
    /**
     * @brief Выбор физического устройства (GPU)
     * Приоритет: Discrete GPU > Integrated GPU > Любое устройство
     */
    bool pickPhysicalDevice();
    
    /**
     * @brief Создание logical device с queue families
     * @param surface Surface для проверки present support
     */
    bool createLogicalDevice(vk::SurfaceKHR surface);
    
    /**
     * @brief Создание VMA allocator для управления памятью
     */
    bool createAllocator();

    // === Vulkan Core Objects ===
    vk::Instance instance_{};
    vk::PhysicalDevice physicalDevice_{};
    vk::Device device_{};
    vk::Queue graphicsQueue_{};
    vk::Queue presentQueue_{};
    VmaAllocator allocator_ = VK_NULL_HANDLE;
    
    uint32_t graphicsQueueFamily_ = 0;
    uint32_t presentQueueFamily_ = 0;
    
    bool initialized_ = false;
};

}  // namespace Core
}  // namespace Rendering
}  // namespace SpectraForge

