/**
 * @file BackendFactory.cpp
 */

#include "HyperEngine/Rendering/FreqVox/BackendFactory.h"
#include "HyperEngine/Rendering/FreqVox/Backends/SimpleDctBackend.h"
#include "HyperEngine/Rendering/FreqVox/Backends/CuFFTBackend.h"
#include "HyperEngine/Rendering/FreqVox/Backends/VkFFTBackend.h"
#include "HyperEngine/Core/SafeConsole.h"

namespace HyperEngine::Rendering::FreqVox {

std::unique_ptr<IFrequencyBackend> BackendFactory::create(BackendType type) {
    if (type == BackendType::Auto) {
        // Автовыбор: приоритет cuFFT > VkFFT > Simple
#ifdef HYPERENGINE_CUDA_AVAILABLE
        SAFE_PRINT_LINE("[BackendFactory] Авто-выбор: cuFFT (CUDA доступна)");
        return std::make_unique<Backends::CuFFTBackend>();
#else
        SAFE_PRINT_LINE("[BackendFactory] Авто-выбор: VkFFT (CUDA недоступна)");
        return std::make_unique<Backends::VkFFTBackend>();
#endif
    }

    switch (type) {
        case BackendType::CuFFT:
#ifdef HYPERENGINE_CUDA_AVAILABLE
            SAFE_PRINT_LINE("[BackendFactory] Создан cuFFT backend");
            return std::make_unique<Backends::CuFFTBackend>();
#else
            SAFE_ERROR("[BackendFactory] cuFFT запрошен, но CUDA недоступна");
            return nullptr;
#endif

        case BackendType::VkFFT:
            SAFE_PRINT_LINE("[BackendFactory] Создан VkFFT backend");
            return std::make_unique<Backends::VkFFTBackend>();

        case BackendType::Simple:
            SAFE_PRINT_LINE("[BackendFactory] Создан Simple backend");
            return std::make_unique<Backends::SimpleDctBackend>();

        case BackendType::Auto:
            // Fallthrough to default
            [[fallthrough]];
        default:
            SAFE_ERROR("[BackendFactory] Неизвестный тип бэкенда");
            return nullptr;
    }
}

bool BackendFactory::isAvailable(BackendType type) {
    switch (type) {
        case BackendType::Auto:
            return true;
        case BackendType::CuFFT:
#ifdef HYPERENGINE_CUDA_AVAILABLE
            return true;
#else
            return false;
#endif
        case BackendType::VkFFT:
            return true;
        case BackendType::Simple:
            return true;
        default:
            return false;
    }
}

std::string BackendFactory::getTypeName(BackendType type) {
    switch (type) {
        case BackendType::Auto:   return "Auto";
        case BackendType::CuFFT:  return "cuFFT";
        case BackendType::VkFFT:  return "VkFFT";
        case BackendType::Simple: return "Simple";
        default:                  return "Unknown";
    }
}

} // namespace HyperEngine::Rendering::FreqVox

