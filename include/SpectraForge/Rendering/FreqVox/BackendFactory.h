/**
 * @file BackendFactory.h
 * @brief Factory for creating FFT/DCT backends based on available hardware
 */

#pragma once

#include "SpectraForge/Rendering/FreqVox/FrequencyShading.h"
#include <memory>
#include <string>

// Forward declarations для избежания циклических зависимостей
namespace SpectraForge::Vulkan {
    class HardwareDetector;
    class VulkanEngine;
}

// Forward declarations для Vulkan
namespace vk {
    class Instance;
    class PhysicalDevice;
    class Device;
}

namespace SpectraForge::Rendering::FreqVox {

/**
 * @brief Фабрика для создания оптимального FFT/DCT бэкенда
 * 
 * Автоматически выбирает лучший доступный бэкенд:
 * - cuFFT (если NVIDIA GPU + CUDA доступна)
 * - VkFFT (кроссплатформенный fallback для любых Vulkan GPU)
 * - SimpleDCT (минимальная заглушка для тестов или fallback)
 */
class BackendFactory {
public:
    enum class BackendType {
        Auto,      ///< Автоматический выбор лучшего (рекомендуется)
        CuFFT,     ///< NVIDIA cuFFT (требует CUDA)
        VkFFT,     ///< VkFFT через Vulkan
        CpuDct2,   ///< CPU-based DCT-II (Math.md compliant)
        Simple     ///< Простая заглушка (deprecated, use CpuDct2)
    };

    /**
     * @brief Создать бэкенд указанного типа
     * @param type Тип бэкенда (Auto для автовыбора)
     * @return Умный указатель на бэкенд или nullptr при ошибке
     */
    static std::unique_ptr<IFrequencyBackend> create(BackendType type = BackendType::Auto);

    /**
     * @brief Создать бэкенд с учетом аппаратных возможностей
     * @param type Тип бэкенда (Auto для автовыбора на основе hardware)
     * @param hwDetector Указатель на HardwareDetector для runtime проверок
     * @param vulkanEngine Указатель на VulkanEngine (опционально, для VkFFT)
     * @return Умный указатель на бэкенд или nullptr при ошибке
     * 
     * Этот метод использует runtime детекцию железа для выбора оптимального бэкенда:
     * - NVIDIA GPU + CUDA → cuFFT
     * - Любой Vulkan GPU → VkFFT (требует vulkanEngine для device)
     * - Fallback → CpuDct2
     * 
     * @note Для VkFFT бэкенда рекомендуется передавать vulkanEngine
     */
    static std::unique_ptr<IFrequencyBackend> createWithHardwareDetection(
        BackendType type,
        SpectraForge::Vulkan::HardwareDetector* hwDetector,
        SpectraForge::Vulkan::VulkanEngine* vulkanEngine = nullptr);

    /**
     * @brief Создать бэкенд с отдельными Vulkan объектами (удобная перегрузка)
     * @param type Тип бэкенда (Auto для автовыбора на основе hardware)
     * @param hwDetector Указатель на HardwareDetector для runtime проверок
     * @param instance Vulkan instance
     * @param physicalDevice Vulkan physical device
     * @param device Vulkan logical device
     * @return Умный указатель на бэкенд или nullptr при ошибке
     * 
     * Эта перегрузка удобна, когда у вас есть отдельные Vulkan объекты,
     * но нет VulkanEngine. Автоматически передаёт их в VkFFT backend.
     */
    static std::unique_ptr<IFrequencyBackend> createWithHardwareDetection(
        BackendType type,
        SpectraForge::Vulkan::HardwareDetector* hwDetector,
        vk::Instance instance,
        vk::PhysicalDevice physicalDevice,
        vk::Device device);

    /**
     * @brief Проверить доступность типа бэкенда (compile-time)
     * @param type Тип бэкенда
     * @return true если бэкенд доступен на текущей платформе
     */
    static bool isAvailable(BackendType type);

    /**
     * @brief Проверить доступность типа бэкенда (runtime + hardware)
     * @param type Тип бэкенда
     * @param hwDetector Указатель на HardwareDetector для runtime проверок
     * @return true если бэкенд доступен на текущем железе
     */
    static bool isAvailableOnHardware(BackendType type, 
                                       SpectraForge::Vulkan::HardwareDetector* hwDetector);

    /**
     * @brief Получить имя типа бэкенда
     * @param type Тип бэкенда
     * @return Строковое представление
     */
    static std::string getTypeName(BackendType type);

    /**
     * @brief Автоматически выбрать лучший тип бэкенда для данного железа
     * @param hwDetector Указатель на HardwareDetector
     * @return Рекомендуемый тип бэкенда
     */
    static BackendType selectBestBackend(SpectraForge::Vulkan::HardwareDetector* hwDetector);

private:
    BackendFactory() = delete;
};

} // namespace SpectraForge::Rendering::FreqVox

