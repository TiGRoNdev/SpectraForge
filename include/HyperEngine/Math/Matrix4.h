#pragma once

#include <array>
#include "Vector3.h"

namespace HyperEngine {
namespace Math {

// Forward declaration
class Quaternion;

/**
 * @brief 4x4 матрица для трансформаций в 3D пространстве
 *
 * Поддерживает повороты, масштабирование, перемещение и проекции в 3D.
 * Использует гомогенные координаты для представления трансформаций.
 */
class Matrix4 {
  public:
    // Элементы матрицы в row-major порядке
    std::array<std::array<float, 4>, 4> m;

    // Конструкторы
    Matrix4();
    Matrix4(const Matrix4& other);
    explicit Matrix4(float diagonal);
    explicit Matrix4(const std::array<std::array<float, 4>, 4>& data);

    // Операторы
    Matrix4& operator=(const Matrix4& other);
    Matrix4 operator+(const Matrix4& other) const;
    Matrix4 operator-(const Matrix4& other) const;
    Matrix4 operator*(const Matrix4& other) const;
    Matrix4 operator*(float scalar) const;
    Vector3 operator*(const Vector3& vec) const;  // Умножение на 3D вектор
    Matrix4& operator+=(const Matrix4& other);
    Matrix4& operator-=(const Matrix4& other);
    Matrix4& operator*=(const Matrix4& other);
    Matrix4& operator*=(float scalar);
    bool operator==(const Matrix4& other) const;
    bool operator!=(const Matrix4& other) const;
    float& operator()(int row, int col);
    const float& operator()(int row, int col) const;

    // Основные операции
    Matrix4 transpose() const;  // Транспонирование
    float determinant() const;  // Определитель
    Matrix4 inverse() const;    // Обратная матрица
    Matrix4 adjugate() const;   // Присоединенная матрица
    bool isInvertible() const;  // Проверка обратимости
    void setIdentity();         // Установка единичной матрицы
    void setZero();             // Установка нулевой матрицы

    // Трансформации
    static Matrix4 translation(const Vector3& translation);
    static Matrix4 translation(float x, float y, float z);
    static Matrix4 scaling(const Vector3& scale);
    static Matrix4 scaling(float x, float y, float z);
    static Matrix4 scaling(float uniformScale);

    // Повороты
    static Matrix4 rotationX(float angle);                       // Поворот вокруг оси X
    static Matrix4 rotationY(float angle);                       // Поворот вокруг оси Y
    static Matrix4 rotationZ(float angle);                       // Поворот вокруг оси Z
    static Matrix4 rotation(const Vector3& axis, float angle);   // Поворот вокруг произвольной оси
    static Matrix4 rotationFromQuaternion(const Quaternion& q);  // Из кватерниона
    static Matrix4 eulerAngles(float pitch, float yaw, float roll);  // Углы Эйлера

    // Камера и проекции
    static Matrix4 lookAt(const Vector3& eye, const Vector3& target, const Vector3& up);
    static Matrix4 perspective(float fovY, float aspect, float near, float far);
    static Matrix4 orthographic(float left,
                                float right,
                                float bottom,
                                float top,
                                float near,
                                float far);

    // Утилиты
    Vector3 transformPoint(const Vector3& point) const;          // Трансформация точки
    Vector3 transformDirection(const Vector3& direction) const;  // Трансформация направления
    Vector3 transformVector(
        const Vector3& vector) const;  // Трансформация вектора (алиас для transformPoint)
    Vector3 getTranslation() const;    // Извлечение трансляции
    Vector3 getScale() const;          // Извлечение масштаба
    void decompose(Vector3& translation, Quaternion& rotation, Vector3& scale) const;

    // Доступ к данным для OpenGL
    const float* data() const { return &m[0][0]; }
    float* data() { return &m[0][0]; }

    // Статические константы
    static Matrix4 identity();
    static Matrix4 zero();
};

// Операторы ввода/вывода
std::ostream& operator<<(std::ostream& os, const Matrix4& mat);
std::istream& operator>>(std::istream& is, Matrix4& mat);

// Оператор для скалярного умножения слева
Matrix4 operator*(float scalar, const Matrix4& mat);

}  // namespace Math
}  // namespace HyperEngine
