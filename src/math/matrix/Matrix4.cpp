#include "HyperEngine/Math/Matrix4.h"
#include <algorithm>
#include <cmath>
#include <limits>
#include <stdexcept>
#include "HyperEngine/Math/Quaternion.h"

using namespace HyperEngine::Math;

// Конструкторы
Matrix4::Matrix4() {
    setIdentity();
}

Matrix4::Matrix4(const Matrix4& other) : m(other.m) {}

Matrix4::Matrix4(float diagonal) {
    setZero();
    for (int i = 0; i < 4; ++i) {
        m[i][i] = diagonal;
    }
}

Matrix4::Matrix4(const std::array<std::array<float, 4>, 4>& data) : m(data) {}

// Операторы
Matrix4& Matrix4::operator=(const Matrix4& other) {
    if (this != &other) {
        m = other.m;
    }
    return *this;
}

Matrix4 Matrix4::operator+(const Matrix4& other) const {
    Matrix4 result;
    for (int i = 0; i < 4; ++i) {
        for (int j = 0; j < 4; ++j) {
            result.m[i][j] = m[i][j] + other.m[i][j];
        }
    }
    return result;
}

Matrix4 Matrix4::operator-(const Matrix4& other) const {
    Matrix4 result;
    for (int i = 0; i < 4; ++i) {
        for (int j = 0; j < 4; ++j) {
            result.m[i][j] = m[i][j] - other.m[i][j];
        }
    }
    return result;
}

Matrix4 Matrix4::operator*(const Matrix4& other) const {
    Matrix4 result;
    for (int i = 0; i < 4; ++i) {
        for (int j = 0; j < 4; ++j) {
            result.m[i][j] = 0.0f;
            for (int k = 0; k < 4; ++k) {
                result.m[i][j] += m[i][k] * other.m[k][j];
            }
        }
    }
    return result;
}

Matrix4 Matrix4::operator*(float scalar) const {
    Matrix4 result;
    for (int i = 0; i < 4; ++i) {
        for (int j = 0; j < 4; ++j) {
            result.m[i][j] = m[i][j] * scalar;
        }
    }
    return result;
}

Vector3 Matrix4::operator*(const Vector3& vec) const {
    // Преобразуем в гомогенные координаты (добавляем w=1)
    float x = m[0][0] * vec.x + m[0][1] * vec.y + m[0][2] * vec.z + m[0][3];
    float y = m[1][0] * vec.x + m[1][1] * vec.y + m[1][2] * vec.z + m[1][3];
    float z = m[2][0] * vec.x + m[2][1] * vec.y + m[2][2] * vec.z + m[2][3];
    float w = m[3][0] * vec.x + m[3][1] * vec.y + m[3][2] * vec.z + m[3][3];

    // Если w != 1, выполняем перспективное деление
    if (std::abs(w) > std::numeric_limits<float>::epsilon()
        && std::abs(w - 1.0f) > std::numeric_limits<float>::epsilon()) {
        x /= w;
        y /= w;
        z /= w;
    }

    return Vector3(x, y, z);
}

Matrix4& Matrix4::operator+=(const Matrix4& other) {
    for (int i = 0; i < 4; ++i) {
        for (int j = 0; j < 4; ++j) {
            m[i][j] += other.m[i][j];
        }
    }
    return *this;
}

Matrix4& Matrix4::operator-=(const Matrix4& other) {
    for (int i = 0; i < 4; ++i) {
        for (int j = 0; j < 4; ++j) {
            m[i][j] -= other.m[i][j];
        }
    }
    return *this;
}

Matrix4& Matrix4::operator*=(const Matrix4& other) {
    *this = *this * other;
    return *this;
}

Matrix4& Matrix4::operator*=(float scalar) {
    for (int i = 0; i < 4; ++i) {
        for (int j = 0; j < 4; ++j) {
            m[i][j] *= scalar;
        }
    }
    return *this;
}

bool Matrix4::operator==(const Matrix4& other) const {
    const float epsilon = std::numeric_limits<float>::epsilon();
    for (int i = 0; i < 4; ++i) {
        for (int j = 0; j < 4; ++j) {
            if (std::abs(m[i][j] - other.m[i][j]) >= epsilon) {
                return false;
            }
        }
    }
    return true;
}

bool Matrix4::operator!=(const Matrix4& other) const {
    return !(*this == other);
}

float& Matrix4::operator()(int row, int col) {
    return m[row][col];
}

