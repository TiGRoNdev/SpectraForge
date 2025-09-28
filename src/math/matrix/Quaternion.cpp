#include "HyperEngine/Math/Quaternion.h"
#include "HyperEngine/Math/Matrix4.h"
#include "HyperEngine/Math/MathConstants.h"
#include <cmath>
#include <algorithm>

using namespace HyperEngine::Math;

namespace HyperEngine {
namespace Math {

// Конструкторы
Quaternion::Quaternion() : w(1.0f), x(0.0f), y(0.0f), z(0.0f) {}

Quaternion::Quaternion(float w, float x, float y, float z) : w(w), x(x), y(y), z(z) {}

Quaternion::Quaternion(const Quaternion& other) : w(other.w), x(other.x), y(other.y), z(other.z) {}

Quaternion::Quaternion(const Vector3& axis, float angle) {
    const float halfAngle = angle * 0.5f;
    const float s = std::sin(halfAngle);
    const Vector3 normalizedAxis = axis.normalized();
    
    w = std::cos(halfAngle);
    x = normalizedAxis.x * s;
    y = normalizedAxis.y * s;
    z = normalizedAxis.z * s;
}

Quaternion::Quaternion(const Vector3& eulerAngles) {
    const float cr = std::cos(eulerAngles.x * 0.5f); // roll
    const float sr = std::sin(eulerAngles.x * 0.5f);
    const float cp = std::cos(eulerAngles.y * 0.5f); // pitch
    const float sp = std::sin(eulerAngles.y * 0.5f);
    const float cy = std::cos(eulerAngles.z * 0.5f); // yaw
    const float sy = std::sin(eulerAngles.z * 0.5f);

    w = cr * cp * cy + sr * sp * sy;
    x = sr * cp * cy - cr * sp * sy;
    y = cr * sp * cy + sr * cp * sy;
    z = cr * cp * sy - sr * sp * cy;
}

// Операторы
Quaternion& Quaternion::operator=(const Quaternion& other) {
    if (this != &other) {
        w = other.w;
        x = other.x;
        y = other.y;
        z = other.z;
    }
    return *this;
}

Quaternion Quaternion::operator+(const Quaternion& other) const {
    return Quaternion(w + other.w, x + other.x, y + other.y, z + other.z);
}

Quaternion Quaternion::operator-(const Quaternion& other) const {
    return Quaternion(w - other.w, x - other.x, y - other.y, z - other.z);
}

Quaternion Quaternion::operator*(const Quaternion& other) const {
    return Quaternion(
        w * other.w - x * other.x - y * other.y - z * other.z,
        w * other.x + x * other.w + y * other.z - z * other.y,
        w * other.y - x * other.z + y * other.w + z * other.x,
        w * other.z + x * other.y - y * other.x + z * other.w
    );
}

Quaternion Quaternion::operator*(float scalar) const {
    return Quaternion(w * scalar, x * scalar, y * scalar, z * scalar);
}

Quaternion Quaternion::operator/(float scalar) const {
    return *this * (1.0f / scalar);
}

Quaternion& Quaternion::operator+=(const Quaternion& other) {
    w += other.w;
    x += other.x;
    y += other.y;
    z += other.z;
    return *this;
}

Quaternion& Quaternion::operator-=(const Quaternion& other) {
    w -= other.w;
    x -= other.x;
    y -= other.y;
    z -= other.z;
    return *this;
}

Quaternion& Quaternion::operator*=(const Quaternion& other) {
    *this = *this * other;
    return *this;
}

Quaternion& Quaternion::operator*=(float scalar) {
    w *= scalar;
    x *= scalar;
    y *= scalar;
    z *= scalar;
    return *this;
}

Quaternion& Quaternion::operator/=(float scalar) {
    return *this *= (1.0f / scalar);
}

bool Quaternion::operator==(const Quaternion& other) const {
    const float epsilon = 1e-6f;
    return std::abs(w - other.w) < epsilon &&
           std::abs(x - other.x) < epsilon &&
           std::abs(y - other.y) < epsilon &&
           std::abs(z - other.z) < epsilon;
}

bool Quaternion::operator!=(const Quaternion& other) const {
    return !(*this == other);
}

// Основные операции
float Quaternion::magnitude() const {
    return std::sqrt(w * w + x * x + y * y + z * z);
}

float Quaternion::magnitudeSquared() const {
    return w * w + x * x + y * y + z * z;
}

Quaternion Quaternion::normalized() const {
    const float mag = magnitude();
    if (mag < 1e-6f) {
        return identity();
    }
    return *this / mag;
}

void Quaternion::normalize() {
    const float mag = magnitude();
    if (mag > 1e-6f) {
        *this /= mag;
    } else {
        *this = identity();
    }
}

Quaternion Quaternion::conjugate() const {
    return Quaternion(w, -x, -y, -z);
}

Quaternion Quaternion::inverse() const {
    const float magSquared = magnitudeSquared();
    if (magSquared < 1e-6f) {
        return identity();
    }
    return conjugate() / magSquared;
}

float Quaternion::dot(const Quaternion& other) const {
    return w * other.w + x * other.x + y * other.y + z * other.z;
}

// Повороты
Vector3 Quaternion::rotate(const Vector3& vec) const {
    // Используем формулу: v' = q * v * q^(-1)
    // где v представлен как кватернион (0, vx, vy, vz)
    const Quaternion qv(0.0f, vec.x, vec.y, vec.z);
    const Quaternion result = (*this) * qv * inverse();
    return Vector3(result.x, result.y, result.z);
}

Matrix4 Quaternion::toMatrix() const {
    const float xx = x * x;
    const float yy = y * y;
    const float zz = z * z;
    const float xy = x * y;
    const float xz = x * z;
    const float yz = y * z;
    const float wx = w * x;
    const float wy = w * y;
    const float wz = w * z;

    Matrix4 result;
    result.m[0][0] = 1.0f - 2.0f * (yy + zz);
    result.m[0][1] = 2.0f * (xy - wz);
    result.m[0][2] = 2.0f * (xz + wy);
    result.m[0][3] = 0.0f;

    result.m[1][0] = 2.0f * (xy + wz);
    result.m[1][1] = 1.0f - 2.0f * (xx + zz);
    result.m[1][2] = 2.0f * (yz - wx);
    result.m[1][3] = 0.0f;

    result.m[2][0] = 2.0f * (xz - wy);
    result.m[2][1] = 2.0f * (yz + wx);
    result.m[2][2] = 1.0f - 2.0f * (xx + yy);
    result.m[2][3] = 0.0f;

    result.m[3][0] = 0.0f;
    result.m[3][1] = 0.0f;
    result.m[3][2] = 0.0f;
    result.m[3][3] = 1.0f;

    return result;
}

Vector3 Quaternion::toEulerAngles() const {
    Vector3 angles;

    // Roll (x-axis rotation)
    const float sinr_cosp = 2.0f * (w * x + y * z);
    const float cosr_cosp = 1.0f - 2.0f * (x * x + y * y);
    angles.x = std::atan2(sinr_cosp, cosr_cosp);

    // Pitch (y-axis rotation)
    const float sinp = 2.0f * (w * y - z * x);
    if (std::abs(sinp) >= 1.0f) {
        angles.y = std::copysign(M_PI / 2.0f, sinp); // use 90 degrees if out of range
    } else {
        angles.y = std::asin(sinp);
    }

    // Yaw (z-axis rotation)
    const float siny_cosp = 2.0f * (w * z + x * y);
    const float cosy_cosp = 1.0f - 2.0f * (y * y + z * z);
    angles.z = std::atan2(siny_cosp, cosy_cosp);

    return angles;
}

void Quaternion::toAxisAngle(Vector3& axis, float& angle) const {
    const float sinHalfAngle = std::sqrt(x * x + y * y + z * z);
    
    if (sinHalfAngle < 1e-6f) {
        // Нет поворота
        axis = Vector3::unitX();
        angle = 0.0f;
    } else {
        angle = 2.0f * std::atan2(sinHalfAngle, w);
        axis = Vector3(x, y, z) / sinHalfAngle;
    }
}

// Создание поворотов
Quaternion Quaternion::fromAxisAngle(const Vector3& axis, float angle) {
    return Quaternion(axis, angle);
}

Quaternion Quaternion::fromEulerAngles(float pitch, float yaw, float roll) {
    return Quaternion(Vector3(roll, pitch, yaw));
}

Quaternion Quaternion::fromEulerAngles(const Vector3& eulerAngles) {
    return Quaternion(eulerAngles);
}

Quaternion Quaternion::fromMatrix(const Matrix4& mat) {
    const float trace = mat.m[0][0] + mat.m[1][1] + mat.m[2][2];
    
    if (trace > 0.0f) {
        const float s = std::sqrt(trace + 1.0f) * 2.0f; // s = 4 * qw
        return Quaternion(
            0.25f * s,
            (mat.m[2][1] - mat.m[1][2]) / s,
            (mat.m[0][2] - mat.m[2][0]) / s,
            (mat.m[1][0] - mat.m[0][1]) / s
        );
    } else if (mat.m[0][0] > mat.m[1][1] && mat.m[0][0] > mat.m[2][2]) {
        const float s = std::sqrt(1.0f + mat.m[0][0] - mat.m[1][1] - mat.m[2][2]) * 2.0f; // s = 4 * qx
        return Quaternion(
            (mat.m[2][1] - mat.m[1][2]) / s,
            0.25f * s,
            (mat.m[0][1] + mat.m[1][0]) / s,
            (mat.m[0][2] + mat.m[2][0]) / s
        );
    } else if (mat.m[1][1] > mat.m[2][2]) {
        const float s = std::sqrt(1.0f + mat.m[1][1] - mat.m[0][0] - mat.m[2][2]) * 2.0f; // s = 4 * qy
        return Quaternion(
            (mat.m[0][2] - mat.m[2][0]) / s,
            (mat.m[0][1] + mat.m[1][0]) / s,
            0.25f * s,
            (mat.m[1][2] + mat.m[2][1]) / s
        );
    } else {
        const float s = std::sqrt(1.0f + mat.m[2][2] - mat.m[0][0] - mat.m[1][1]) * 2.0f; // s = 4 * qz
        return Quaternion(
            (mat.m[1][0] - mat.m[0][1]) / s,
            (mat.m[0][2] + mat.m[2][0]) / s,
            (mat.m[1][2] + mat.m[2][1]) / s,
            0.25f * s
        );
    }
}

Quaternion Quaternion::lookRotation(const Vector3& forward, const Vector3& up) {
    const Vector3 normalizedForward = forward.normalized();
    const Vector3 normalizedUp = up.normalized();
    
    const Vector3 right = normalizedUp.cross(normalizedForward).normalized();
    const Vector3 actualUp = normalizedForward.cross(right);
    
    Matrix4 rotationMatrix;
    rotationMatrix.m[0][0] = right.x;
    rotationMatrix.m[0][1] = right.y;
    rotationMatrix.m[0][2] = right.z;
    rotationMatrix.m[1][0] = actualUp.x;
    rotationMatrix.m[1][1] = actualUp.y;
    rotationMatrix.m[1][2] = actualUp.z;
    rotationMatrix.m[2][0] = normalizedForward.x;
    rotationMatrix.m[2][1] = normalizedForward.y;
    rotationMatrix.m[2][2] = normalizedForward.z;
    
    return fromMatrix(rotationMatrix);
}

// Интерполяция
Quaternion Quaternion::slerp(const Quaternion& other, float t) const {
    const float cosTheta = dot(other);
    
    // Если кватернионы очень близки, используем линейную интерполяцию
    if (std::abs(cosTheta) >= 1.0f - 1e-6f) {
        return lerp(other, t);
    }
    
    const float theta = std::acos(std::abs(cosTheta));
    const float sinTheta = std::sin(theta);
    
    const float w1 = std::sin((1.0f - t) * theta) / sinTheta;
    const float w2 = std::sin(t * theta) / sinTheta;
    
    if (cosTheta < 0.0f) {
        return *this * w1 - other * w2;
    } else {
        return *this * w1 + other * w2;
    }
}

Quaternion Quaternion::lerp(const Quaternion& other, float t) const {
    if (dot(other) < 0.0f) {
        return (*this * (1.0f - t) - other * t).normalized();
    } else {
        return (*this * (1.0f - t) + other * t).normalized();
    }
}

Quaternion Quaternion::nlerp(const Quaternion& other, float t) const {
    return lerp(other, t);
}

// Утилиты
void Quaternion::setIdentity() {
    w = 1.0f;
    x = 0.0f;
    y = 0.0f;
    z = 0.0f;
}

bool Quaternion::isIdentity() const {
    const float epsilon = 1e-6f;
    return std::abs(w - 1.0f) < epsilon &&
           std::abs(x) < epsilon &&
           std::abs(y) < epsilon &&
           std::abs(z) < epsilon;
}

bool Quaternion::isZero() const {
    const float epsilon = 1e-6f;
    return std::abs(w) < epsilon &&
           std::abs(x) < epsilon &&
           std::abs(y) < epsilon &&
           std::abs(z) < epsilon;
}

void Quaternion::set(float w, float x, float y, float z) {
    this->w = w;
    this->x = x;
    this->y = y;
    this->z = z;
}

float Quaternion::angle() const {
    return 2.0f * std::acos(std::abs(w));
}

Vector3 Quaternion::axis() const {
    const float sinHalfAngle = std::sqrt(x * x + y * y + z * z);
    
    if (sinHalfAngle < 1e-6f) {
        return Vector3::unitX();
    } else {
        return Vector3(x, y, z) / sinHalfAngle;
    }
}

// Статические константы
Quaternion Quaternion::identity() {
    return Quaternion(1.0f, 0.0f, 0.0f, 0.0f);
}

Quaternion Quaternion::zero() {
    return Quaternion(0.0f, 0.0f, 0.0f, 0.0f);
}

// Операторы ввода/вывода
std::ostream& operator<<(std::ostream& os, const Quaternion& q) {
    os << "Quaternion(" << q.w << ", " << q.x << ", " << q.y << ", " << q.z << ")";
    return os;
}

std::istream& operator>>(std::istream& is, Quaternion& q) {
    is >> q.w >> q.x >> q.y >> q.z;
    return is;
}

// Оператор для скалярного умножения слева
Quaternion operator*(float scalar, const Quaternion& q) {
    return q * scalar;
}

} // namespace Math
} // namespace HyperEngine
