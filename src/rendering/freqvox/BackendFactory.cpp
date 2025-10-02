/**
 * @file BackendFactory.cpp
 * @brief Реализация фабрики FFT/DCT бэкендов с железо-зависимой логикой
 */

#include "SpectraForge/Rendering/FreqVox/BackendFactory.h"
#include "SpectraForge/Rendering/FreqVox/Backends/SimpleDctBackend.h"
#include "SpectraForge/Rendering/FreqVox/Backends/CpuDct2Backend.h"
#include "SpectraForge/Rendering/FreqVox/Backends/CuFFTBackend.h"
#include "SpectraForge/Rendering/FreqVox/Backends/VkFFTBackend.h"
#include "SpectraForge/Vulkan/HardwareDetector.h"
#include "SpectraForge/Vulkan/VulkanEngine.h"
#include "SpectraForge/Core/SafeConsole.h"

namespace SpectraForge::Rendering::FreqVox {

std::unique_ptr<IFrequencyBackend> BackendFactory::create(BackendType type) {
    if (type == BackendType::Auto) {
        // Автовыбор БЕЗ HardwareDetector: только compile-time проверки
        // Приоритет: cuFFT (если CUDA скомпилирована) > VkFFT > Simple
#ifdef HYPERENGINE_CUDA_AVAILABLE
        SAFE_PRINT_LINE("[BackendFactory] Авто-выбор (compile-time): cuFFT (CUDA доступна при компиляции)");
        SAFE_PRINT_LINE("[BackendFactory] ПРЕДУПРЕЖДЕНИЕ: Runtime проверка CUDA не выполнена!");
        SAFE_PRINT_LINE("[BackendFactory] Используйте createWithHardwareDetection() для умного выбора.");
        return std::make_unique<Backends::CuFFTBackend>();
#else
        SAFE_PRINT_LINE("[BackendFactory] Авто-выбор (compile-time): VkFFT (CUDA недоступна)");
        return std::make_unique<Backends::VkFFTBackend>();
#endif
    }

    switch (type) {
        case BackendType::CuFFT:
#ifdef HYPERENGINE_CUDA_AVAILABLE
            SAFE_PRINT_LINE("[BackendFactory] Создан cuFFT backend");
            return std::make_unique<Backends::CuFFTBackend>();
#else
            SAFE_ERROR("[BackendFactory] cuFFT запрошен, но CUDA недоступна при компиляции");
            return nullptr;
#endif

        case BackendType::VkFFT:
            SAFE_PRINT_LINE("[BackendFactory] Создан VkFFT backend");
            return std::make_unique<Backends::VkFFTBackend>();

        case BackendType::CpuDct2:
            SAFE_PRINT_LINE("[BackendFactory] Создан CpuDct2 backend (Math.md DCT-II)");
            return std::make_unique<Backends::CpuDct2Backend>();

        case BackendType::Simple:
            SAFE_PRINT_LINE("[BackendFactory] Создан Simple backend (deprecated)");
            return std::make_unique<Backends::SimpleDctBackend>();

        case BackendType::Auto:
            // Fallthrough to default
            [[fallthrough]];
        default:
            SAFE_ERROR("[BackendFactory] Неизвестный тип бэкенда");
            return nullptr;
    }
}

std::unique_ptr<IFrequencyBackend> BackendFactory::createWithHardwareDetection(
    BackendType type,
    SpectraForge::Vulkan::HardwareDetector* hwDetector,
    SpectraForge::Vulkan::VulkanEngine* vulkanEngine) {
    
    // Если HardwareDetector не передан, fallback на обычный create
    if (!hwDetector) {
        SAFE_WARNING("[BackendFactory] HardwareDetector не передан, используется compile-time выбор");
        return create(type);
    }

    // Для Auto режима: умный выбор на основе железа
    if (type == BackendType::Auto) {
        BackendType selectedType = selectBestBackend(hwDetector);
        SAFE_PRINT_LINE("[BackendFactory] Авто-выбор (hardware-aware): " + getTypeName(selectedType));
        
        // Рекурсивно создаем выбранный бэкенд
        return createWithHardwareDetection(selectedType, hwDetector, vulkanEngine);
    }

    // Для явно указанного типа: проверяем доступность на железе
    if (!isAvailableOnHardware(type, hwDetector)) {
        SAFE_ERROR("[BackendFactory] Бэкенд " + getTypeName(type) + 
                   " недоступен на текущем железе (" + hwDetector->getDeviceName() + ")");
        
        // Fallback на CpuDct2
        SAFE_WARNING("[BackendFactory] Fallback на CpuDct2 (CPU)");
        return std::make_unique<Backends::CpuDct2Backend>();
    }

    // Создаем запрошенный бэкенд
    switch (type) {
        case BackendType::CuFFT:
            SAFE_PRINT_LINE("[BackendFactory] Создан cuFFT backend (NVIDIA: " + 
                           hwDetector->getDeviceName() + ")");
            return std::make_unique<Backends::CuFFTBackend>();

        case BackendType::VkFFT: {
            SAFE_PRINT_LINE("[BackendFactory] Создан VkFFT backend (GPU: " + 
                           hwDetector->getDeviceName() + ")");
            
            // Если передан VulkanEngine, используем его контекст
            if (vulkanEngine) {
                SAFE_PRINT_LINE("[BackendFactory] VkFFT: используется Vulkan контекст из VulkanEngine");
                return std::make_unique<Backends::VkFFTBackend>(
                    vulkanEngine->getInstance(),
                    vulkanEngine->getPhysicalDevice(),
                    vulkanEngine->getDevice()
                );
            } else {
                // Если VulkanEngine не передан, но есть HardwareDetector
                SAFE_WARNING("[BackendFactory] VkFFT: VulkanEngine не передан, используется только physical device");
                SAFE_WARNING("[BackendFactory] VkFFT требует полный Vulkan контекст - используйте перегрузку с отдельными объектами");
                return std::make_unique<Backends::VkFFTBackend>(
                    vk::Instance{},
                    hwDetector->getPhysicalDevice(),
                    vk::Device{}
                );
            }
        }

        case BackendType::CpuDct2:
            SAFE_PRINT_LINE("[BackendFactory] Создан CpuDct2 backend (Math.md)");
            return std::make_unique<Backends::CpuDct2Backend>();

        case BackendType::Simple:
            SAFE_PRINT_LINE("[BackendFactory] Создан Simple backend");
            return std::make_unique<Backends::SimpleDctBackend>();

        default:
            SAFE_ERROR("[BackendFactory] Неизвестный тип бэкенда");
            return nullptr;
    }
}

std::unique_ptr<IFrequencyBackend> BackendFactory::createWithHardwareDetection(
    BackendType type,
    SpectraForge::Vulkan::HardwareDetector* hwDetector,
    vk::Instance instance,
    vk::PhysicalDevice physicalDevice,
    vk::Device device) {
    
    // Если HardwareDetector не передан, fallback на обычный create
    if (!hwDetector) {
        SAFE_WARNING("[BackendFactory] HardwareDetector не передан, используется compile-time выбор");
        return create(type);
    }

    // Для Auto режима: умный выбор на основе железа
    if (type == BackendType::Auto) {
        BackendType selectedType = selectBestBackend(hwDetector);
        SAFE_PRINT_LINE("[BackendFactory] Авто-выбор (hardware-aware): " + getTypeName(selectedType));
        
        // Рекурсивно создаем выбранный бэкенд с переданными Vulkan объектами
        return createWithHardwareDetection(selectedType, hwDetector, instance, physicalDevice, device);
    }

    // Для явно указанного типа: проверяем доступность на железе
    if (!isAvailableOnHardware(type, hwDetector)) {
        SAFE_ERROR("[BackendFactory] Бэкенд " + getTypeName(type) + 
                   " недоступен на текущем железе (" + hwDetector->getDeviceName() + ")");
        
        // Fallback на CpuDct2
        SAFE_WARNING("[BackendFactory] Fallback на CpuDct2 (CPU)");
        return std::make_unique<Backends::CpuDct2Backend>();
    }

    // Создаем запрошенный бэкенд
    switch (type) {
        case BackendType::CuFFT:
            SAFE_PRINT_LINE("[BackendFactory] Создан cuFFT backend (NVIDIA: " + 
                           hwDetector->getDeviceName() + ")");
            return std::make_unique<Backends::CuFFTBackend>();

        case BackendType::VkFFT: {
            SAFE_PRINT_LINE("[BackendFactory] Создан VkFFT backend (GPU: " + 
                           hwDetector->getDeviceName() + ")");
            
            // Используем переданные Vulkan объекты
            if (instance && device) {
                SAFE_PRINT_LINE("[BackendFactory] VkFFT: используется переданный Vulkan контекст");
                return std::make_unique<Backends::VkFFTBackend>(
                    instance,
                    physicalDevice,
                    device
                );
            } else {
                SAFE_ERROR("[BackendFactory] VkFFT: некорректный Vulkan контекст (пустые instance или device)");
                SAFE_WARNING("[BackendFactory] Fallback на CpuDct2");
                return std::make_unique<Backends::CpuDct2Backend>();
            }
        }

        case BackendType::CpuDct2:
            SAFE_PRINT_LINE("[BackendFactory] Создан CpuDct2 backend (Math.md)");
            return std::make_unique<Backends::CpuDct2Backend>();

        case BackendType::Simple:
            SAFE_PRINT_LINE("[BackendFactory] Создан Simple backend");
            return std::make_unique<Backends::SimpleDctBackend>();

        default:
            SAFE_ERROR("[BackendFactory] Неизвестный тип бэкенда");
            return nullptr;
    }
}

bool BackendFactory::isAvailable(BackendType type) {
    // Compile-time проверка доступности бэкенда
    switch (type) {
        case BackendType::Auto:
            return true;  // Auto всегда доступен (выбирает лучший)
            
        case BackendType::CuFFT:
#ifdef HYPERENGINE_CUDA_AVAILABLE
            return true;  // Доступен, если скомпилирован с CUDA
#else
            return false;
#endif

        case BackendType::VkFFT:
            return true;  // VkFFT всегда доступен (через Vulkan)
            
        case BackendType::CpuDct2:
            return true;  // CpuDct2 всегда доступен (pure CPU)
            
        case BackendType::Simple:
            return true;  // Simple всегда доступен (не требует ничего)
            
        default:
            return false;
    }
}

bool BackendFactory::isAvailableOnHardware(
    BackendType type,
    SpectraForge::Vulkan::HardwareDetector* hwDetector) {
    
    if (!hwDetector) {
        // Без HardwareDetector используем только compile-time проверки
        return isAvailable(type);
    }

    // Runtime проверка доступности на конкретном железе
    switch (type) {
        case BackendType::Auto:
            return true;  // Auto всегда доступен
            
        case BackendType::CuFFT: {
#ifdef HYPERENGINE_CUDA_AVAILABLE
            // Проверяем: NVIDIA GPU + CUDA поддержка
            auto vendor = hwDetector->detectVendor();
            bool cudaSupported = hwDetector->supportsCUDA();
            
            if (vendor != SpectraForge::Vulkan::VendorType::NVIDIA) {
                SAFE_WARNING("[BackendFactory] cuFFT недоступен: не NVIDIA GPU (" + 
                           hwDetector->getDeviceName() + ")");
                return false;
            }
            
            if (!cudaSupported) {
                SAFE_WARNING("[BackendFactory] cuFFT недоступен: CUDA не поддерживается runtime");
                return false;
            }
            
            return true;
#else
            SAFE_WARNING("[BackendFactory] cuFFT недоступен: не скомпилирован с CUDA");
            return false;
#endif
        }
            
        case BackendType::VkFFT:
            // VkFFT доступен на любом Vulkan GPU
            SAFE_PRINT_LINE("[BackendFactory] VkFFT доступен на " + hwDetector->getDeviceName());
            return true;
            
        case BackendType::CpuDct2:
            // CpuDct2 всегда доступен (CPU fallback)
            return true;
            
        case BackendType::Simple:
            // Simple всегда доступен
            return true;
            
        default:
            return false;
    }
}

BackendFactory::BackendType BackendFactory::selectBestBackend(
    SpectraForge::Vulkan::HardwareDetector* hwDetector) {
    
    if (!hwDetector) {
        SAFE_WARNING("[BackendFactory] selectBestBackend: HardwareDetector не передан");
        
        // Fallback на compile-time выбор
#ifdef HYPERENGINE_CUDA_AVAILABLE
        return BackendType::CuFFT;
#else
        return BackendType::VkFFT;
#endif
    }

    SAFE_PRINT_LINE("[BackendFactory] === Анализ железа для выбора оптимального бэкенда ===");
    SAFE_PRINT_LINE("[BackendFactory] GPU: " + hwDetector->getDeviceName());
    
    auto vendor = hwDetector->detectVendor();
    std::string vendorName;
    switch (vendor) {
        case SpectraForge::Vulkan::VendorType::NVIDIA:
            vendorName = "NVIDIA";
            break;
        case SpectraForge::Vulkan::VendorType::AMD:
            vendorName = "AMD";
            break;
        case SpectraForge::Vulkan::VendorType::INTEL:
            vendorName = "Intel";
            break;
        default:
            vendorName = "Unknown";
            break;
    }
    SAFE_PRINT_LINE("[BackendFactory] Вендор: " + vendorName);

    // Приоритет 1: NVIDIA + CUDA → cuFFT (самый быстрый)
    if (vendor == SpectraForge::Vulkan::VendorType::NVIDIA) {
#ifdef HYPERENGINE_CUDA_AVAILABLE
        bool cudaSupported = hwDetector->supportsCUDA();
        SAFE_PRINT_LINE("[BackendFactory] CUDA поддержка: " + 
                       std::string(cudaSupported ? "ДА" : "НЕТ"));
        
        if (cudaSupported) {
            SAFE_PRINT_LINE("[BackendFactory] ✅ Выбран cuFFT (NVIDIA + CUDA)");
            SAFE_PRINT_LINE("[BackendFactory] Ожидаемое ускорение: ~10-20x против CPU");
            return BackendType::CuFFT;
        } else {
            SAFE_WARNING("[BackendFactory] NVIDIA GPU найден, но CUDA runtime недоступен");
            SAFE_PRINT_LINE("[BackendFactory] Fallback на VkFFT");
        }
#else
        SAFE_WARNING("[BackendFactory] NVIDIA GPU найден, но CUDA не скомпилирован");
        SAFE_PRINT_LINE("[BackendFactory] Fallback на VkFFT");
#endif
    }

    // Приоритет 2: Любой Vulkan GPU → VkFFT (хорошая производительность)
    if (vendor == SpectraForge::Vulkan::VendorType::AMD ||
        vendor == SpectraForge::Vulkan::VendorType::INTEL ||
        vendor == SpectraForge::Vulkan::VendorType::NVIDIA) {
        
        SAFE_PRINT_LINE("[BackendFactory] ✅ Выбран VkFFT (" + vendorName + " GPU через Vulkan)");
        SAFE_PRINT_LINE("[BackendFactory] Ожидаемое ускорение: ~5-10x против CPU");
        return BackendType::VkFFT;
    }

    // Приоритет 3: Fallback на CpuDct2 (CPU DCT-II согласно Math.md)
    SAFE_WARNING("[BackendFactory] ⚠️ Неизвестный вендор GPU, используется CpuDct2 backend");
    SAFE_WARNING("[BackendFactory] Производительность будет низкой (CPU-only)!");
    return BackendType::CpuDct2;
}

std::string BackendFactory::getTypeName(BackendType type) {
    switch (type) {
        case BackendType::Auto:    return "Auto";
        case BackendType::CuFFT:   return "cuFFT";
        case BackendType::VkFFT:   return "VkFFT";
        case BackendType::CpuDct2: return "CpuDct2";
        case BackendType::Simple:  return "Simple";
        default:                   return "Unknown";
    }
}

} // namespace SpectraForge::Rendering::FreqVox

