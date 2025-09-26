#include "Engine4D/Math/Vector4.h"
#include <algorithm>
#include <limits>

namespace Engine4D {
namespace Math {

// Операторы присваивания и арифметики
Vector4& Vector4::operator=(const Vector4& other) {
    if (this != &other) {
        x = other.x;
        y = other.y;
        z = other.z;
        w = other.w;
    }
    return *this;
}

Vector4 Vector4::operator+(const Vector4& other) const {
    return Vector4(x + other.x, y + other.y, z + other.z, w + other.w);
}

Vector4 Vector4::operator-(const Vector4& other) const {
    return Vector4(x - other.x, y - other.y, z - other.z, w - other.w);
}

Vector4 Vector4::operator*(float scalar) const {
    return Vector4(x * scalar, y * scalar, z * scalar, w * scalar);
}

Vector4 Vector4::operator/(float scalar) const {
    if (std::abs(scalar) < std::numeric_limits<float>::epsilon()) {
        throw std::invalid_argument("Деление на ноль");
    }
    return Vector4(x / scalar, y / scalar, z / scalar, w / scalar);
}

Vector4& Vector4::operator+=(const Vector4& other) {
    x += other.x;
    y += other.y;
    z += other.z;
    w += other.w;
    return *this;
}

Vector4& Vector4::operator-=(const Vector4& other) {
    x -= other.x;
    y -= other.y;
    z -= other.z;
    w -= other.w;
    return *this;
}

Vector4& Vector4::operator*=(float scalar) {
    x *= scalar;
    y *= scalar;
    z *= scalar;
    w *= scalar;
    return *this;
}

Vector4& Vector4::operator/=(float scalar) {
    if (std::abs(scalar) < std::numeric_limits<float>::epsilon()) {
        throw std::invalid_argument("Деление на ноль");
    }
    x /= scalar;
    y /= scalar;
    z /= scalar;
    w /= scalar;
    return *this;
}

bool Vector4::operator==(const Vector4& other) const {
    const float epsilon = std::numeric_limits<float>::epsilon();
    return std::abs(x - other.x) < epsilon &&
           std::abs(y - other.y) < epsilon &&
           std::abs(z - other.z) < epsilon &&
           std::abs(w - other.w) < epsilon;
}

bool Vector4::operator!=(const Vector4& other) const {
    return !(*this == other);
}

// Математические операции
float Vector4::dot(const Vector4& other) const {
    return x * other.x + y * other.y + z * other.z + w * other.w;
}

Vector4 Vector4::cross(const Vector4& other) const {
    // Векторное произведение в 4D пространстве
    // Используем формулу для 4D cross product
    return Vector4(
        y * other.z - z * other.y + w * other.w,
        z * other.x - x * other.z + w * other.w,
        x * other.y - y * other.x + w * other.w,
        x * other.w - w * other.x + y * other.z - z * other.y
    );
}

float Vector4::magnitude() const {
    return std::sqrt(x * x + y * y + z * z + w * w);
}

float Vector4::magnitudeSquared() const {
    return x * x + y * y + z * z + w * w;
}

Vector4 Vector4::normalized() const {
    float mag = magnitude();
    if (mag < std::numeric_limits<float>::epsilon()) {
        return Vector4::zero();
    }
    return *this / mag;
}

void Vector4::normalize() {
    float mag = magnitude();
    if (mag >= std::numeric_limits<float>::epsilon()) {
        *this /= mag;
    } else {
        zero();
    }
}

float Vector4::distance(const Vector4& other) const {
    return (*this - other).magnitude();
}

float Vector4::distanceSquared(const Vector4& other) const {
    return (*this - other).magnitudeSquared();
}

// Специальные операции для 4D
Vector4 Vector4::projectTo3D() const {
    // Ортогональная проекция - просто отбрасываем w-координату
    return Vector4(x, y, z, 0.0f);
}

Vector4 Vector4::perspectiveProject(float distance) const {
    // Перспективная проекция из 4D в 3D
    // Формула: V' = [x*d/(d-w), y*d/(d-w), z*d/(d-w)]
    if (std::abs(distance - w) < std::numeric_limits<float>::epsilon()) {
        // Избегаем деления на ноль
        return Vector4::zero();
    }
    
    float factor = distance / (distance - w);
    return Vector4(x * factor, y * factor, z * factor, 0.0f);
}

Vector4 Vector4::crossSection(float wValue) const {
    // Находим пересечение с плоскостью w = wValue
    // Если текущая точка уже на плоскости, возвращаем её
    if (std::abs(w - wValue) < std::numeric_limits<float>::epsilon()) {
        return *this;
    }
    
    // Для сечений нужен второй вектор для определения линии
    // Здесь возвращаем проекцию на плоскость w = wValue
    return Vector4(x, y, z, wValue);
}

// Утилиты
void Vector4::set(float x, float y, float z, float w) {
    this->x = x;
    this->y = y;
    this->z = z;
    this->w = w;
}

void Vector4::zero() {
    x = y = z = w = 0.0f;
}

bool Vector4::isZero() const {
    const float epsilon = std::numeric_limits<float>::epsilon();
    return std::abs(x) < epsilon && std::abs(y) < epsilon && 
           std::abs(z) < epsilon && std::abs(w) < epsilon;
}

Vector4 Vector4::lerp(const Vector4& other, float t) const {
    t = std::clamp(t, 0.0f, 1.0f);
    return *this + (other - *this) * t;
}

// Статические методы
Vector4 Vector4::zero() {
    return Vector4(0.0f, 0.0f, 0.0f, 0.0f);
}

Vector4 Vector4::one() {
    return Vector4(1.0f, 1.0f, 1.0f, 1.0f);
}

Vector4 Vector4::unitX() {
    return Vector4(1.0f, 0.0f, 0.0f, 0.0f);
}

Vector4 Vector4::unitY() {
    return Vector4(0.0f, 1.0f, 0.0f, 0.0f);
}

Vector4 Vector4::unitZ() {
    return Vector4(0.0f, 0.0f, 1.0f, 0.0f);
}

Vector4 Vector4::unitW() {
    return Vector4(0.0f, 0.0f, 0.0f, 1.0f);
}

// Операторы ввода/вывода
std::ostream& operator<<(std::ostream& os, const Vector4& vec) {
    os << "[" << vec.x << ", " << vec.y << ", " << vec.z << ", " << vec.w << "]";
    return os;
}

std::istream& operator>>(std::istream& is, Vector4& vec) {
    char ch;
    is >> ch; // '['
    is >> vec.x >> ch; // x и ','
    is >> vec.y >> ch; // y и ','
    is >> vec.z >> ch; // z и ','
    is >> vec.w >> ch; // w и ']'
    return is;
}

// Оператор для скалярного умножения слева
Vector4 operator*(float scalar, const Vector4& vec) {
    return vec * scalar;
}

} // namespace Math
} // namespace Engine4D