const float& Matrix4::operator()(int row, int col) const {
    return m[row][col];
}

// Основные операции
Matrix4 Matrix4::transpose() const {
    Matrix4 result;
    for (int i = 0; i < 4; ++i) {
        for (int j = 0; j < 4; ++j) {
            result.m[i][j] = m[j][i];
        }
    }
    return result;
}

float Matrix4::determinant() const {
    // Вычисление определителя 4x4 матрицы через разложение по первой строке
    float det = 0.0f;

    // Миноры 3x3
    float m00 = m[1][1] * (m[2][2] * m[3][3] - m[2][3] * m[3][2])
                - m[1][2] * (m[2][1] * m[3][3] - m[2][3] * m[3][1])
                + m[1][3] * (m[2][1] * m[3][2] - m[2][2] * m[3][1]);

    float m01 = m[1][0] * (m[2][2] * m[3][3] - m[2][3] * m[3][2])
                - m[1][2] * (m[2][0] * m[3][3] - m[2][3] * m[3][0])
                + m[1][3] * (m[2][0] * m[3][2] - m[2][2] * m[3][0]);

    float m02 = m[1][0] * (m[2][1] * m[3][3] - m[2][3] * m[3][1])
                - m[1][1] * (m[2][0] * m[3][3] - m[2][3] * m[3][0])
                + m[1][3] * (m[2][0] * m[3][1] - m[2][1] * m[3][0]);

    float m03 = m[1][0] * (m[2][1] * m[3][2] - m[2][2] * m[3][1])
                - m[1][1] * (m[2][0] * m[3][2] - m[2][2] * m[3][0])
                + m[1][2] * (m[2][0] * m[3][1] - m[2][1] * m[3][0]);

    det = m[0][0] * m00 - m[0][1] * m01 + m[0][2] * m02 - m[0][3] * m03;

    return det;
}

Matrix4 Matrix4::adjugate() const {
    Matrix4 result;

    // Вычисляем алгебраические дополнения для каждого элемента
    for (int i = 0; i < 4; ++i) {
        for (int j = 0; j < 4; ++j) {
            // Создаем минор 3x3, исключая строку i и столбец j
            float minor[3][3];
            int mi = 0, mj = 0;

            for (int row = 0; row < 4; ++row) {
                if (row == i)
                    continue;
                mj = 0;
                for (int col = 0; col < 4; ++col) {
                    if (col == j)
                        continue;
                    minor[mi][mj] = m[row][col];
                    mj++;
                }
                mi++;
            }

            // Вычисляем определитель минора 3x3
            float det3 = minor[0][0] * (minor[1][1] * minor[2][2] - minor[1][2] * minor[2][1])
                         - minor[0][1] * (minor[1][0] * minor[2][2] - minor[1][2] * minor[2][0])
                         + minor[0][2] * (minor[1][0] * minor[2][1] - minor[1][1] * minor[2][0]);

            // Алгебраическое дополнение с учетом знака
            result.m[j][i] = ((i + j) % 2 == 0) ? det3 : -det3;
        }
    }

    return result;
}

Matrix4 Matrix4::inverse() const {
    float det = determinant();
    if (std::abs(det) < std::numeric_limits<float>::epsilon()) {
        throw std::runtime_error("Матрица необратима (определитель равен нулю)");
    }

    return adjugate() * (1.0f / det);
}

bool Matrix4::isInvertible() const {
    return std::abs(determinant()) >= std::numeric_limits<float>::epsilon();
}

void Matrix4::setIdentity() {
    setZero();
    for (int i = 0; i < 4; ++i) {
        m[i][i] = 1.0f;
    }
}

void Matrix4::setZero() {
    for (int i = 0; i < 4; ++i) {
        for (int j = 0; j < 4; ++j) {
            m[i][j] = 0.0f;
        }
    }
}

// Трансформации
Matrix4 Matrix4::translation(const Vector3& translation) {
    return Matrix4::translation(translation.x, translation.y, translation.z);
}

Matrix4 Matrix4::translation(float x, float y, float z) {
    Matrix4 result;
    result.setIdentity();
    result.m[0][3] = x;
    result.m[1][3] = y;
    result.m[2][3] = z;
    return result;
}

Matrix4 Matrix4::scaling(const Vector3& scale) {
    return Matrix4::scaling(scale.x, scale.y, scale.z);
}

