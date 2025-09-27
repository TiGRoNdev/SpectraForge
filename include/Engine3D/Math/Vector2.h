#pragma once

#include <cmath>
#include <string>

namespace Engine3D {
namespace Math {

/**
 * @brief 2D вектор для работы с координатами экрана и 2D операциями
 */
class Vector2 {
public:
    float x, y;

    // Конструкторы
    Vector2() : x(0.0f), y(0.0f) {}
    Vector2(float x_, float y_) : x(x_), y(y_) {}
    Vector2(const Vector2& other) : x(other.x), y(other.y) {}

    // Операторы присваивания
    Vector2& operator=(const Vector2& other) {
        if (this != &other) {
            x = other.x;
            y = other.y;
        }
        return *this;
    }

    // Арифметические операторы
    Vector2 operator+(const Vector2& other) const {
        return Vector2(x + other.x, y + other.y);
    }

    Vector2 operator-(const Vector2& other) const {
        return Vector2(x - other.x, y - other.y);
    }

    Vector2 operator*(float scalar) const {
        return Vector2(x * scalar, y * scalar);
    }

    Vector2 operator/(float scalar) const {
        return Vector2(x / scalar, y / scalar);
    }

    // Составные операторы присваивания
    Vector2& operator+=(const Vector2& other) {
        x += other.x;
        y += other.y;
        return *this;
    }

    Vector2& operator-=(const Vector2& other) {
        x -= other.x;
        y -= other.y;
        return *this;
    }

    Vector2& operator*=(float scalar) {
        x *= scalar;
        y *= scalar;
        return *this;
    }

    Vector2& operator/=(float scalar) {
        x /= scalar;
        y /= scalar;
        return *this;
    }

    // Операторы сравнения
    bool operator==(const Vector2& other) const {
        return (std::abs(x - other.x) < 1e-6f) && (std::abs(y - other.y) < 1e-6f);
    }

    bool operator!=(const Vector2& other) const {
        return !(*this == other);
    }

    // Доступ к элементам
    float& operator[](int index) {
        return (&x)[index];
    }

    const float& operator[](int index) const {
        return (&x)[index];
    }

    // Математические функции
    float magnitude() const {
        return std::sqrt(x * x + y * y);
    }

    float magnitudeSquared() const {
        return x * x + y * y;
    }

    Vector2 normalized() const {
        float mag = magnitude();
        if (mag > 1e-6f) {
            return *this / mag;
        }
        return Vector2(0.0f, 0.0f);
    }

    void normalize() {
        float mag = magnitude();
        if (mag > 1e-6f) {
            x /= mag;
            y /= mag;
        }
    }

    float dot(const Vector2& other) const {
        return x * other.x + y * other.y;
    }

    float distance(const Vector2& other) const {
        return (*this - other).magnitude();
    }

    Vector2 lerp(const Vector2& other, float t) const {
        return *this + (other - *this) * t;
    }

    // Статические методы
    static Vector2 zero() {
        return Vector2(0.0f, 0.0f);
    }

    static Vector2 one() {
        return Vector2(1.0f, 1.0f);
    }

    static Vector2 unitX() {
        return Vector2(1.0f, 0.0f);
    }

    static Vector2 unitY() {
        return Vector2(0.0f, 1.0f);
    }

    // Преобразование в строку для отладки
    std::string toString() const {
        return "Vector2(" + std::to_string(x) + ", " + std::to_string(y) + ")";
    }
};

// Внешние операторы
inline Vector2 operator*(float scalar, const Vector2& vec) {
    return vec * scalar;
}

} // namespace Math
} // namespace Engine3D
