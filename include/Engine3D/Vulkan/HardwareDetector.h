#pragma once

#include <string>
#include <vulkan/vulkan.hpp>

namespace Engine3D::Vulkan {

/**
 * @brief Типы вендоров GPU
 */
enum class VendorType { NVIDIA, AMD, INTEL, OTHER };

/**
 * @brief Типы систем upscaling
 */
enum class UpscalerType {
    DLSS,  // NVIDIA DLSS
    FSR,   // AMD FSR
    NONE   // Без upscaling
};

/**
 * @brief Детектор аппаратного обеспечения
 *
 * Определяет возможности GPU и выбирает оптимальные пути рендеринга
 * согласно UML архитектуре.
 */
class HardwareDetector {
  public:
    /**
     * @brief Конструктор
     */
    HardwareDetector();

    /**
     * @brief Деструктор
     */
    ~HardwareDetector();

    /**
     * @brief Инициализация детектора
     * @param physicalDevice Vulkan физическое устройство
     * @return true если инициализация успешна
     */
    bool init(vk::PhysicalDevice physicalDevice);

    /**
     * @brief Определение вендора GPU
     * @return Тип вендора
     */
    VendorType detectVendor();

    /**
     * @brief Проверка поддержки ray tracing
     * @return true если ray tracing поддерживается
     */
    bool supportsRayTracing();

    /**
     * @brief Выбор оптимального пути upscaling
     * @return Тип upscaler'а
     */
    UpscalerType selectUpscalerPath();

    /**
     * @brief Проверка поддержки CUDA
     * @return true если CUDA поддерживается
     */
    bool supportsCUDA();

    /**
     * @brief Проверка поддержки OptiX
     * @return true если OptiX поддерживается
     */
    bool supportsOptiX();

    /**
     * @brief Получение свойств устройства
     * @return Свойства физического устройства
     */
    const vk::PhysicalDeviceProperties& getDeviceProperties() const { return props; }

    /**
     * @brief Получение возможностей устройства
     * @return Возможности физического устройства
     */
    const vk::PhysicalDeviceFeatures& getDeviceFeatures() const { return features; }

    /**
     * @brief Получение имени устройства
     * @return Имя GPU
     */
    std::string getDeviceName() const;

    /**
     * @brief Получение объема видеопамяти
     * @return Размер VRAM в байтах
     */
    size_t getVRAMSize() const;

    /**
     * @brief Проверка поддержки расширения
     * @param extensionName Имя расширения
     * @return true если расширение поддерживается
     */
    bool supportsExtension(const std::string& extensionName) const;

  private:
    vk::PhysicalDevice physicalDevice;
    vk::PhysicalDeviceProperties props;
    vk::PhysicalDeviceFeatures features;
    vk::PhysicalDeviceMemoryProperties memoryProps;

    std::vector<vk::ExtensionProperties> availableExtensions;

    bool initialized = false;

    /**
     * @brief Определение вендора по ID
     * @param vendorID ID вендора
     * @return Тип вендора
     */
    VendorType getVendorFromID(uint32_t vendorID);

    /**
     * @brief Проверка поддержки ray tracing расширений
     * @return true если RT расширения поддерживаются
     */
    bool checkRayTracingSupport();

    /**
     * @brief Загрузка списка доступных расширений
     */
    void loadAvailableExtensions();
};

}  // namespace Engine3D::Vulkan
