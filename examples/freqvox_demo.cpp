/**
 * @file freqvox_demo.cpp
 * @brief Демонстрация FreqVox Renderer
 */

#include "HyperEngine/Rendering/FreqVox/FreqVoxStrategy.h"
#include "HyperEngine/Rendering/FreqVox/BackendFactory.h"
#include "HyperEngine/Rendering/StrategyFactory.h"
#include "HyperEngine/Core/SafeConsole.h"
#include <iostream>

using namespace HyperEngine::Rendering;
using namespace HyperEngine::Rendering::FreqVox;

int main() {
    std::cout << "=== FreqVox Renderer Demo ===" << std::endl;

    // 1. Проверка доступности бэкендов
    std::cout << "\nДоступные FFT бэкенды:" << std::endl;
    std::cout << "  - Auto: " << (BackendFactory::isAvailable(BackendFactory::BackendType::Auto) ? "ДА" : "НЕТ") << std::endl;
    std::cout << "  - cuFFT: " << (BackendFactory::isAvailable(BackendFactory::BackendType::CuFFT) ? "ДА" : "НЕТ") << std::endl;
    std::cout << "  - VkFFT: " << (BackendFactory::isAvailable(BackendFactory::BackendType::VkFFT) ? "ДА" : "НЕТ") << std::endl;
    std::cout << "  - Simple: " << (BackendFactory::isAvailable(BackendFactory::BackendType::Simple) ? "ДА" : "НЕТ") << std::endl;

    // 2. Создание FFT бэкенда
    std::cout << "\nСоздание FFT бэкенда (Auto)..." << std::endl;
    auto backend = BackendFactory::create(BackendFactory::BackendType::Auto);
    if (!backend) {
        SAFE_ERROR("Не удалось создать FFT бэкенд!");
        return 1;
    }
    std::cout << "Выбран бэкенд: " << backend->getName() << std::endl;

    // 3. Инициализация бэкенда
    DctBlockConfig config;
    config.block_width = 8;
    config.block_height = 8;
    config.batch_count = 4;

    std::cout << "\nИнициализация бэкенда (блоки " << config.block_width << "x" << config.block_height 
              << ", батчей=" << config.batch_count << ")..." << std::endl;
    
    if (!backend->initialize(config)) {
        SAFE_ERROR("Ошибка инициализации бэкенда!");
        return 1;
    }
    std::cout << "Бэкенд инициализирован успешно" << std::endl;

    // 4. Тест прямого и обратного преобразования
    size_t data_size = config.block_width * config.block_height * config.batch_count;
    std::vector<float> test_data(data_size);
    
    // Заполняем тестовыми данными
    for (size_t i = 0; i < data_size; ++i) {
        test_data[i] = static_cast<float>(i % 10);
    }

    std::cout << "\nВыполнение прямого DCT..." << std::endl;
    if (!backend->transform_forward(test_data)) {
        SAFE_ERROR("Ошибка прямого преобразования!");
        backend->shutdown();
        return 1;
    }
    std::cout << "Прямое DCT выполнено" << std::endl;

    std::cout << "\nВыполнение обратного DCT..." << std::endl;
    if (!backend->transform_inverse(test_data)) {
        SAFE_ERROR("Ошибка обратного преобразования!");
        backend->shutdown();
        return 1;
    }
    std::cout << "Обратное DCT выполнено" << std::endl;

    // 5. Создание FreqVox стратегии через фабрику
    std::cout << "\nСоздание FreqVox стратегии через фабрику..." << std::endl;
    auto strategy = create_strategy_by_name("freqvox");
    if (!strategy) {
        SAFE_ERROR("Не удалось создать FreqVox стратегию (возможно ENABLE_FREQVOX=OFF)!");
        backend->shutdown();
        return 1;
    }
    std::cout << "Стратегия создана: " << strategy->getName() << std::endl;

    // 6. Инициализация стратегии
    std::cout << "\nИнициализация FreqVox стратегии..." << std::endl;
    if (!strategy->initialize()) {
        SAFE_ERROR("Ошибка инициализации FreqVox стратегии!");
        backend->shutdown();
        return 1;
    }
    std::cout << "FreqVox стратегия инициализирована" << std::endl;

    // 7. Cleanup
    std::cout << "\nЗавершение работы..." << std::endl;
    strategy->shutdown();
    backend->shutdown();

    std::cout << "\n=== FreqVox Demo завершено успешно ===" << std::endl;
    return 0;
}

