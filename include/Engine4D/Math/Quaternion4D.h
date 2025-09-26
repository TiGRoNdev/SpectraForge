#pragma once

#include "Vector4.h"
#include "Matrix4.h"

namespace Engine4D {
namespace Math {

/**
 * @brief 4D кватернион для представления поворотов в гиперпространстве
 * 
 * Расширяет концепцию 3D кватернионов для работы с 4D поворотами.
 * Использует 6 параметров для поворотов в 6 плоскостях 4D пространства.
 */
class Quaternion4D {
public:
    float w, x, y, z;  // w - скалярная часть, x,y,z - векторная часть
    float u, v;        // Дополнительные параметры для 4D поворотов

    // Конструкторы
    Quaternion4D();
    Quaternion4D(float w, float x, float y, float z, float u = 0.0f, float v = 0.0f);
    Quaternion4D(const Quaternion4D& other);
    explicit Quaternion4D(const Vector4& axis, float angle);

    // Операторы
    Quaternion4D& operator=(const Quaternion4D& other);
    Quaternion4D operator+(const Quaternion4D& other) const;
    Quaternion4D operator-(const Quaternion4D& other) const;
    Quaternion4D operator*(const Quaternion4D& other) const;
    Quaternion4D operator*(float scalar) const;
    Quaternion4D operator/(float scalar) const;
    Quaternion4D& operator+=(const Quaternion4D& other);
    Quaternion4D& operator-=(const Quaternion4D& other);
    Quaternion4D& operator*=(const Quaternion4D& other);
    Quaternion4D& operator*=(float scalar);
    Quaternion4D& operator/=(float scalar);
    bool operator==(const Quaternion4D& other) const;
    bool operator!=(const Quaternion4D& other) const;

    // Основные операции
    float magnitude() const;                         // Длина кватерниона
    float magnitudeSquared() const;                  // Квадрат длины
    Quaternion4D normalized() const;                 // Нормализованный кватернион
    void normalize();                                // Нормализация на месте
    Quaternion4D conjugate() const;                  // Сопряженный кватернион
    Quaternion4D inverse() const;                    // Обратный кватернион
    float dot(const Quaternion4D& other) const;      // Скалярное произведение

    // Повороты
    Vector4 rotate(const Vector4& vec) const;        // Поворот вектора
    Matrix4 toMatrix() const;                        // Преобразование в матрицу
    static Quaternion4D fromMatrix(const Matrix4& mat); // Создание из матрицы

    // Создание поворотов в различных плоскостях
    static Quaternion4D rotationXY(float angle);
    static Quaternion4D rotationXZ(float angle);
    static Quaternion4D rotationXW(float angle);
    static Quaternion4D rotationYZ(float angle);
    static Quaternion4D rotationYW(float angle);
    static Quaternion4D rotationZW(float angle);

    // Интерполяция
    Quaternion4D slerp(const Quaternion4D& other, float t) const; // Сферическая интерполяция
    Quaternion4D lerp(const Quaternion4D& other, float t) const;  // Линейная интерполяция

    // Утилиты
    void setIdentity();
    bool isIdentity() const;
    bool isZero() const;
    void set(float w, float x, float y, float z, float u = 0.0f, float v = 0.0f);

    // Статические константы
    static Quaternion4D identity();
    static Quaternion4D zero();

    // Операторы ввода/вывода
    friend std::ostream& operator<<(std::ostream& os, const Quaternion4D& q);
    friend std::istream& operator>>(std::istream& is, Quaternion4D& q);
};

// Оператор для скалярного умножения слева
Quaternion4D operator*(float scalar, const Quaternion4D& q);

} // namespace Math
} // namespace Engine4D
