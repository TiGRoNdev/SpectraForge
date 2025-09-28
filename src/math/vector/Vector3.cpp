#include "HyperEngine/Math/Vector3.h"
#include <algorithm>
#include <limits>
#include <stdexcept>

using namespace HyperEngine::Math;

// Операторы присваивания и арифметики
Vector3& Vector3::operator=(const Vector3& other) {
    if (this != &other) {
        x = other.x;
        y = other.y;
        z = other.z;
    }
    return *this;
}

Vector3 Vector3::operator+(const Vector3& other) const {
    return Vector3(x + other.x, y + other.y, z + other.z);
}

Vector3 Vector3::operator-(const Vector3& other) const {
    return Vector3(x - other.x, y - other.y, z - other.z);
}

Vector3 Vector3::operator-() const {
    return Vector3(-x, -y, -z);
}

Vector3 Vector3::operator*(float scalar) const {
    return Vector3(x * scalar, y * scalar, z * scalar);
}

Vector3 Vector3::operator/(float scalar) const {
    if (std::abs(scalar) < std::numeric_limits<float>::epsilon()) {
        throw std::invalid_argument("Деление на ноль");
    }
    return Vector3(x / scalar, y / scalar, z / scalar);
}

Vector3& Vector3::operator+=(const Vector3& other) {
    x += other.x;
    y += other.y;
    z += other.z;
    return *this;
}

Vector3& Vector3::operator-=(const Vector3& other) {
    x -= other.x;
    y -= other.y;
    z -= other.z;
    return *this;
}

Vector3& Vector3::operator*=(float scalar) {
    x *= scalar;
    y *= scalar;
    z *= scalar;
    return *this;
}

Vector3& Vector3::operator/=(float scalar) {
    if (std::abs(scalar) < std::numeric_limits<float>::epsilon()) {
        throw std::invalid_argument("Деление на ноль");
    }
    x /= scalar;
    y /= scalar;
    z /= scalar;
    return *this;
}

bool Vector3::operator==(const Vector3& other) const {
    const float epsilon = std::numeric_limits<float>::epsilon();
    return std::abs(x - other.x) < epsilon &&
           std::abs(y - other.y) < epsilon &&
           std::abs(z - other.z) < epsilon;
}

bool Vector3::operator!=(const Vector3& other) const {
    return !(*this == other);
}

float Vector3::operator[](int index) const {
    switch (index) {
        case 0: return x;
        case 1: return y;
        case 2: return z;
        default: throw std::out_of_range("Индекс Vector3 должен быть от 0 до 2");
    }
}

float& Vector3::operator[](int index) {
    switch (index) {
        case 0: return x;
        case 1: return y;
        case 2: return z;
        default: throw std::out_of_range("Индекс Vector3 должен быть от 0 до 2");
    }
}

// Математические операции
float Vector3::dot(const Vector3& other) const {
    return x * other.x + y * other.y + z * other.z;
}

Vector3 Vector3::cross(const Vector3& other) const {
    return Vector3(
        y * other.z - z * other.y,
        z * other.x - x * other.z,
        x * other.y - y * other.x
    );
}

float Vector3::magnitude() const {
    return std::sqrt(x * x + y * y + z * z);
}

float Vector3::magnitudeSquared() const {
    return x * x + y * y + z * z;
}

Vector3 Vector3::normalized() const {
    float mag = magnitude();
    if (mag < std::numeric_limits<float>::epsilon()) {
        return Vector3::zero();
    }
    return *this / mag;
}

void Vector3::normalize() {
    float mag = magnitude();
    if (mag >= std::numeric_limits<float>::epsilon()) {
        *this /= mag;
    } else {
        setZero();
    }
}

float Vector3::distance(const Vector3& other) const {
    return (*this - other).magnitude();
}

float Vector3::distanceSquared(const Vector3& other) const {
    return (*this - other).magnitudeSquared();
}

// Утилиты
void Vector3::set(float x, float y, float z) {
    this->x = x;
    this->y = y;
    this->z = z;
}

void Vector3::setZero() {
    x = y = z = 0.0f;
}

bool Vector3::isZero() const {
    const float epsilon = std::numeric_limits<float>::epsilon();
    return std::abs(x) < epsilon && std::abs(y) < epsilon && std::abs(z) < epsilon;
}

Vector3 Vector3::lerp(const Vector3& other, float t) const {
    t = std::clamp(t, 0.0f, 1.0f);
    return *this + (other - *this) * t;
}

Vector3 Vector3::reflect(const Vector3& normal) const {
    // Формула отражения: V' = V - 2 * (V · N) * N
    return *this - normal * (2.0f * this->dot(normal));
}

Vector3 Vector3::project(const Vector3& other) const {
    // Проекция вектора this на вектор other
    float dotProduct = this->dot(other);
    float otherMagnitudeSquared = other.magnitudeSquared();
    
    if (otherMagnitudeSquared < std::numeric_limits<float>::epsilon()) {
        return Vector3::zero();
    }
    
    return other * (dotProduct / otherMagnitudeSquared);
}

float Vector3::angle(const Vector3& other) const {
    float mag1 = magnitude();
    float mag2 = other.magnitude();
    
    if (mag1 < std::numeric_limits<float>::epsilon() || 
        mag2 < std::numeric_limits<float>::epsilon()) {
        return 0.0f;
    }
    
    float cosAngle = this->dot(other) / (mag1 * mag2);
    cosAngle = std::clamp(cosAngle, -1.0f, 1.0f);
    return std::acos(cosAngle);
}

// Статические методы
Vector3 Vector3::zero() {
    return Vector3(0.0f, 0.0f, 0.0f);
}

Vector3 Vector3::one() {
    return Vector3(1.0f, 1.0f, 1.0f);
}

Vector3 Vector3::unitX() {
    return Vector3(1.0f, 0.0f, 0.0f);
}

Vector3 Vector3::unitY() {
    return Vector3(0.0f, 1.0f, 0.0f);
}

Vector3 Vector3::unitZ() {
    return Vector3(0.0f, 0.0f, 1.0f);
}

Vector3 Vector3::forward() {
    return Vector3(0.0f, 0.0f, -1.0f);
}

Vector3 Vector3::back() {
    return Vector3(0.0f, 0.0f, 1.0f);
}

Vector3 Vector3::left() {
    return Vector3(-1.0f, 0.0f, 0.0f);
}

Vector3 Vector3::right() {
    return Vector3(1.0f, 0.0f, 0.0f);
}

Vector3 Vector3::up() {
    return Vector3(0.0f, 1.0f, 0.0f);
}

Vector3 Vector3::down() {
    return Vector3(0.0f, -1.0f, 0.0f);
}

// Операторы ввода/вывода
std::ostream& operator<<(std::ostream& os, const Vector3& vec) {
    os << "[" << vec.x << ", " << vec.y << ", " << vec.z << "]";
    return os;
}

std::istream& operator>>(std::istream& is, Vector3& vec) {
    char ch;
    is >> ch; // '['
    is >> vec.x >> ch; // x и ','
    is >> vec.y >> ch; // y и ','
    is >> vec.z >> ch; // z и ']'
    return is;
}

// Оператор для скалярного умножения слева
namespace HyperEngine::Math {
Vector3 operator*(float scalar, const Vector3& vec) {
    return vec * scalar;
}
} // namespace HyperEngine::Math
