#pragma once

#include "Vector4.h"
#include <array>

namespace Engine4D {
namespace Math {

/**
 * @brief 4x4 матрица для трансформаций в 4D пространстве
 * 
 * Поддерживает повороты в 6 плоскостях (XY, XZ, XW, YZ, YW, ZW),
 * масштабирование, перемещение и проекции
 */
class Matrix4 {
public:
    // Элементы матрицы в row-major порядке
    std::array<std::array<float, 4>, 4> m;

    // Конструкторы
    Matrix4();
    Matrix4(const Matrix4& other);
    explicit Matrix4(float diagonal);
    Matrix4(const std::array<std::array<float, 4>, 4>& data);

    // Операторы
    Matrix4& operator=(const Matrix4& other);
    Matrix4 operator+(const Matrix4& other) const;
    Matrix4 operator-(const Matrix4& other) const;
    Matrix4 operator*(const Matrix4& other) const;
    Matrix4 operator*(float scalar) const;
    Vector4 operator*(const Vector4& vec) const;
    Matrix4& operator+=(const Matrix4& other);
    Matrix4& operator-=(const Matrix4& other);
    Matrix4& operator*=(const Matrix4& other);
    Matrix4& operator*=(float scalar);
    bool operator==(const Matrix4& other) const;
    bool operator!=(const Matrix4& other) const;
    float& operator()(int row, int col);
    const float& operator()(int row, int col) const;

    // Основные операции
    Matrix4 transpose() const;                       // Транспонирование
    float determinant() const;                       // Определитель
    Matrix4 inverse() const;                         // Обратная матрица
    Matrix4 adjugate() const;                        // Присоединенная матрица
    bool isInvertible() const;                       // Проверка обратимости
    void setIdentity();                              // Установка единичной матрицы
    void setZero();                                  // Установка нулевой матрицы

    // Трансформации
    static Matrix4 translation(const Vector4& translation);
    static Matrix4 translation(float x, float y, float z, float w);
    static Matrix4 scaling(const Vector4& scale);
    static Matrix4 scaling(float x, float y, float z, float w);
    static Matrix4 scaling(float uniformScale);

    // Повороты в 4D (6 плоскостей)
    static Matrix4 rotationXY(float angle);          // Поворот в плоскости XY
    static Matrix4 rotationXZ(float angle);          // Поворот в плоскости XZ
    static Matrix4 rotationXW(float angle);          // Поворот в плоскости XW
    static Matrix4 rotationYZ(float angle);          // Поворот в плоскости YZ
    static Matrix4 rotationYW(float angle);          // Поворот в плоскости YW
    static Matrix4 rotationZW(float angle);          // Поворот в плоскости ZW

    // Комбинированные повороты
    static Matrix4 rotation(const Vector4& axis, float angle);
    static Matrix4 eulerAngles(float x, float y, float z, float w);
    
    // Дополнительные методы для совместимости
    static Matrix4 lookAt(const Vector4& eye, const Vector4& target, const Vector4& up);
    static Matrix4 perspective(float fov, float aspect, float near, float far);

    // Проекции
    static Matrix4 orthographicProjection();         // Ортогональная проекция 4D->3D
    static Matrix4 perspectiveProjection(float distance); // Перспективная проекция
    static Matrix4 crossSectionProjection(float wValue);  // Проекция сечения

    // Утилиты
    Vector4 transformPoint(const Vector4& point) const;
    Vector4 transformDirection(const Vector4& direction) const;
    void decompose(Vector4& translation, Vector4& rotation, Vector4& scale) const;

    // Статические константы
    static Matrix4 identity();
    static Matrix4 zero();

    // Операторы ввода/вывода
    friend std::ostream& operator<<(std::ostream& os, const Matrix4& mat);
    friend std::istream& operator>>(std::istream& is, Matrix4& mat);
};

// Оператор для скалярного умножения слева
Matrix4 operator*(float scalar, const Matrix4& mat);

} // namespace Math
} // namespace Engine4D
