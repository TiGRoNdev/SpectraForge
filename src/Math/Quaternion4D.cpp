#include "Engine4D/Math/Quaternion4D.h"
#include <cmath>
#include <algorithm>
#include <limits>

namespace Engine4D {
namespace Math {

// Конструкторы
Quaternion4D::Quaternion4D() : w(1.0f), x(0.0f), y(0.0f), z(0.0f), u(0.0f), v(0.0f) {}

Quaternion4D::Quaternion4D(float w, float x, float y, float z, float u, float v) 
    : w(w), x(x), y(y), z(z), u(u), v(v) {}

Quaternion4D::Quaternion4D(const Quaternion4D& other) 
    : w(other.w), x(other.x), y(other.y), z(other.z), u(other.u), v(other.v) {}

Quaternion4D::Quaternion4D(const Vector4& axis, float angle) {
    float halfAngle = angle * 0.5f;
    float s = std::sin(halfAngle);
    float c = std::cos(halfAngle);
    
    w = c;
    x = axis.x * s;
    y = axis.y * s;
    z = axis.z * s;
    u = axis.w * s;  // Используем w-координату для 4D поворота
    v = 0.0f;        // Дополнительный параметр для сложных поворотов
}

// Операторы
Quaternion4D& Quaternion4D::operator=(const Quaternion4D& other) {
    if (this != &other) {
        w = other.w;
        x = other.x;
        y = other.y;
        z = other.z;
        u = other.u;
        v = other.v;
    }
    return *this;
}

Quaternion4D Quaternion4D::operator+(const Quaternion4D& other) const {
    return Quaternion4D(w + other.w, x + other.x, y + other.y, z + other.z, 
                       u + other.u, v + other.v);
}

Quaternion4D Quaternion4D::operator-(const Quaternion4D& other) const {
    return Quaternion4D(w - other.w, x - other.x, y - other.y, z - other.z, 
                       u - other.u, v - other.v);
}

Quaternion4D Quaternion4D::operator*(const Quaternion4D& other) const {
    // Умножение 4D кватернионов
    return Quaternion4D(
        w * other.w - x * other.x - y * other.y - z * other.z - u * other.u - v * other.v,
        w * other.x + x * other.w + y * other.z - z * other.y + u * other.v - v * other.u,
        w * other.y - x * other.z + y * other.w + z * other.x - u * other.v + v * other.u,
        w * other.z + x * other.y - y * other.x + z * other.w + u * other.u - v * other.v,
        w * other.u - x * other.v + y * other.v - z * other.u + u * other.w + v * other.z,
        w * other.v + x * other.u - y * other.u + z * other.v - u * other.z + v * other.w
    );
}

Quaternion4D Quaternion4D::operator*(float scalar) const {
    return Quaternion4D(w * scalar, x * scalar, y * scalar, z * scalar, 
                       u * scalar, v * scalar);
}

Quaternion4D& Quaternion4D::operator+=(const Quaternion4D& other) {
    w += other.w;
    x += other.x;
    y += other.y;
    z += other.z;
    u += other.u;
    v += other.v;
    return *this;
}

Quaternion4D& Quaternion4D::operator-=(const Quaternion4D& other) {
    w -= other.w;
    x -= other.x;
    y -= other.y;
    z -= other.z;
    u -= other.u;
    v -= other.v;
    return *this;
}

Quaternion4D& Quaternion4D::operator*=(const Quaternion4D& other) {
    *this = *this * other;
    return *this;
}

Quaternion4D& Quaternion4D::operator*=(float scalar) {
    w *= scalar;
    x *= scalar;
    y *= scalar;
    z *= scalar;
    u *= scalar;
    v *= scalar;
    return *this;
}

bool Quaternion4D::operator==(const Quaternion4D& other) const {
    const float epsilon = std::numeric_limits<float>::epsilon();
    return std::abs(w - other.w) < epsilon &&
           std::abs(x - other.x) < epsilon &&
           std::abs(y - other.y) < epsilon &&
           std::abs(z - other.z) < epsilon &&
           std::abs(u - other.u) < epsilon &&
           std::abs(v - other.v) < epsilon;
}

bool Quaternion4D::operator!=(const Quaternion4D& other) const {
    return !(*this == other);
}

// Основные операции
float Quaternion4D::magnitude() const {
    return std::sqrt(w * w + x * x + y * y + z * z + u * u + v * v);
}

float Quaternion4D::magnitudeSquared() const {
    return w * w + x * x + y * y + z * z + u * u + v * v;
}

Quaternion4D Quaternion4D::normalized() const {
    float mag = magnitude();
    if (mag < std::numeric_limits<float>::epsilon()) {
        return Quaternion4D::identity();
    }
    return *this / mag;
}

void Quaternion4D::normalize() {
    float mag = magnitude();
    if (mag >= std::numeric_limits<float>::epsilon()) {
        *this /= mag;
    } else {
        setIdentity();
    }
}

Quaternion4D Quaternion4D::conjugate() const {
    return Quaternion4D(w, -x, -y, -z, -u, -v);
}

Quaternion4D Quaternion4D::inverse() const {
    float magSq = magnitudeSquared();
    if (magSq < std::numeric_limits<float>::epsilon()) {
        return Quaternion4D::zero();
    }
    return conjugate() / magSq;
}

float Quaternion4D::dot(const Quaternion4D& other) const {
    return w * other.w + x * other.x + y * other.y + z * other.z + u * other.u + v * other.v;
}

// Повороты
Vector4 Quaternion4D::rotate(const Vector4& vec) const {
    // Преобразуем вектор в кватернион для поворота
    Quaternion4D vecQuat(0.0f, vec.x, vec.y, vec.z, vec.w, 0.0f);
    
    // Поворачиваем: q * v * q^-1
    Quaternion4D result = *this * vecQuat * inverse();
    
    return Vector4(result.x, result.y, result.z, result.u);
}

Matrix4 Quaternion4D::toMatrix() const {
    Matrix4 result;
    
    // Нормализуем кватернион
    Quaternion4D q = normalized();
    
    // Вычисляем элементы матрицы поворота
    float xx = q.x * q.x;
    float yy = q.y * q.y;
    float zz = q.z * q.z;
    float ww = q.w * q.w;
    float uu = q.u * q.u;
    float vv = q.v * q.v;
    
    float xy = q.x * q.y;
    float xz = q.x * q.z;
    float xw = q.x * q.w;
    float xu = q.x * q.u;
    float xv = q.x * q.v;
    float yz = q.y * q.z;
    float yw = q.y * q.w;
    float yu = q.y * q.u;
    float yv = q.y * q.v;
    float zw = q.z * q.w;
    float zu = q.z * q.u;
    float zv = q.z * q.v;
    float wu = q.w * q.u;
    float wv = q.w * q.v;
    float uv = q.u * q.v;
    
    // Заполняем матрицу поворота 4x4
    result.m[0][0] = ww + xx - yy - zz - uu - vv;
    result.m[0][1] = 2.0f * (xy - wv + zu);
    result.m[0][2] = 2.0f * (xz + wu - yv);
    result.m[0][3] = 2.0f * (xw - yu + zv);
    
    result.m[1][0] = 2.0f * (xy + wv - zu);
    result.m[1][1] = ww - xx + yy - zz - uu - vv;
    result.m[1][2] = 2.0f * (yz - wu - xv);
    result.m[1][3] = 2.0f * (yw + xu - zv);
    
    result.m[2][0] = 2.0f * (xz - wu + yv);
    result.m[2][1] = 2.0f * (yz + wu + xv);
    result.m[2][2] = ww - xx - yy + zz - uu - vv;
    result.m[2][3] = 2.0f * (zw - xu - yv);
    
    result.m[3][0] = 2.0f * (xw + yu - zv);
    result.m[3][1] = 2.0f * (yw - xu + zv);
    result.m[3][2] = 2.0f * (zw + xu + yv);
    result.m[3][3] = ww - xx - yy - zz + uu + vv;
    
    return result;
}

Quaternion4D Quaternion4D::fromMatrix(const Matrix4& mat) {
    // Извлечение кватерниона из матрицы поворота
    // Это упрощенная версия для 4D случая
    float trace = mat.m[0][0] + mat.m[1][1] + mat.m[2][2] + mat.m[3][3];
    
    if (trace > 0.0f) {
        float s = std::sqrt(trace + 1.0f) * 2.0f; // s = 4 * w
        float w = 0.25f * s;
        float x = (mat.m[2][1] - mat.m[1][2]) / s;
        float y = (mat.m[0][2] - mat.m[2][0]) / s;
        float z = (mat.m[1][0] - mat.m[0][1]) / s;
        return Quaternion4D(w, x, y, z);
    } else if (mat.m[0][0] > mat.m[1][1] && mat.m[0][0] > mat.m[2][2] && mat.m[0][0] > mat.m[3][3]) {
        float s = std::sqrt(1.0f + mat.m[0][0] - mat.m[1][1] - mat.m[2][2] - mat.m[3][3]) * 2.0f;
        float w = (mat.m[2][1] - mat.m[1][2]) / s;
        float x = 0.25f * s;
        float y = (mat.m[0][1] + mat.m[1][0]) / s;
        float z = (mat.m[0][2] + mat.m[2][0]) / s;
        return Quaternion4D(w, x, y, z);
    } else if (mat.m[1][1] > mat.m[2][2] && mat.m[1][1] > mat.m[3][3]) {
        float s = std::sqrt(1.0f + mat.m[1][1] - mat.m[0][0] - mat.m[2][2] - mat.m[3][3]) * 2.0f;
        float w = (mat.m[0][2] - mat.m[2][0]) / s;
        float x = (mat.m[0][1] + mat.m[1][0]) / s;
        float y = 0.25f * s;
        float z = (mat.m[1][2] + mat.m[2][1]) / s;
        return Quaternion4D(w, x, y, z);
    } else if (mat.m[2][2] > mat.m[3][3]) {
        float s = std::sqrt(1.0f + mat.m[2][2] - mat.m[0][0] - mat.m[1][1] - mat.m[3][3]) * 2.0f;
        float w = (mat.m[1][0] - mat.m[0][1]) / s;
        float x = (mat.m[0][2] + mat.m[2][0]) / s;
        float y = (mat.m[1][2] + mat.m[2][1]) / s;
        float z = 0.25f * s;
        return Quaternion4D(w, x, y, z);
    } else {
        float s = std::sqrt(1.0f + mat.m[3][3] - mat.m[0][0] - mat.m[1][1] - mat.m[2][2]) * 2.0f;
        float w = (mat.m[0][1] - mat.m[1][0]) / s;
        float x = (mat.m[0][3] + mat.m[3][0]) / s;
        float y = (mat.m[1][3] + mat.m[3][1]) / s;
        float z = (mat.m[2][3] + mat.m[3][2]) / s;
        return Quaternion4D(w, x, y, z);
    }
}

// Создание поворотов в различных плоскостях
Quaternion4D Quaternion4D::rotationXY(float angle) {
    float halfAngle = angle * 0.5f;
    return Quaternion4D(std::cos(halfAngle), 0.0f, 0.0f, 0.0f, 0.0f, std::sin(halfAngle));
}

Quaternion4D Quaternion4D::rotationXZ(float angle) {
    float halfAngle = angle * 0.5f;
    return Quaternion4D(std::cos(halfAngle), 0.0f, 0.0f, 0.0f, std::sin(halfAngle), 0.0f);
}

Quaternion4D Quaternion4D::rotationXW(float angle) {
    float halfAngle = angle * 0.5f;
    return Quaternion4D(std::cos(halfAngle), 0.0f, 0.0f, 0.0f, 0.0f, std::sin(halfAngle));
}

Quaternion4D Quaternion4D::rotationYZ(float angle) {
    float halfAngle = angle * 0.5f;
    return Quaternion4D(std::cos(halfAngle), 0.0f, 0.0f, 0.0f, 0.0f, std::sin(halfAngle));
}

Quaternion4D Quaternion4D::rotationYW(float angle) {
    float halfAngle = angle * 0.5f;
    return Quaternion4D(std::cos(halfAngle), 0.0f, 0.0f, 0.0f, 0.0f, std::sin(halfAngle));
}

Quaternion4D Quaternion4D::rotationZW(float angle) {
    float halfAngle = angle * 0.5f;
    return Quaternion4D(std::cos(halfAngle), 0.0f, 0.0f, 0.0f, 0.0f, std::sin(halfAngle));
}

// Интерполяция
Quaternion4D Quaternion4D::slerp(const Quaternion4D& other, float t) const {
    t = std::clamp(t, 0.0f, 1.0f);
    
    float dot = this->dot(other);
    
    // Если dot < 0, то slerp не будет кратчайшим путем
    Quaternion4D target = (dot < 0.0f) ? -other : other;
    if (dot < 0.0f) dot = -dot;
    
    // Если кватернионы очень близки, используем линейную интерполяцию
    if (dot > 0.9995f) {
        return (*this + t * (target - *this)).normalized();
    }
    
    float theta = std::acos(dot);
    float sinTheta = std::sin(theta);
    float factor1 = std::sin((1.0f - t) * theta) / sinTheta;
    float factor2 = std::sin(t * theta) / sinTheta;
    
    return *this * factor1 + target * factor2;
}

Quaternion4D Quaternion4D::lerp(const Quaternion4D& other, float t) const {
    t = std::clamp(t, 0.0f, 1.0f);
    return (*this + t * (other - *this)).normalized();
}

// Утилиты
void Quaternion4D::setIdentity() {
    w = 1.0f;
    x = y = z = u = v = 0.0f;
}

bool Quaternion4D::isIdentity() const {
    const float epsilon = std::numeric_limits<float>::epsilon();
    return std::abs(w - 1.0f) < epsilon &&
           std::abs(x) < epsilon &&
           std::abs(y) < epsilon &&
           std::abs(z) < epsilon &&
           std::abs(u) < epsilon &&
           std::abs(v) < epsilon;
}

bool Quaternion4D::isZero() const {
    const float epsilon = std::numeric_limits<float>::epsilon();
    return std::abs(w) < epsilon &&
           std::abs(x) < epsilon &&
           std::abs(y) < epsilon &&
           std::abs(z) < epsilon &&
           std::abs(u) < epsilon &&
           std::abs(v) < epsilon;
}

void Quaternion4D::set(float w, float x, float y, float z, float u, float v) {
    this->w = w;
    this->x = x;
    this->y = y;
    this->z = z;
    this->u = u;
    this->v = v;
}

// Статические константы
Quaternion4D Quaternion4D::identity() {
    return Quaternion4D(1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f);
}

Quaternion4D Quaternion4D::zero() {
    return Quaternion4D(0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f);
}

// Операторы ввода/вывода
std::ostream& operator<<(std::ostream& os, const Quaternion4D& q) {
    os << "Quaternion4D(" << q.w << ", " << q.x << ", " << q.y << ", " << q.z << ", " << q.u << ", " << q.v << ")";
    return os;
}

std::istream& operator>>(std::istream& is, Quaternion4D& q) {
    char ch;
    is >> ch; // '('
    is >> q.w >> ch; // w и ','
    is >> q.x >> ch; // x и ','
    is >> q.y >> ch; // y и ','
    is >> q.z >> ch; // z и ','
    is >> q.u >> ch; // u и ','
    is >> q.v >> ch; // v и ')'
    return is;
}

// Оператор для скалярного умножения слева
Quaternion4D operator*(float scalar, const Quaternion4D& q) {
    return q * scalar;
}

} // namespace Math
} // namespace Engine4D
