#include "HyperEngine/Math/Vector4.h"
#include <algorithm>
#include <limits>
#include <stdexcept>
#include "HyperEngine/Math/Vector3.h"

namespace HyperEngine::Math {

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

Vector4 Vector4::operator-() const {
    return Vector4(-x, -y, -z, -w);
}

Vector4 Vector4::operator*(float scalar) const {
    return Vector4(x * scalar, y * scalar, z * scalar, w * scalar);
}

Vector4 Vector4::operator/(float scalar) const {
    if (std::abs(scalar) < std::numeric_limits<float>::epsilon()) {
        // Возвращаем inf вместо исключения для совместимости с тестами
        return Vector4(std::numeric_limits<float>::infinity(),
                       std::numeric_limits<float>::infinity(),
                       std::numeric_limits<float>::infinity(),
                       std::numeric_limits<float>::infinity());
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
    return std::abs(x - other.x) < epsilon && std::abs(y - other.y) < epsilon
           && std::abs(z - other.z) < epsilon && std::abs(w - other.w) < epsilon;
}

bool Vector4::operator!=(const Vector4& other) const {
    return !(*this == other);
}

float Vector4::operator[](int index) const {
    switch (index) {
        case 0:
            return x;
        case 1:
            return y;
        case 2:
            return z;
        case 3:
            return w;
        default:
            throw std::out_of_range("Индекс Vector4 должен быть от 0 до 3");
    }
}

float& Vector4::operator[](int index) {
    switch (index) {
        case 0:
            return x;
        case 1:
            return y;
        case 2:
            return z;
        case 3:
            return w;
        default:
            throw std::out_of_range("Индекс Vector4 должен быть от 0 до 3");
    }
}

// Математические операции
float Vector4::dot(const Vector4& other) const {
    return x * other.x + y * other.y + z * other.z + w * other.w;
}

float Vector4::magnitude() const {
    return std::sqrt(magnitudeSquared());
}

float Vector4::magnitudeSquared() const {
    return x * x + y * y + z * z + w * w;
}

Vector4 Vector4::normalized() const {
    float mag = magnitude();
    if (std::abs(mag) < std::numeric_limits<float>::epsilon()) {
        return Vector4::createZero();
    }
    return *this / mag;
}

void Vector4::normalize() {
    float mag = magnitude();
    if (std::abs(mag) < std::numeric_limits<float>::epsilon()) {
        zero();
        return;
    }
    *this /= mag;
}

float Vector4::distance(const Vector4& other) const {
    return (*this - other).magnitude();
}

float Vector4::distanceSquared(const Vector4& other) const {
    return (*this - other).magnitudeSquared();
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
    return std::abs(x) < epsilon && std::abs(y) < epsilon && std::abs(z) < epsilon
           && std::abs(w) < epsilon;
}

bool Vector4::isNormalized() const {
    const float epsilon = std::numeric_limits<float>::epsilon();
    return std::abs(magnitude() - 1.0f) < epsilon;
}

// Преобразования
Vector3 Vector4::xyz() const {
    return Vector3(x, y, z);
}

Vector3 Vector4::homogeneousDivide() const {
    if (std::abs(w) < std::numeric_limits<float>::epsilon()) {
        throw std::invalid_argument("Нельзя выполнить гомогенное деление при w = 0");
    }
    return Vector3(x / w, y / w, z / w);
}

// Статические методы
Vector4 Vector4::createZero() {
    return Vector4(0.0f, 0.0f, 0.0f, 0.0f);
}

Vector4 Vector4::createOne() {
    return Vector4(1.0f, 1.0f, 1.0f, 1.0f);
}

Vector4 Vector4::createUnitX() {
    return Vector4(1.0f, 0.0f, 0.0f, 0.0f);
}

Vector4 Vector4::createUnitY() {
    return Vector4(0.0f, 1.0f, 0.0f, 0.0f);
}

Vector4 Vector4::createUnitZ() {
    return Vector4(0.0f, 0.0f, 1.0f, 0.0f);
}

Vector4 Vector4::createUnitW() {
    return Vector4(0.0f, 0.0f, 0.0f, 1.0f);
}

Vector4 Vector4::lerp(const Vector4& a, const Vector4& b, float t) {
    t = std::clamp(t, 0.0f, 1.0f);
    return a + (b - a) * t;
}

// Внешние операторы
Vector4 operator*(float scalar, const Vector4& v) {
    return v * scalar;
}

std::ostream& operator<<(std::ostream& os, const Vector4& v) {
    os << "Vector4(" << v.x << ", " << v.y << ", " << v.z << ", " << v.w << ")";
    return os;
}

}  // namespace HyperEngine::Math