Matrix4 Matrix4::scaling(float x, float y, float z) {
    Matrix4 result;
    result.setZero();
    result.m[0][0] = x;
    result.m[1][1] = y;
    result.m[2][2] = z;
    result.m[3][3] = 1.0f;
    return result;
}

Matrix4 Matrix4::scaling(float uniformScale) {
    return Matrix4::scaling(uniformScale, uniformScale, uniformScale);
}

// Повороты
Matrix4 Matrix4::rotationX(float angle) {
    Matrix4 result;
    result.setIdentity();
    float c = std::cos(angle);
    float s = std::sin(angle);
    result.m[1][1] = c;
    result.m[1][2] = -s;
    result.m[2][1] = s;
    result.m[2][2] = c;
    return result;
}

Matrix4 Matrix4::rotationY(float angle) {
    Matrix4 result;
    result.setIdentity();
    float c = std::cos(angle);
    float s = std::sin(angle);
    result.m[0][0] = c;
    result.m[0][2] = s;
    result.m[2][0] = -s;
    result.m[2][2] = c;
    return result;
}

Matrix4 Matrix4::rotationZ(float angle) {
    Matrix4 result;
    result.setIdentity();
    float c = std::cos(angle);
    float s = std::sin(angle);
    result.m[0][0] = c;
    result.m[0][1] = -s;
    result.m[1][0] = s;
    result.m[1][1] = c;
    return result;
}

Matrix4 Matrix4::rotation(const Vector3& axis, float angle) {
    Vector3 normalizedAxis = axis.normalized();
    float c = std::cos(angle);
    float s = std::sin(angle);
    float t = 1.0f - c;

    Matrix4 result;
    result.setIdentity();

    // Формула Родригеса для поворота вокруг произвольной оси
    result.m[0][0] = t * normalizedAxis.x * normalizedAxis.x + c;
    result.m[0][1] = t * normalizedAxis.x * normalizedAxis.y - s * normalizedAxis.z;
    result.m[0][2] = t * normalizedAxis.x * normalizedAxis.z + s * normalizedAxis.y;

    result.m[1][0] = t * normalizedAxis.x * normalizedAxis.y + s * normalizedAxis.z;
    result.m[1][1] = t * normalizedAxis.y * normalizedAxis.y + c;
    result.m[1][2] = t * normalizedAxis.y * normalizedAxis.z - s * normalizedAxis.x;

    result.m[2][0] = t * normalizedAxis.x * normalizedAxis.z - s * normalizedAxis.y;
    result.m[2][1] = t * normalizedAxis.y * normalizedAxis.z + s * normalizedAxis.x;
    result.m[2][2] = t * normalizedAxis.z * normalizedAxis.z + c;

    return result;
}

Matrix4 Matrix4::rotationFromQuaternion(const Quaternion& q) {
    return q.toMatrix();
}

Matrix4 Matrix4::eulerAngles(float pitch, float yaw, float roll) {
    // Порядок применения: Roll (Z) -> Pitch (X) -> Yaw (Y)
    Matrix4 rotX = rotationX(pitch);
    Matrix4 rotY = rotationY(yaw);
    Matrix4 rotZ = rotationZ(roll);

    return rotY * rotX * rotZ;
}

// Камера и проекции
Matrix4 Matrix4::lookAt(const Vector3& eye, const Vector3& target, const Vector3& up) {
    Vector3 forward = (target - eye).normalized();
    Vector3 right = forward.cross(up).normalized();
    Vector3 upCorrected = right.cross(forward).normalized();

    Matrix4 result;
    result.setIdentity();

    // Ориентация
    result.m[0][0] = right.x;
    result.m[0][1] = right.y;
    result.m[0][2] = right.z;

    result.m[1][0] = upCorrected.x;
    result.m[1][1] = upCorrected.y;
    result.m[1][2] = upCorrected.z;

    result.m[2][0] = -forward.x;
    result.m[2][1] = -forward.y;
    result.m[2][2] = -forward.z;

    // Позиция (через скалярное произведение для инверсии)
    result.m[0][3] = -right.dot(eye);
    result.m[1][3] = -upCorrected.dot(eye);
    result.m[2][3] = forward.dot(eye);

    return result;
}

Matrix4 Matrix4::perspective(float fovY, float aspect, float near, float far) {
    Matrix4 result;
    result.setZero();

    float tanHalfFov = std::tan(fovY * 0.5f);

    result.m[0][0] = 1.0f / (aspect * tanHalfFov);
    result.m[1][1] = 1.0f / tanHalfFov;
    result.m[2][2] = -(far + near) / (far - near);
    result.m[2][3] = -1.0f;
    result.m[3][2] = -(2.0f * far * near) / (far - near);

    return result;
}

