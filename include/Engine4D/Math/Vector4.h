#pragma once

#include <cmath>
#include <iostream>

namespace Engine4D {
namespace Math {

/**
 * @brief 4-мерный вектор для работы с гиперпространством
 * 
 * Представляет точку или направление в 4D пространстве с координатами [x, y, z, w]
 * где w - четвертое измерение (гиперпространство)
 */
class Vector4 {
public:
    float x, y, z, w;

    // Конструкторы
    Vector4() : x(0.0f), y(0.0f), z(0.0f), w(0.0f) {}
    Vector4(float x, float y, float z, float w) : x(x), y(y), z(z), w(w) {}
    Vector4(const Vector4& other) : x(other.x), y(other.y), z(other.z), w(other.w) {}

    // Операторы
    Vector4& operator=(const Vector4& other);
    Vector4 operator+(const Vector4& other) const;
    Vector4 operator-(const Vector4& other) const;
    Vector4 operator*(float scalar) const;
    Vector4 operator/(float scalar) const;
    Vector4& operator+=(const Vector4& other);
    Vector4& operator-=(const Vector4& other);
    Vector4& operator*=(float scalar);
    Vector4& operator/=(float scalar);
    bool operator==(const Vector4& other) const;
    bool operator!=(const Vector4& other) const;

    // Математические операции
    float dot(const Vector4& other) const;           // Скалярное произведение
    Vector4 cross(const Vector4& other) const;       // Векторное произведение (4D)
    float magnitude() const;                         // Длина вектора
    float magnitudeSquared() const;                  // Квадрат длины вектора
    Vector4 normalized() const;                      // Нормализованный вектор
    void normalize();                                // Нормализация на месте
    float distance(const Vector4& other) const;      // Расстояние до другого вектора
    float distanceSquared(const Vector4& other) const; // Квадрат расстояния

    // Специальные операции для 4D
    Vector4 projectTo3D() const;                     // Проекция в 3D (отбрасывание w)
    Vector4 perspectiveProject(float distance) const; // Перспективная проекция
    Vector4 crossSection(float wValue) const;        // Сечение при заданном w

    // Утилиты
    void set(float x, float y, float z, float w);
    void setZero();
    bool isZero() const;
    Vector4 lerp(const Vector4& other, float t) const; // Линейная интерполяция

    // Статические методы
    static Vector4 zero();
    static Vector4 one();
    static Vector4 unitX();
    static Vector4 unitY();
    static Vector4 unitZ();
    static Vector4 unitW();

    // Операторы ввода/вывода
    friend std::ostream& operator<<(std::ostream& os, const Vector4& vec);
    friend std::istream& operator>>(std::istream& is, Vector4& vec);
};

// Операторы для скалярного умножения
Vector4 operator*(float scalar, const Vector4& vec);

} // namespace Math
} // namespace Engine4D
