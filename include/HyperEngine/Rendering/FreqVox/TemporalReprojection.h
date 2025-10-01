/**
 * @file TemporalReprojection.h
 * @brief Temporal reprojection and reconstruction with motion vectors
 */

#pragma once

#include "FreqVoxTypes.h"
#include "HyperEngine/Math/Vector3.h"
#include "HyperEngine/Math/Matrix4.h"
#include <vector>
#include <memory>

namespace HyperEngine::Rendering::FreqVox {

/**
 * @brief Модуль темпоральной репроекции
 * 
 * Реализует переиспользование данных предыдущего кадра с motion vectors
 * для снижения шума и увеличения стабильности изображения.
 */
class TemporalReprojection {
public:
    TemporalReprojection();
    ~TemporalReprojection();

    /**
     * @brief Инициализация модуля
     * @param width Ширина изображения
     * @param height Высота изображения
     * @param params Параметры репроекции
     * @return true если инициализация успешна
     */
    bool initialize(uint32_t width, uint32_t height, const TemporalReprojectionParams& params);

    /**
     * @brief Применить темпоральную репроекцию
     * @param current_frame Текущий кадр
     * @param motion_vectors Векторы движения пикселей
     * @param depth_buffer Буфер глубины
     * @param output_frame Выходной кадр с репроекцией
     * @return true если репроекция выполнена успешно
     */
    bool reproject(const std::vector<float>& current_frame,
                   const std::vector<Math::Vector3>& motion_vectors,
                   const std::vector<float>& depth_buffer,
                   std::vector<float>& output_frame);

    /**
     * @brief Сброс истории
     */
    void resetHistory();

    /**
     * @brief Обновить параметры
     * @param params Новые параметры репроекции
     */
    void updateParams(const TemporalReprojectionParams& params);

    void shutdown();

private:
    uint32_t width_ = 0;
    uint32_t height_ = 0;
    TemporalReprojectionParams params_;
    bool initialized_ = false;

    // Буферы предыдущего кадра
    std::vector<float> previous_frame_;
    std::vector<float> previous_depth_;
    
    bool hasHistory_ = false;

    // Вспомогательные методы
    bool validateMotionVector(const Math::Vector3& mv, float current_depth, float prev_depth) const;
    float calculateBlendFactor(const Math::Vector3& mv, float depth_diff) const;

    // Запрет копирования
    TemporalReprojection(const TemporalReprojection&) = delete;
    TemporalReprojection& operator=(const TemporalReprojection&) = delete;
};

} // namespace HyperEngine::Rendering::FreqVox

