/**
 * @file NeuralUpscaling.h
 * @brief Neural upscaling module (DLSS/FSR2/TinyCNN)
 */

#pragma once

#include "FreqVoxTypes.h"
#include <vector>
#include <memory>
#include <string>

namespace HyperEngine::Rendering::FreqVox {

/**
 * @brief Тип апскейлера
 */
enum class UpscalerType {
    Auto,      ///< Автоматический выбор
    DLSS,      ///< NVIDIA DLSS (Streamline)
    FSR2,      ///< AMD FSR 2.x
    TinyCNN,   ///< Простая CNN для fallback
    Bilinear   ///< Простое билинейное масштабирование
};

/**
 * @brief Модуль нейронного апскейлинга
 * 
 * Предоставляет унифицированный интерфейс для различных технологий апскейлинга:
 * - NVIDIA DLSS (через Streamline)
 * - AMD FSR2
 * - Fallback Tiny CNN
 */
class NeuralUpscaler {
public:
    NeuralUpscaler();
    ~NeuralUpscaler();

    /**
     * @brief Инициализация апскейлера
     * @param type Тип апскейлера (Auto для автовыбора)
     * @param input_width Ширина входного изображения
     * @param input_height Высота входного изображения
     * @param params Параметры апскейлинга
     * @return true если инициализация успешна
     */
    bool initialize(UpscalerType type,
                    uint32_t input_width,
                    uint32_t input_height,
                    const NeuralUpscalingParams& params);

    /**
     * @brief Применить апскейлинг
     * @param input_image Входное изображение (низкое разрешение)
     * @param output_image Выходное изображение (высокое разрешение)
     * @param motion_vectors Опциональные векторы движения для DLSS/FSR2
     * @return true если апскейлинг выполнен успешно
     */
    bool upscale(const std::vector<float>& input_image,
                 std::vector<float>& output_image,
                 const std::vector<float>* motion_vectors = nullptr);

    /**
     * @brief Получить размер выходного изображения
     * @return Ширина и высота выходного изображения
     */
    std::pair<uint32_t, uint32_t> getOutputSize() const {
        return {output_width_, output_height_};
    }

    /**
     * @brief Получить используемый тип апскейлера
     */
    UpscalerType getActiveType() const { return active_type_; }

    /**
     * @brief Получить имя активного апскейлера
     */
    std::string getActiveName() const;

    void shutdown();

private:
    UpscalerType active_type_ = UpscalerType::Bilinear;
    uint32_t input_width_ = 0;
    uint32_t input_height_ = 0;
    uint32_t output_width_ = 0;
    uint32_t output_height_ = 0;
    NeuralUpscalingParams params_;
    bool initialized_ = false;

    // Вспомогательные методы
    UpscalerType selectBestUpscaler() const;
    bool upscaleBilinear(const std::vector<float>& input, std::vector<float>& output);

    // Запрет копирования
    NeuralUpscaler(const NeuralUpscaler&) = delete;
    NeuralUpscaler& operator=(const NeuralUpscaler&) = delete;
};

} // namespace HyperEngine::Rendering::FreqVox

