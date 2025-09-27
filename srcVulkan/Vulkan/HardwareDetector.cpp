/**
 * @file HardwareDetector.cpp
 * @brief Реализация детектора аппаратного обеспечения
 * 
 * Определяет возможности GPU и выбирает оптимальные пути рендеринга
 * согласно UML архитектуре из FEATURE_PLAN.
 */

#include "Engine3D/Vulkan/HardwareDetector.h"
#include <iostream>
#include <algorithm>

using namespace Engine3D::Vulkan;

namespace Engine3D::Vulkan {

HardwareDetector::HardwareDetector() {
    // Инициализация в init()
}

HardwareDetector::~HardwareDetector() {
    // Автоматическая очистка
}

bool HardwareDetector::init(vk::PhysicalDevice device) {
    try {
        this->physicalDevice = device;
        
        // Получаем свойства устройства
        props = physicalDevice.getProperties();
        features = physicalDevice.getFeatures();
        memoryProps = physicalDevice.getMemoryProperties();
        
        // Загружаем список доступных расширений
        loadAvailableExtensions();
        
        initialized = true;
        
        std::cout << "[HardwareDetector] Инициализация завершена для устройства: " 
                  << getDeviceName() << std::endl;
        
        return true;
        
    } catch (const std::exception& e) {
        std::cerr << "[HardwareDetector] Ошибка инициализации: " << e.what() << std::endl;
        return false;
    }
}

VendorType HardwareDetector::detectVendor() {
    if (!initialized) {
        return VendorType::OTHER;
    }
    
    return getVendorFromID(props.vendorID);
}

bool HardwareDetector::supportsRayTracing() {
    if (!initialized) {
        return false;
    }
    
    return checkRayTracingSupport();
}

UpscalerType HardwareDetector::selectUpscalerPath() {
    if (!initialized) {
        return UpscalerType::NONE;
    }
    
    VendorType vendor = detectVendor();
    
    switch (vendor) {
        case VendorType::NVIDIA:
            // Проверяем поддержку DLSS (требует RTX карты)
            if (supportsRayTracing()) {
                std::cout << "[HardwareDetector] Выбран путь DLSS для NVIDIA RTX" << std::endl;
                return UpscalerType::DLSS;
            }
            // Fallback на FSR для старых NVIDIA карт
            std::cout << "[HardwareDetector] Fallback на FSR для старой NVIDIA карты" << std::endl;
            return UpscalerType::FSR;
            
        case VendorType::AMD:
            std::cout << "[HardwareDetector] Выбран путь FSR для AMD" << std::endl;
            return UpscalerType::FSR;
            
        case VendorType::INTEL:
            std::cout << "[HardwareDetector] Выбран путь FSR для Intel" << std::endl;
            return UpscalerType::FSR;
            
        default:
            std::cout << "[HardwareDetector] Неизвестный вендор, upscaling отключен" << std::endl;
            return UpscalerType::NONE;
    }
}

bool HardwareDetector::supportsCUDA() {
    if (!initialized) {
        return false;
    }
    
    // CUDA поддерживается только на NVIDIA картах
    VendorType vendor = detectVendor();
    bool cudaSupported = (vendor == VendorType::NVIDIA);
    
    if (cudaSupported) {
        // Дополнительно можно проверить наличие CUDA runtime
        // Пока просто возвращаем true для NVIDIA
        std::cout << "[HardwareDetector] CUDA поддерживается (NVIDIA GPU)" << std::endl;
    } else {
        std::cout << "[HardwareDetector] CUDA не поддерживается (не NVIDIA GPU)" << std::endl;
    }
    
    return cudaSupported;
}

bool HardwareDetector::supportsOptiX() {
    if (!initialized) {
        return false;
    }
    
    // OptiX поддерживается только на NVIDIA RTX картах
    VendorType vendor = detectVendor();
    bool rtSupported = supportsRayTracing();
    
    bool optixSupported = (vendor == VendorType::NVIDIA) && rtSupported;
    
    if (optixSupported) {
        std::cout << "[HardwareDetector] OptiX поддерживается (NVIDIA RTX)" << std::endl;
    } else {
        std::cout << "[HardwareDetector] OptiX не поддерживается" << std::endl;
    }
    
    return optixSupported;
}

std::string HardwareDetector::getDeviceName() const {
    if (!initialized) {
        return "Неинициализировано";
    }
    
    return std::string(props.deviceName.data());
}

size_t HardwareDetector::getVRAMSize() const {
    if (!initialized) {
        return 0;
    }
    
    try {
        size_t maxVRAM = 0;
        
        // Ищем самую большую heap с флагом DEVICE_LOCAL
        for (uint32_t i = 0; i < memoryProps.memoryHeapCount; ++i) {
            const auto& heap = memoryProps.memoryHeaps[i];
            if (heap.flags & vk::MemoryHeapFlagBits::eDeviceLocal) {
                maxVRAM = std::max(maxVRAM, static_cast<size_t>(heap.size));
            }
        }
        
        return maxVRAM;
        
    } catch (const std::exception& e) {
        std::cerr << "[HardwareDetector] Ошибка получения размера VRAM: " << e.what() << std::endl;
        return 0;
    }
}

bool HardwareDetector::supportsExtension(const std::string& extensionName) const {
    if (!initialized) {
        return false;
    }
    
    return std::any_of(availableExtensions.begin(), availableExtensions.end(),
        [&extensionName](const vk::ExtensionProperties& ext) {
            return std::string(ext.extensionName.data()) == extensionName;
        });
}

// Приватные методы

VendorType HardwareDetector::getVendorFromID(uint32_t vendorID) {
    switch (vendorID) {
        case 0x10DE: // NVIDIA
            return VendorType::NVIDIA;
        case 0x1002: // AMD
            return VendorType::AMD;
        case 0x8086: // Intel
            return VendorType::INTEL;
        default:
            return VendorType::OTHER;
    }
}

bool HardwareDetector::checkRayTracingSupport() {
    // Проверяем наличие необходимых расширений для ray tracing
    bool hasRayTracingPipeline = supportsExtension("VK_KHR_ray_tracing_pipeline");
    bool hasAccelerationStructure = supportsExtension("VK_KHR_acceleration_structure");
    bool hasRayQuery = supportsExtension("VK_KHR_ray_query");
    
    // Для полной поддержки RT нужны все три расширения
    bool rtSupported = hasRayTracingPipeline && hasAccelerationStructure && hasRayQuery;
    
    if (rtSupported) {
        std::cout << "[HardwareDetector] Ray Tracing полностью поддерживается" << std::endl;
    } else {
        std::cout << "[HardwareDetector] Ray Tracing не поддерживается:" << std::endl;
        std::cout << "  - VK_KHR_ray_tracing_pipeline: " << (hasRayTracingPipeline ? "Да" : "Нет") << std::endl;
        std::cout << "  - VK_KHR_acceleration_structure: " << (hasAccelerationStructure ? "Да" : "Нет") << std::endl;
        std::cout << "  - VK_KHR_ray_query: " << (hasRayQuery ? "Да" : "Нет") << std::endl;
    }
    
    return rtSupported;
}

void HardwareDetector::loadAvailableExtensions() {
    try {
        availableExtensions = physicalDevice.enumerateDeviceExtensionProperties();
        
        std::cout << "[HardwareDetector] Найдено " << availableExtensions.size() 
                  << " доступных расширений" << std::endl;
        
        // Выводим важные расширения для отладки
        std::vector<std::string> importantExtensions = {
            "VK_KHR_ray_tracing_pipeline",
            "VK_KHR_acceleration_structure", 
            "VK_KHR_ray_query",
            "VK_KHR_external_memory",
            "VK_KHR_external_memory_win32",  // Добавлено для CUDA interop
            "VK_KHR_external_semaphore",
            "VK_KHR_external_semaphore_win32", // Добавлено для CUDA interop
            "VK_EXT_memory_budget"
        };
        
        for (const auto& extName : importantExtensions) {
            bool supported = supportsExtension(extName);
            std::cout << "[HardwareDetector] " << extName << ": " 
                      << (supported ? "Поддерживается" : "Не поддерживается") << std::endl;
        }
        
    } catch (const std::exception& e) {
        std::cerr << "[HardwareDetector] Ошибка загрузки расширений: " << e.what() << std::endl;
        availableExtensions.clear();
    }
}

} // namespace Engine3D::Vulkan
