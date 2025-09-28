#pragma once

#include "Vector3.h"

namespace HyperEngine {
namespace Math {

// Forward declaration
class Matrix4;

/**
 * @brief Кватернион для представления поворотов в 3D пространстве
 *
 * Кватернионы обеспечивают эффективное и стабильное представление поворотов
 * без проблем с gimbal lock, характерных для углов Эйлера.
 */
class Quaternion {
  public:
    float w, x, y, z;  // w - скалярная часть, x,y,z - векторная часть

    // Конструкторы
    Quaternion();
    Quaternion(float w, float x, float y, float z);
    Quaternion(const Quaternion& other);
    explicit Quaternion(const Vector3& axis, float angle);  // Из оси и угла
    explicit Quaternion(const Vector3& eulerAngles);  // Из углов Эйлера (pitch, yaw, roll)

    // Операторы
    Quaternion& operator=(const Quaternion& other);
    Quaternion operator+(const Quaternion& other) const;
    Quaternion operator-(const Quaternion& other) const;
    Quaternion operator*(const Quaternion& other) const;  // Композиция поворотов
    Quaternion operator*(float scalar) const;
    Quaternion operator/(float scalar) const;
    Quaternion& operator+=(const Quaternion& other);
    Quaternion& operator-=(const Quaternion& other);
    Quaternion& operator*=(const Quaternion& other);
    Quaternion& operator*=(float scalar);
    Quaternion& operator/=(float scalar);
    bool operator==(const Quaternion& other) const;
    bool operator!=(const Quaternion& other) const;

    // Основные операции
    float magnitude() const;         // Длина кватерниона
    float magnitudeSquared() const;  // Квадрат длины
    Quaternion normalized() const;   // Нормализованный кватернион
    void normalize();                // Нормализация на месте
    Quaternion conjugate() const;    // Сопряженный кватернион
    Quaternion inverse() const;      // Обратный кватернион
    float dot(const Quaternion& other) const;  // Скалярное произведение

    // Повороты
    Vector3 rotate(const Vector3& vec) const;  // Поворот вектора
    Matrix4 toMatrix() const;                  // Преобразование в матрицу
    Vector3 toEulerAngles() const;  // Преобразование в углы Эйлера
    void toAxisAngle(Vector3& axis, float& angle) const;  // Преобразование в ось-угол

    // Создание поворотов
    static Quaternion fromAxisAngle(const Vector3& axis, float angle);
    static Quaternion fromEulerAngles(float pitch, float yaw, float roll);
    static Quaternion fromEulerAngles(const Vector3& eulerAngles);
    static Quaternion fromMatrix(const Matrix4& mat);
    static Quaternion lookRotation(const Vector3& forward, const Vector3& up = Vector3::up());

    // Интерполяция
    Quaternion slerp(const Quaternion& other, float t) const;  // Сферическая интерполяция
    Quaternion lerp(const Quaternion& other, float t) const;  // Линейная интерполяция
    Quaternion nlerp(const Quaternion& other,
                     float t) const;  // Нормализованная линейная интерполяция

    // Утилиты
    void setIdentity();
    bool isIdentity() const;
    bool isZero() const;
    void set(float w, float x, float y, float z);
    float angle() const;   // Угол поворота
    Vector3 axis() const;  // Ось поворота

    // Статические константы
    static Quaternion identity();
    static Quaternion zero();

    // Операторы ввода/вывода
    friend std::ostream& operator<<(std::ostream& os, const Quaternion& q);
    friend std::istream& operator>>(std::istream& is, Quaternion& q);
};

// Оператор для скалярного умножения слева
Quaternion operator*(float scalar, const Quaternion& q);

}  // namespace Math
}  // namespace HyperEngine