Matrix4 Matrix4::orthographic(float left,
                              float right,
                              float bottom,
                              float top,
                              float near,
                              float far) {
    Matrix4 result;
    result.setZero();

    result.m[0][0] = 2.0f / (right - left);
    result.m[1][1] = 2.0f / (top - bottom);
    result.m[2][2] = -2.0f / (far - near);
    result.m[3][3] = 1.0f;

    result.m[0][3] = -(right + left) / (right - left);
    result.m[1][3] = -(top + bottom) / (top - bottom);
    result.m[2][3] = -(far + near) / (far - near);

    return result;
}

// Утилиты
Vector3 Matrix4::transformPoint(const Vector3& point) const {
    return *this * point;
}

Vector3 Matrix4::transformDirection(const Vector3& direction) const {
    // Для направлений не применяем трансляцию
    Vector3 result;
    result.x = m[0][0] * direction.x + m[0][1] * direction.y + m[0][2] * direction.z;
    result.y = m[1][0] * direction.x + m[1][1] * direction.y + m[1][2] * direction.z;
    result.z = m[2][0] * direction.x + m[2][1] * direction.y + m[2][2] * direction.z;
    return result;
}

Vector3 Matrix4::transformVector(const Vector3& vector) const {
    // Алиас для transformPoint - трансформируем как точку с трансляцией
    return transformPoint(vector);
}

Vector3 Matrix4::getTranslation() const {
    return Vector3(m[0][3], m[1][3], m[2][3]);
}

Vector3 Matrix4::getScale() const {
    Vector3 scale;
    scale.x = Vector3(m[0][0], m[1][0], m[2][0]).magnitude();
    scale.y = Vector3(m[0][1], m[1][1], m[2][1]).magnitude();
    scale.z = Vector3(m[0][2], m[1][2], m[2][2]).magnitude();
    return scale;
}

void Matrix4::decompose(Vector3& translation, Quaternion& rotation, Vector3& scale) const {
    // Извлекаем трансляцию
    translation = getTranslation();

    // Извлекаем масштаб
    scale = getScale();

    // Создаем матрицу без масштаба для извлечения поворота
    Matrix4 rotMatrix = *this;
    rotMatrix.m[0][3] = 0;
    rotMatrix.m[1][3] = 0;
    rotMatrix.m[2][3] = 0;
    rotMatrix.m[3][3] = 1;

    // Нормализуем столбцы
    if (scale.x != 0.0f) {
        rotMatrix.m[0][0] /= scale.x;
        rotMatrix.m[1][0] /= scale.x;
        rotMatrix.m[2][0] /= scale.x;
    }
    if (scale.y != 0.0f) {
        rotMatrix.m[0][1] /= scale.y;
        rotMatrix.m[1][1] /= scale.y;
        rotMatrix.m[2][1] /= scale.y;
    }
    if (scale.z != 0.0f) {
        rotMatrix.m[0][2] /= scale.z;
        rotMatrix.m[1][2] /= scale.z;
        rotMatrix.m[2][2] /= scale.z;
    }

    rotation = Quaternion::fromMatrix(rotMatrix);
}

// Статические константы
Matrix4 Matrix4::identity() {
    Matrix4 result;
    result.setIdentity();
    return result;
}

Matrix4 Matrix4::zero() {
    Matrix4 result;
    result.setZero();
    return result;
}

// Операторы ввода/вывода
namespace HyperEngine::Math {
std::ostream& operator<<(std::ostream& os, const Matrix4& mat) {
    os << "Matrix4:\n";
    for (int i = 0; i < 4; ++i) {
        os << "[";
        for (int j = 0; j < 4; ++j) {
            os << mat.m[i][j];
            if (j < 3)
                os << ", ";
        }
        os << "]\n";
    }
    return os;
}

std::istream& operator>>(std::istream& is, Matrix4& mat) {
    for (int i = 0; i < 4; ++i) {
        for (int j = 0; j < 4; ++j) {
            is >> mat.m[i][j];
        }
    }
    return is;
}

// Оператор для скалярного умножения слева
Matrix4 operator*(float scalar, const Matrix4& mat) {
    return mat * scalar;
}

}  // namespace HyperEngine::Math
