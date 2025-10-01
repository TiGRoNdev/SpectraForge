/**
 * @file BackendFactory.h
 * @brief Factory for creating FFT/DCT backends based on available hardware
 */

#pragma once

#include "HyperEngine/Rendering/FreqVox/FrequencyShading.h"
#include <memory>
#include <string>

namespace HyperEngine::Rendering::FreqVox {

/**
 * @brief Фабрика для создания оптимального FFT/DCT бэкенда
 * 
 * Автоматически выбирает лучший доступный бэкенд:
 * - cuFFT (если CUDA доступна)
 * - VkFFT (кроссплатформенный fallback)
 * - SimpleDCT (минимальная заглушка для тестов)
 */
class BackendFactory {
public:
    enum class BackendType {
        Auto,      ///< Автоматический выбор лучшего
        CuFFT,     ///< NVIDIA cuFFT (требует CUDA)
        VkFFT,     ///< VkFFT через Vulkan
        Simple     ///< Простая заглушка
    };

    /**
     * @brief Создать бэкенд указанного типа
     * @param type Тип бэкенда (Auto для автовыбора)
     * @return Умный указатель на бэкенд или nullptr при ошибке
     */
    static std::unique_ptr<IFrequencyBackend> create(BackendType type = BackendType::Auto);

    /**
     * @brief Проверить доступность типа бэкенда
     * @param type Тип бэкенда
     * @return true если бэкенд доступен на текущей платформе
     */
    static bool isAvailable(BackendType type);

    /**
     * @brief Получить имя типа бэкенда
     * @param type Тип бэкенда
     * @return Строковое представление
     */
    static std::string getTypeName(BackendType type);

private:
    BackendFactory() = delete;
};

} // namespace HyperEngine::Rendering::FreqVox

