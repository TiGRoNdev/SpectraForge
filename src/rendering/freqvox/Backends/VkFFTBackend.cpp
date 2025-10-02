/**
 * @file VkFFTBackend.cpp
 */

#include "HyperEngine/Rendering/FreqVox/Backends/VkFFTBackend.h"
#include "HyperEngine/Core/SafeConsole.h"

namespace HyperEngine::Rendering::FreqVox::Backends {

VkFFTBackend::VkFFTBackend() {
    SAFE_PRINT_LINE("[VkFFTBackend] Конструктор вызван (заглушка)");
}

VkFFTBackend::~VkFFTBackend() {
    shutdown();
}

bool VkFFTBackend::initialize(const DctBlockConfig& config) {
    if (initialized_) {
        SAFE_WARNING("[VkFFTBackend] Уже инициализирован");
        return true;
    }

    cfg_ = config;
    
    // TODO: Интеграция VkFFT через header-only библиотеку
    // Сейчас - минимальная заглушка для сборки
    
    initialized_ = true;
    SAFE_PRINT_LINE("[VkFFTBackend] Инициализирован (заглушка)");
    return true;
}

void VkFFTBackend::shutdown() {
    if (!initialized_) return;
    
    initialized_ = false;
    SAFE_PRINT_LINE("[VkFFTBackend] Завершение работы");
}

bool VkFFTBackend::transform_forward(std::vector<float>& io_block_batched) {
    if (!initialized_) {
        SAFE_ERROR("[VkFFTBackend] Не инициализирован");
        return false;
    }

    // Заглушка: тождественное преобразование
    (void)io_block_batched;
    return true;
}

bool VkFFTBackend::transform_inverse(std::vector<float>& io_block_batched) {
    if (!initialized_) {
        SAFE_ERROR("[VkFFTBackend] Не инициализирован");
        return false;
    }

    // Заглушка: тождественное преобразование
    (void)io_block_batched;
    return true;
}

} // namespace HyperEngine::Rendering::FreqVox::Backends

