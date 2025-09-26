#include "Engine4D/Math/Matrix4.h"
#include <cmath>
#include <algorithm>
#include <limits>

namespace Engine4D {
namespace Math {

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

Vector4 Matrix4::operator*(const Vector4& vec) const {
    return Vector4(
        m[0][0] * vec.x + m[0][1] * vec.y + m[0][2] * vec.z + m[0][3] * vec.w,
        m[1][0] * vec.x + m[1][1] * vec.y + m[1][2] * vec.z + m[1][3] * vec.w,
        m[2][0] * vec.x + m[2][1] * vec.y + m[2][2] * vec.z + m[2][3] * vec.w,
        m[3][0] * vec.x + m[3][1] * vec.y + m[3][2] * vec.z + m[3][3] * vec.w
    );
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
    float m00 = m[1][1] * (m[2][2] * m[3][3] - m[2][3] * m[3][2]) -
                m[1][2] * (m[2][1] * m[3][3] - m[2][3] * m[3][1]) +
                m[1][3] * (m[2][1] * m[3][2] - m[2][2] * m[3][1]);
    
    float m01 = m[1][0] * (m[2][2] * m[3][3] - m[2][3] * m[3][2]) -
                m[1][2] * (m[2][0] * m[3][3] - m[2][3] * m[3][0]) +
                m[1][3] * (m[2][0] * m[3][2] - m[2][2] * m[3][0]);
    
    float m02 = m[1][0] * (m[2][1] * m[3][3] - m[2][3] * m[3][1]) -
                m[1][1] * (m[2][0] * m[3][3] - m[2][3] * m[3][0]) +
                m[1][3] * (m[2][0] * m[3][1] - m[2][1] * m[3][0]);
    
    float m03 = m[1][0] * (m[2][1] * m[3][2] - m[2][2] * m[3][1]) -
                m[1][1] * (m[2][0] * m[3][2] - m[2][2] * m[3][0]) +
                m[1][2] * (m[2][0] * m[3][1] - m[2][1] * m[3][0]);
    
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
                if (row == i) continue;
                mj = 0;
                for (int col = 0; col < 4; ++col) {
                    if (col == j) continue;
                    minor[mi][mj] = m[row][col];
                    mj++;
                }
                mi++;
            }
            
            // Вычисляем определитель минора 3x3
            float det3 = minor[0][0] * (minor[1][1] * minor[2][2] - minor[1][2] * minor[2][1]) -
                        minor[0][1] * (minor[1][0] * minor[2][2] - minor[1][2] * minor[2][0]) +
                        minor[0][2] * (minor[1][0] * minor[2][1] - minor[1][1] * minor[2][0]);
            
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
Matrix4 Matrix4::translation(const Vector4& translation) {
    return translation(translation.x, translation.y, translation.z, translation.w);
}

Matrix4 Matrix4::translation(float x, float y, float z, float w) {
    Matrix4 result;
    result.setIdentity();
    result.m[0][3] = x;
    result.m[1][3] = y;
    result.m[2][3] = z;
    result.m[3][3] = w;
    return result;
}

Matrix4 Matrix4::scaling(const Vector4& scale) {
    return scaling(scale.x, scale.y, scale.z, scale.w);
}

Matrix4 Matrix4::scaling(float x, float y, float z, float w) {
    Matrix4 result;
    result.setZero();
    result.m[0][0] = x;
    result.m[1][1] = y;
    result.m[2][2] = z;
    result.m[3][3] = w;
    return result;
}

Matrix4 Matrix4::scaling(float uniformScale) {
    return scaling(uniformScale, uniformScale, uniformScale, uniformScale);
}

// Повороты в 4D
Matrix4 Matrix4::rotationXY(float angle) {
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

Matrix4 Matrix4::rotationXZ(float angle) {
    Matrix4 result;
    result.setIdentity();
    float c = std::cos(angle);
    float s = std::sin(angle);
    result.m[0][0] = c;
    result.m[0][2] = -s;
    result.m[2][0] = s;
    result.m[2][2] = c;
    return result;
}

Matrix4 Matrix4::rotationXW(float angle) {
    Matrix4 result;
    result.setIdentity();
    float c = std::cos(angle);
    float s = std::sin(angle);
    result.m[0][0] = c;
    result.m[0][3] = -s;
    result.m[3][0] = s;
    result.m[3][3] = c;
    return result;
}

Matrix4 Matrix4::rotationYZ(float angle) {
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

Matrix4 Matrix4::rotationYW(float angle) {
    Matrix4 result;
    result.setIdentity();
    float c = std::cos(angle);
    float s = std::sin(angle);
    result.m[1][1] = c;
    result.m[1][3] = -s;
    result.m[3][1] = s;
    result.m[3][3] = c;
    return result;
}

Matrix4 Matrix4::rotationZW(float angle) {
    Matrix4 result;
    result.setIdentity();
    float c = std::cos(angle);
    float s = std::sin(angle);
    result.m[2][2] = c;
    result.m[2][3] = -s;
    result.m[3][2] = s;
    result.m[3][3] = c;
    return result;
}

// Проекции
Matrix4 Matrix4::orthographicProjection() {
    // Ортогональная проекция 4D -> 3D (отбрасываем w)
    Matrix4 result;
    result.setIdentity();
    result.m[3][3] = 0.0f; // Обнуляем w-координату
    return result;
}

Matrix4 Matrix4::perspectiveProjection(float distance) {
    // Перспективная проекция 4D -> 3D
    Matrix4 result;
    result.setIdentity();
    result.m[0][3] = 0.0f;
    result.m[1][3] = 0.0f;
    result.m[2][3] = 0.0f;
    result.m[3][0] = 0.0f;
    result.m[3][1] = 0.0f;
    result.m[3][2] = 0.0f;
    result.m[3][3] = 1.0f / distance;
    return result;
}

Matrix4 Matrix4::crossSectionProjection(float wValue) {
    // Проекция сечения при заданном w
    Matrix4 result;
    result.setIdentity();
    result.m[3][3] = 0.0f; // Обнуляем w-координату
    return result;
}

// Утилиты
Vector4 Matrix4::transformPoint(const Vector4& point) const {
    return *this * point;
}

Vector4 Matrix4::transformDirection(const Vector4& direction) const {
    // Для направлений не применяем трансляцию
    Matrix4 rotationMatrix = *this;
    rotationMatrix.m[0][3] = 0.0f;
    rotationMatrix.m[1][3] = 0.0f;
    rotationMatrix.m[2][3] = 0.0f;
    rotationMatrix.m[3][3] = 1.0f;
    return rotationMatrix * direction;
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
std::ostream& operator<<(std::ostream& os, const Matrix4& mat) {
    os << "Matrix4:\n";
    for (int i = 0; i < 4; ++i) {
        os << "[";
        for (int j = 0; j < 4; ++j) {
            os << mat.m[i][j];
            if (j < 3) os << ", ";
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

} // namespace Math
} // namespace Engine4D
