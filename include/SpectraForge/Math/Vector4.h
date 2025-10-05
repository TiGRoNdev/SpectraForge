
#pragma once

#include <cmath>
#include <iostream>

// Forward declaration
namespace SpectraForge {
namespace Math {
class Vector3;
}
}  // namespace SpectraForge

namespace SpectraForge {
namespace Math {

/**
 * @brief 4-мерный вектор для работы с 4D пространством
 *
 * Представляет точку или направление в 4D пространстве с координатами [x, y, z, w]
 * Используется для 4D трансформаций, гомогенных координат и 4D математических операций
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
    Vector4 operator-() const;  // Унарный минус
    Vector4 operator*(float scalar) const;
    Vector4 operator/(float scalar) const;
    Vector4& operator+=(const Vector4& other);
    Vector4& operator-=(const Vector4& other);
    Vector4& operator*=(float scalar);
    Vector4& operator/=(float scalar);
    bool operator==(const Vector4& other) const;
    bool operator!=(const Vector4& other) const;
    float operator[](int index) const;
    float& operator[](int index);

    // Математические операции
    float dot(const Vector4& other) const;  // Скалярное произведение
    float magnitude() const;                // Длина вектора
    float magnitudeSquared() const;         // Квадрат длины вектора
    Vector4 normalized() const;             // Нормализованный вектор
    void normalize();                       // Нормализация на месте
    float distance(const Vector4& other) const;  // Расстояние до другого вектора
    float distanceSquared(const Vector4& other) const;  // Квадрат расстояния

    // Утилиты
    void set(float x, float y, float z, float w);
    void zero();
    bool isZero() const;
    bool isNormalized() const;

    // Преобразования
    Vector3 xyz() const;  // Возвращает первые 3 компоненты как Vector3
    Vector3 homogeneousDivide() const;  // Деление на w (для гомогенных координат)

    // Статические методы
    static Vector4 createZero();
    static Vector4 createOne();
    static Vector4 createUnitX();
    static Vector4 createUnitY();
    static Vector4 createUnitZ();
    static Vector4 createUnitW();
    static Vector4 lerp(const Vector4& a, const Vector4& b, float t);

    // Операторы для дружественных функций
    friend Vector4 operator*(float scalar, const Vector4& v);
    friend std::ostream& operator<<(std::ostream& os, const Vector4& v);
};

// Внешние операторы
Vector4 operator*(float scalar, const Vector4& v);
std::ostream& operator<<(std::ostream& os, const Vector4& v);

}  // namespace Math
}  // namespace SpectraForge
