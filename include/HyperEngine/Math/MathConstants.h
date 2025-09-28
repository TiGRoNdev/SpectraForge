#pragma once

#include <cmath>

namespace HyperEngine {
namespace Math {

/**
 * @brief Математические константы для движка
 */
namespace Constants {
    constexpr float PI = 3.14159265358979323846f;
    constexpr float TWO_PI = 2.0f * PI;
    constexpr float HALF_PI = PI / 2.0f;
    constexpr float QUARTER_PI = PI / 4.0f;
    constexpr float INV_PI = 1.0f / PI;
    constexpr float DEG_TO_RAD = PI / 180.0f;
    constexpr float RAD_TO_DEG = 180.0f / PI;
    constexpr float EPSILON = 1e-6f;
    constexpr float FLOAT_EPSILON = 1.19209290e-07f;
}

// Для совместимости с существующим кодом
#ifndef M_PI
#define M_PI Constants::PI
#endif

} // namespace Math
} // namespace HyperEngine
