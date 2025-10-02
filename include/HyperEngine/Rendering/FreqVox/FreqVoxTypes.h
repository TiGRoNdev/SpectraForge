/**
 * @file FreqVoxTypes.h
 * @brief Типы данных для FreqVox Renderer (AFS-NVR)
 */

#pragma once

#include <cstdint>
#include <array>
#include <memory>
#include "HyperEngine/Math/Vector3.h"

namespace HyperEngine::Rendering::FreqVox {

/**
 * @brief SH коэффициенты до порядка l=2 (9 коэффициентов на канал)
 */
struct SH9 {
    std::array<float, 9> r{};
    std::array<float, 9> g{};
    std::array<float, 9> b{};
};

/**
 * @brief Базовый формат вокселя для FreqVox
 */
struct Voxel {
    Math::Vector3 position{0.0f, 0.0f, 0.0f};
    float lod_bias = 0.0f;
    SH9 radiance_sh;  ///< Низкочастотная радиансная информация
};

/**
 * @brief Параметры DCT блока (например, 8x8)
 */
struct DctBlockConfig {
    uint32_t width = 8;
    uint32_t height = 8;
    uint32_t batch_size = 0;  ///< число блоков в батче
};

/**
 * @brief Параметры фовеации
 */
struct FoveatedParams {
    float foveal_radius = 10.0f;    ///< Радиус фовеальной области в метрах
    float foveal_angle = 10.0f;     ///< Угол фовеальной области в градусах
    float peripheral_sigma = 5.0f;  ///< Сигма для Гауссова спада
    float far_plane = 100.0f;       ///< Дальняя плоскость отсечения
};

/**
 * @brief Параметры темпоральной репроекции
 */
struct TemporalReprojectionParams {
    float blendFactor = 0.2f;                 ///< Фактор смешивания (alpha)
    float motionVectorThreshold = 2.0f;       ///< Порог величины motion vector в пикселях
    float depthChangeThreshold = 0.01f;       ///< Порог изменения глубины
};

/**
 * @brief Параметры нейронного апскейлинга
 */
struct NeuralUpscalingParams {
    float upscaleFactor = 2.0f;  ///< Коэффициент увеличения разрешения
};

}  // namespace HyperEngine::Rendering::FreqVox


