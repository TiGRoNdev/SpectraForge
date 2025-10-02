/**
 * @file FreqVoxTypes.h
 * @brief Типы данных для FreqVox Renderer (AFS-NVR)
 */

#pragma once

#include <cstdint>
#include <array>
#include <memory>
#include "SpectraForge/Math/Vector3.h"

namespace SpectraForge::Rendering::FreqVox {

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
    uint32_t blockSize = 8;     ///< Размер блока (квадратный: blockSize×blockSize)
    uint32_t batchCount = 1;    ///< Число блоков в батче
};

/**
 * @brief Параметры фовеации (согласно Math.md раздел 3)
 * 
 * Фовеация использует Gaussian weighting:
 * w_i = exp(-φ_i² / 2σ²)
 * 
 * где φ_i - angular distance от gaze center,
 * σ - foveal radius (стандартное отклонение).
 */
struct FoveatedParams {
    Math::Vector3 gaze_center{0.0f, 0.0f, 1.0f};  ///< Направление взгляда (normalized)
    Math::Vector3 eye_position{0.0f, 0.0f, 0.0f}; ///< Позиция камеры в мировых координатах
    
    float foveal_sigma_deg = 5.0f;   ///< σ для Gaussian weight в градусах (Math.md: ~5°)
    float far_plane = 100.0f;        ///< Дальняя плоскость отсечения
    float weight_threshold = 0.01f;  ///< Минимальный вес для включения вокселя (оптимизация)
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

}  // namespace SpectraForge::Rendering::FreqVox


