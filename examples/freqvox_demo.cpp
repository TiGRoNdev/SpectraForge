/**
 * @file freqvox_demo.cpp
 * @brief Демонстрация FreqVox Renderer с compile-time выбором бэкенда
 * 
 * ПРИМЕЧАНИЕ: Для упрощения, это демо использует compile-time выбор бэкенда.
 * Для hardware-aware выбора см. документацию docs/FreqVox_HardwareDetection.md
 * и используйте BackendFactory::createWithHardwareDetection() в вашем приложении
 * с уже инициализированным HardwareDetector из VulkanRenderer.
 */

#include "SpectraForge/Rendering/FreqVox/FreqVoxStrategy.h"
#include "SpectraForge/Rendering/FreqVox/BackendFactory.h"
#include "SpectraForge/Rendering/StrategyFactory.h"
#include "SpectraForge/Core/SafeConsole.h"
#include <iostream>

using namespace SpectraForge::Rendering;
using namespace SpectraForge::Rendering::FreqVox;

int main() {
    std::cout << "=== FreqVox Renderer Demo ===" << std::endl;

    // 1. Информация о hardware-aware выборе
    std::cout << "\n[1/7] Режим работы: Compile-time выбор бэкенда" << std::endl;
    std::cout << "📝 ПРИМЕЧАНИЕ: Это упрощенное демо для тестирования базовых функций" << std::endl;
    std::cout << "💡 Для hardware-aware выбора см. docs/FreqVox_HardwareDetection.md" << std::endl;
    std::cout << "   Используйте BackendFactory::createWithHardwareDetection() в реальном приложении" << std::endl;

    // 2. Проверка доступности бэкендов (compile-time)
    std::cout << "\n[2/7] Проверка доступности бэкендов (compile-time):" << std::endl;
    std::cout << "  - Auto: " << (BackendFactory::isAvailable(BackendFactory::BackendType::Auto) ? "✅ ДА" : "❌ НЕТ") << std::endl;
    std::cout << "  - cuFFT: " << (BackendFactory::isAvailable(BackendFactory::BackendType::CuFFT) ? "✅ ДА" : "❌ НЕТ") << std::endl;
    std::cout << "  - VkFFT: " << (BackendFactory::isAvailable(BackendFactory::BackendType::VkFFT) ? "✅ ДА" : "❌ НЕТ") << std::endl;
    std::cout << "  - Simple: " << (BackendFactory::isAvailable(BackendFactory::BackendType::Simple) ? "✅ ДА" : "❌ НЕТ") << std::endl;

    // 3. Пропускаем runtime проверку (без HardwareDetector)
    std::cout << "\n[3/7] Пропускаем runtime проверку (HardwareDetector недоступен в этом демо)" << std::endl;
    std::cout << "💡 Для hardware-aware выбора используйте createWithHardwareDetection()" << std::endl;

    // 4. Создание FFT бэкенда с fallback логикой
    std::cout << "\n[4/7] Создание FFT бэкенда (Auto, compile-time)..." << std::endl;
    std::cout << "📝 Используется BackendFactory::create() для простоты" << std::endl;
    
    auto backend = BackendFactory::create(BackendFactory::BackendType::Auto);
    
    if (!backend) {
        SAFE_ERROR("Не удалось создать FFT бэкенд!");
        return 1;
    }
    std::cout << "✅ FFT бэкенд создан успешно" << std::endl;

    // 5. Инициализация бэкенда с fallback на другие бэкенды при ошибке
    std::cout << "\n[5/7] Инициализация бэкенда..." << std::endl;
    DctBlockConfig config;
    config.blockSize = 8;
    config.batchCount = 4;

    std::cout << "  Параметры: блоки " << config.blockSize << "x" << config.blockSize 
              << ", батчей=" << config.batchCount << std::endl;
    
    if (!backend->initialize(config)) {
        SAFE_WARNING("Ошибка инициализации первого бэкенда! Пробуем fallback...");
        
        // Пробуем VkFFT
        std::cout << "  Пробуем VkFFT бэкенд..." << std::endl;
        backend = BackendFactory::create(BackendFactory::BackendType::VkFFT);
        if (backend && backend->initialize(config)) {
            std::cout << "  ✅ VkFFT бэкенд инициализирован успешно" << std::endl;
        } else {
            SAFE_WARNING("VkFFT также не удалось инициализировать. Пробуем Simple...");
            
            // Последний fallback - Simple
            std::cout << "  Пробуем Simple бэкенд..." << std::endl;
            backend = BackendFactory::create(BackendFactory::BackendType::Simple);
            if (backend && backend->initialize(config)) {
                std::cout << "  ✅ Simple бэкенд инициализирован успешно" << std::endl;
            } else {
                SAFE_ERROR("Не удалось инициализировать ни один бэкенд!");
                return 1;
            }
        }
    } else {
        std::cout << "✅ Бэкенд инициализирован успешно" << std::endl;
    }

    // 6. Тест прямого и обратного преобразования
    std::cout << "\n[6/7] Тестирование FFT преобразований..." << std::endl;
    size_t data_size = config.blockSize * config.blockSize * config.batchCount;
    std::vector<float> test_data(data_size);
    
    // Заполняем тестовыми данными
    for (size_t i = 0; i < data_size; ++i) {
        test_data[i] = static_cast<float>(i % 10);
    }

    std::cout << "  Прямое DCT..." << std::endl;
    if (!backend->transform_forward(test_data)) {
        SAFE_ERROR("Ошибка прямого преобразования!");
        backend->shutdown();
        return 1;
    }
    std::cout << "  ✅ Прямое DCT выполнено" << std::endl;

    std::cout << "  Обратное DCT..." << std::endl;
    if (!backend->transform_inverse(test_data)) {
        SAFE_ERROR("Ошибка обратного преобразования!");
        backend->shutdown();
        return 1;
    }
    std::cout << "  ✅ Обратное DCT выполнено" << std::endl;

    // 7. Cleanup
    std::cout << "\n[7/7] Завершение работы..." << std::endl;
    std::cout << "  Shutdown бэкенда..." << std::endl;
    backend->shutdown();
    
    std::cout << "\n💡 ПРИМЕЧАНИЕ: Тест FreqVoxStrategy пропущен для упрощения" << std::endl;
    std::cout << "   FreqVoxStrategy требует инициализированный рендерер" << std::endl;

    std::cout << "\n✅ FreqVox Demo завершено успешно!" << std::endl;
    std::cout << "\n💡 СОВЕТ: Для лучшей производительности убедитесь, что:" << std::endl;
    std::cout << "   - Используется NVIDIA GPU с CUDA для cuFFT бэкенда" << std::endl;
    std::cout << "   - Драйверы Vulkan установлены корректно" << std::endl;
    std::cout << "   - CUDA Toolkit установлен (для cuFFT)" << std::endl;
    
    return 0;
}

