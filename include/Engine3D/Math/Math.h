#pragma once

/**
 * @file Math.h
 * @brief Основной заголовочный файл математической библиотеки 3D движка
 *
 * Включает все основные математические классы и функции для работы с 3D графикой:
 * - Vector2: 2D векторы
 * - Vector3: 3D векторы
 * - Matrix4: 4x4 матрицы для трансформаций
 * - Quaternion: кватернионы для поворотов
 */

#include "Matrix4.h"
#include "Quaternion.h"
#include "Vector2.h"
#include "Vector3.h"

namespace Engine3D {
namespace Math {

// Математические константы
constexpr float PI = 3.14159265358979323846f;
constexpr float TWO_PI = 2.0f * PI;
constexpr float HALF_PI = PI * 0.5f;
constexpr float DEG_TO_RAD = PI / 180.0f;
constexpr float RAD_TO_DEG = 180.0f / PI;
constexpr float EPSILON = 1e-6f;

// Утилитарные функции
inline float toRadians(float degrees) {
    return degrees * DEG_TO_RAD;
}

inline float toDegrees(float radians) {
    return radians * RAD_TO_DEG;
}

inline float clamp(float value, float min, float max) {
    return value < min ? min : (value > max ? max : value);
}

inline float lerp(float a, float b, float t) {
    return a + (b - a) * clamp(t, 0.0f, 1.0f);
}

inline bool isNearlyEqual(float a, float b, float epsilon = EPSILON) {
    return std::abs(a - b) < epsilon;
}

inline bool isNearlyZero(float value, float epsilon = EPSILON) {
    return std::abs(value) < epsilon;
}

}  // namespace Math
}  // namespace Engine3D
