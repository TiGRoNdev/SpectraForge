#pragma once

#include <cmath>
#include <iostream>

namespace SpectraForge {
namespace Math {

/**
 * @brief 3-мерный вектор для работы с 3D пространством
 *
 * Представляет точку или направление в 3D пространстве с координатами [x, y, z]
 */
class Vector3 {
  public:
    float x, y, z;

    // Конструкторы
    Vector3() : x(0.0f), y(0.0f), z(0.0f) {}
    Vector3(float x, float y, float z) : x(x), y(y), z(z) {}
    Vector3(const Vector3& other) : x(other.x), y(other.y), z(other.z) {}

    // Операторы
    Vector3& operator=(const Vector3& other);
    Vector3 operator+(const Vector3& other) const;
    Vector3 operator-(const Vector3& other) const;
    Vector3 operator-() const;  // Унарный минус
    Vector3 operator*(float scalar) const;
    Vector3 operator/(float scalar) const;
    Vector3& operator+=(const Vector3& other);
    Vector3& operator-=(const Vector3& other);
    Vector3& operator*=(float scalar);
    Vector3& operator/=(float scalar);
    bool operator==(const Vector3& other) const;
    bool operator!=(const Vector3& other) const;
    float operator[](int index) const;
    float& operator[](int index);

    // Математические операции
    float dot(const Vector3& other) const;      // Скалярное произведение
    Vector3 cross(const Vector3& other) const;  // Векторное произведение
    float magnitude() const;                    // Длина вектора
    float magnitudeSquared() const;             // Квадрат длины вектора
    Vector3 normalized() const;                 // Нормализованный вектор
    void normalize();                           // Нормализация на месте
    float distance(const Vector3& other) const;  // Расстояние до другого вектора
    float distanceSquared(const Vector3& other) const;  // Квадрат расстояния

    // Утилиты
    void set(float x, float y, float z);
    void setZero();
    bool isZero() const;
    Vector3 lerp(const Vector3& other, float t) const;  // Линейная интерполяция
    Vector3 reflect(const Vector3& normal) const;       // Отражение от нормали
    Vector3 project(const Vector3& other) const;  // Проекция на другой вектор
    float angle(const Vector3& other) const;      // Угол между векторами

    // Статические методы
    // Удобные перегрузки в стиле glm: статические реализации
    static float dot(const Vector3& a, const Vector3& b);
    static Vector3 cross(const Vector3& a, const Vector3& b);
    static Vector3 zero();
    static Vector3 one();
    static Vector3 unitX();
    static Vector3 unitY();
    static Vector3 unitZ();
    static Vector3 forward();  // В направлении -Z
    static Vector3 back();     // В направлении +Z
    static Vector3 left();     // В направлении -X
    static Vector3 right();    // В направлении +X
    static Vector3 up();       // В направлении +Y
    static Vector3 down();     // В направлении -Y

    // Операторы ввода/вывода
    friend std::ostream& operator<<(std::ostream& os, const Vector3& vec);
    friend std::istream& operator>>(std::istream& is, Vector3& vec);
};

// Операторы для скалярного умножения
Vector3 operator*(float scalar, const Vector3& vec);

}  // namespace Math
}  // namespace SpectraForge
