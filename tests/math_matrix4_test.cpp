/**
 * @file math_matrix4_test.cpp
 * @brief Комплексные тесты для класса Matrix4
 *
 * Покрытие: конструкторы, операторы, трансформации, проекции
 */

#include <gtest/gtest.h>
#include <SpectraForge/Math/Matrix4.h>
#include <SpectraForge/Math/Vector3.h>
#include <SpectraForge/Math/Vector4.h>
#include <SpectraForge/Math/Quaternion.h>
#include <cmath>

using namespace SpectraForge::Math;

class Matrix4Test : public ::testing::Test {
protected:
    void SetUp() override {
        identity = Matrix4::identity();
        zero_mat = Matrix4::zero();
        epsilon = 0.0001f;
    }

    bool nearlyEqual(float a, float b) {
        return std::abs(a - b) < epsilon;
    }

    bool matricesEqual(const Matrix4& a, const Matrix4& b) {
        for (int i = 0; i < 4; ++i) {
            for (int j = 0; j < 4; ++j) {
                if (!nearlyEqual(a.m[i][j], b.m[i][j])) {
                    return false;
                }
            }
        }
        return true;
    }

    Matrix4 identity, zero_mat;
    float epsilon;
};

// ============================================================================
// Конструкторы
// ============================================================================

TEST_F(Matrix4Test, DefaultConstructor) {
    // Arrange & Act
    Matrix4 mat;
    
    // Assert - должна быть единичная матрица
    EXPECT_TRUE(matricesEqual(mat, identity));
}

TEST_F(Matrix4Test, DiagonalConstructor) {
    // Arrange & Act
    Matrix4 mat(2.0f);
    
    // Assert
    for (int i = 0; i < 4; ++i) {
        for (int j = 0; j < 4; ++j) {
            if (i == j) {
                EXPECT_FLOAT_EQ(mat.m[i][j], 2.0f);
            } else {
                EXPECT_FLOAT_EQ(mat.m[i][j], 0.0f);
            }
        }
    }
}

TEST_F(Matrix4Test, CopyConstructor) {
    // Arrange
    Matrix4 original = Matrix4::translation(1.0f, 2.0f, 3.0f);
    
    // Act
    Matrix4 copy(original);
    
    // Assert
    EXPECT_TRUE(matricesEqual(copy, original));
}

TEST_F(Matrix4Test, ParameterConstructor16Values) {
    // Arrange & Act
    Matrix4 mat(1, 2, 3, 4,
                5, 6, 7, 8,
                9, 10, 11, 12,
                13, 14, 15, 16);
    
    // Assert
    EXPECT_FLOAT_EQ(mat.m[0][0], 1.0f);
    EXPECT_FLOAT_EQ(mat.m[0][3], 4.0f);
    EXPECT_FLOAT_EQ(mat.m[3][3], 16.0f);
}

// ============================================================================
// Операторы
// ============================================================================

TEST_F(Matrix4Test, AssignmentOperator) {
    // Arrange
    Matrix4 mat = Matrix4::translation(1, 2, 3);
    Matrix4 other;
    
    // Act
    other = mat;
    
    // Assert
    EXPECT_TRUE(matricesEqual(other, mat));
}

TEST_F(Matrix4Test, MatrixAddition) {
    // Arrange
    Matrix4 a(1.0f);
    Matrix4 b(2.0f);
    
    // Act
    Matrix4 result = a + b;
    
    // Assert
    for (int i = 0; i < 4; ++i) {
        EXPECT_FLOAT_EQ(result.m[i][i], 3.0f);
    }
}

TEST_F(Matrix4Test, MatrixSubtraction) {
    // Arrange
    Matrix4 a(3.0f);
    Matrix4 b(1.0f);
    
    // Act
    Matrix4 result = a - b;
    
    // Assert
    for (int i = 0; i < 4; ++i) {
        EXPECT_FLOAT_EQ(result.m[i][i], 2.0f);
    }
}

TEST_F(Matrix4Test, MatrixMultiplication) {
    // Arrange
    Matrix4 a = Matrix4::scaling(2.0f, 2.0f, 2.0f);
    Matrix4 b = Matrix4::translation(1.0f, 2.0f, 3.0f);
    
    // Act
    Matrix4 result = a * b;
    
    // Assert - умножение должно дать комбинированную трансформацию
    EXPECT_NE(result.m[0][3], 0.0f);  // Есть трансляция
}

TEST_F(Matrix4Test, MatrixMultiplicationByIdentity) {
    // Arrange
    Matrix4 mat = Matrix4::translation(1, 2, 3);
    
    // Act
    Matrix4 result = mat * identity;
    
    // Assert
    EXPECT_TRUE(matricesEqual(result, mat));
}

TEST_F(Matrix4Test, ScalarMultiplication) {
    // Arrange
    Matrix4 mat(2.0f);
    
    // Act
    Matrix4 result = mat * 3.0f;
    
    // Assert
    for (int i = 0; i < 4; ++i) {
        EXPECT_FLOAT_EQ(result.m[i][i], 6.0f);
    }
}

TEST_F(Matrix4Test, VectorMultiplication) {
    // Arrange
    Matrix4 mat = Matrix4::scaling(2.0f, 3.0f, 4.0f);
    Vector3 vec(1.0f, 1.0f, 1.0f);
    
    // Act
    Vector3 result = mat * vec;
    
    // Assert
    EXPECT_FLOAT_EQ(result.x, 2.0f);
    EXPECT_FLOAT_EQ(result.y, 3.0f);
    EXPECT_FLOAT_EQ(result.z, 4.0f);
}

TEST_F(Matrix4Test, EqualityOperator) {
    // Arrange
    Matrix4 a = Matrix4::translation(1, 2, 3);
    Matrix4 b = Matrix4::translation(1, 2, 3);
    
    // Act & Assert
    EXPECT_TRUE(a == b);
}

TEST_F(Matrix4Test, InequalityOperator) {
    // Arrange
    Matrix4 a = Matrix4::translation(1, 2, 3);
    Matrix4 b = Matrix4::translation(4, 5, 6);
    
    // Act & Assert
    EXPECT_TRUE(a != b);
}

TEST_F(Matrix4Test, IndexOperator) {
    // Arrange
    Matrix4 mat;
    
    // Act
    mat(1, 2) = 5.0f;
    
    // Assert
    EXPECT_FLOAT_EQ(mat.m[1][2], 5.0f);
    EXPECT_FLOAT_EQ(mat(1, 2), 5.0f);
}

// ============================================================================
// Основные операции
// ============================================================================

TEST_F(Matrix4Test, Transpose) {
    // Arrange
    Matrix4 mat(1, 2, 3, 4,
                5, 6, 7, 8,
                9, 10, 11, 12,
                13, 14, 15, 16);
    
    // Act
    Matrix4 result = mat.transpose();
    
    // Assert - после транспонирования m[i][j] -> m[j][i]
    EXPECT_FLOAT_EQ(result.m[0][1], 5.0f);   // было m[1][0]
    EXPECT_FLOAT_EQ(result.m[1][0], 2.0f);   // было m[0][1]
    EXPECT_FLOAT_EQ(result.m[3][2], 12.0f);  // было m[2][3]
}

TEST_F(Matrix4Test, TransposeIdentity) {
    // Arrange & Act
    Matrix4 result = identity.transpose();
    
    // Assert
    EXPECT_TRUE(matricesEqual(result, identity));
}

TEST_F(Matrix4Test, DeterminantIdentity) {
    // Arrange & Act
    float det = identity.determinant();
    
    // Assert
    EXPECT_FLOAT_EQ(det, 1.0f);
}

TEST_F(Matrix4Test, DeterminantZero) {
    // Arrange
    Matrix4 singular;
    singular.m[0][0] = 0.0f;  // Делаем матрицу вырожденной
    
    // Act
    float det = singular.determinant();
    
    // Assert
    EXPECT_FLOAT_EQ(det, 0.0f);
}

TEST_F(Matrix4Test, InverseIdentity) {
    // Arrange & Act
    Matrix4 inv = identity.inverse();
    
    // Assert
    EXPECT_TRUE(matricesEqual(inv, identity));
}

TEST_F(Matrix4Test, InverseTranslation) {
    // Arrange
    Matrix4 trans = Matrix4::translation(1.0f, 2.0f, 3.0f);
    
    // Act
    Matrix4 inv = trans.inverse();
    Matrix4 product = trans * inv;
    
    // Assert - произведение должно давать единичную матрицу
    EXPECT_TRUE(matricesEqual(product, identity));
}

TEST_F(Matrix4Test, InverseScaling) {
    // Arrange
    Matrix4 scale = Matrix4::scaling(2.0f, 3.0f, 4.0f);
    
    // Act
    Matrix4 inv = scale.inverse();
    Matrix4 product = scale * inv;
    
    // Assert
    EXPECT_TRUE(matricesEqual(product, identity));
}

TEST_F(Matrix4Test, IsInvertible) {
    // Arrange & Act & Assert
    EXPECT_TRUE(identity.isInvertible());
    
    Matrix4 singular;
    singular.m[0][0] = 0.0f;
    EXPECT_FALSE(singular.isInvertible());
}

TEST_F(Matrix4Test, SetIdentity) {
    // Arrange
    Matrix4 mat = Matrix4::scaling(2, 3, 4);
    
    // Act
    mat.setIdentity();
    
    // Assert
    EXPECT_TRUE(matricesEqual(mat, identity));
}

TEST_F(Matrix4Test, SetZero) {
    // Arrange
    Matrix4 mat = identity;
    
    // Act
    mat.setZero();
    
    // Assert
    for (int i = 0; i < 4; ++i) {
        for (int j = 0; j < 4; ++j) {
            EXPECT_FLOAT_EQ(mat.m[i][j], 0.0f);
        }
    }
}

// ============================================================================
// Трансформации
// ============================================================================

TEST_F(Matrix4Test, TranslationVector) {
    // Arrange
    Vector3 trans(1.0f, 2.0f, 3.0f);
    
    // Act
    Matrix4 mat = Matrix4::translation(trans);
    
    // Assert
    EXPECT_FLOAT_EQ(mat.m[0][3], 1.0f);
    EXPECT_FLOAT_EQ(mat.m[1][3], 2.0f);
    EXPECT_FLOAT_EQ(mat.m[2][3], 3.0f);
}

TEST_F(Matrix4Test, TranslationComponents) {
    // Arrange & Act
    Matrix4 mat = Matrix4::translation(1.0f, 2.0f, 3.0f);
    
    // Assert
    EXPECT_FLOAT_EQ(mat.m[0][3], 1.0f);
    EXPECT_FLOAT_EQ(mat.m[1][3], 2.0f);
    EXPECT_FLOAT_EQ(mat.m[2][3], 3.0f);
}

TEST_F(Matrix4Test, ScalingVector) {
    // Arrange
    Vector3 scale(2.0f, 3.0f, 4.0f);
    
    // Act
    Matrix4 mat = Matrix4::scaling(scale);
    
    // Assert
    EXPECT_FLOAT_EQ(mat.m[0][0], 2.0f);
    EXPECT_FLOAT_EQ(mat.m[1][1], 3.0f);
    EXPECT_FLOAT_EQ(mat.m[2][2], 4.0f);
}

TEST_F(Matrix4Test, ScalingUniform) {
    // Arrange & Act
    Matrix4 mat = Matrix4::scaling(2.0f);
    
    // Assert
    EXPECT_FLOAT_EQ(mat.m[0][0], 2.0f);
    EXPECT_FLOAT_EQ(mat.m[1][1], 2.0f);
    EXPECT_FLOAT_EQ(mat.m[2][2], 2.0f);
}

TEST_F(Matrix4Test, RotationX) {
    // Arrange
    float angle = M_PI / 2.0f;  // 90 градусов
    
    // Act
    Matrix4 mat = Matrix4::rotationX(angle);
    Vector3 point(0.0f, 1.0f, 0.0f);
    Vector3 rotated = mat * point;
    
    // Assert - Y поворачивается в Z
    EXPECT_TRUE(nearlyEqual(rotated.y, 0.0f));
    EXPECT_TRUE(nearlyEqual(rotated.z, 1.0f));
}

TEST_F(Matrix4Test, RotationY) {
    // Arrange
    float angle = M_PI / 2.0f;
    
    // Act
    Matrix4 mat = Matrix4::rotationY(angle);
    Vector3 point(1.0f, 0.0f, 0.0f);
    Vector3 rotated = mat * point;
    
    // Assert - X поворачивается в -Z
    EXPECT_TRUE(nearlyEqual(rotated.x, 0.0f));
    EXPECT_TRUE(nearlyEqual(rotated.z, -1.0f));
}

TEST_F(Matrix4Test, RotationZ) {
    // Arrange
    float angle = M_PI / 2.0f;
    
    // Act
    Matrix4 mat = Matrix4::rotationZ(angle);
    Vector3 point(1.0f, 0.0f, 0.0f);
    Vector3 rotated = mat * point;
    
    // Assert - X поворачивается в Y
    EXPECT_TRUE(nearlyEqual(rotated.x, 0.0f));
    EXPECT_TRUE(nearlyEqual(rotated.y, 1.0f));
}

TEST_F(Matrix4Test, RotationArbitraryAxis) {
    // Arrange
    Vector3 axis(0.0f, 1.0f, 0.0f);  // Y axis
    float angle = M_PI / 2.0f;
    
    // Act
    Matrix4 mat = Matrix4::rotation(axis, angle);
    Vector3 point(1.0f, 0.0f, 0.0f);
    Vector3 rotated = mat * point;
    
    // Assert - должен дать тот же результат, что rotationY
    EXPECT_TRUE(nearlyEqual(rotated.x, 0.0f));
}

TEST_F(Matrix4Test, EulerAngles) {
    // Arrange
    float pitch = 0.0f;
    float yaw = M_PI / 2.0f;
    float roll = 0.0f;
    
    // Act
    Matrix4 mat = Matrix4::eulerAngles(pitch, yaw, roll);
    
    // Assert - матрица должна быть создана без ошибок
    EXPECT_FALSE(std::isnan(mat.m[0][0]));
}

// ============================================================================
// Камера и проекции
// ============================================================================

TEST_F(Matrix4Test, LookAt) {
    // Arrange
    Vector3 eye(0.0f, 0.0f, 5.0f);
    Vector3 target(0.0f, 0.0f, 0.0f);
    Vector3 up(0.0f, 1.0f, 0.0f);
    
    // Act
    Matrix4 view = Matrix4::lookAt(eye, target, up);
    
    // Assert - матрица должна быть создана
    EXPECT_FALSE(std::isnan(view.m[0][0]));
}

TEST_F(Matrix4Test, PerspectiveProjection) {
    // Arrange
    float fovY = M_PI / 4.0f;  // 45 градусов
    float aspect = 16.0f / 9.0f;
    float near = 0.1f;
    float far = 100.0f;
    
    // Act
    Matrix4 proj = Matrix4::perspective(fovY, aspect, near, far);
    
    // Assert
    EXPECT_FALSE(std::isnan(proj.m[0][0]));
    EXPECT_NE(proj.m[0][0], 0.0f);
}

TEST_F(Matrix4Test, OrthographicProjection) {
    // Arrange & Act
    Matrix4 proj = Matrix4::orthographic(-1.0f, 1.0f, -1.0f, 1.0f, 0.1f, 100.0f);
    
    // Assert
    EXPECT_FALSE(std::isnan(proj.m[0][0]));
}

// ============================================================================
// Утилиты
// ============================================================================

TEST_F(Matrix4Test, TransformPoint) {
    // Arrange
    Matrix4 mat = Matrix4::translation(1.0f, 2.0f, 3.0f);
    Vector3 point(0.0f, 0.0f, 0.0f);
    
    // Act
    Vector3 result = mat.transformPoint(point);
    
    // Assert
    EXPECT_FLOAT_EQ(result.x, 1.0f);
    EXPECT_FLOAT_EQ(result.y, 2.0f);
    EXPECT_FLOAT_EQ(result.z, 3.0f);
}

TEST_F(Matrix4Test, TransformDirection) {
    // Arrange
    Matrix4 mat = Matrix4::scaling(2.0f, 2.0f, 2.0f);
    Vector3 direction(1.0f, 0.0f, 0.0f);
    
    // Act
    Vector3 result = mat.transformDirection(direction);
    
    // Assert - направление должно остаться нормализованным
    EXPECT_TRUE(nearlyEqual(result.magnitude(), 1.0f));
}

TEST_F(Matrix4Test, GetTranslation) {
    // Arrange
    Matrix4 mat = Matrix4::translation(1.0f, 2.0f, 3.0f);
    
    // Act
    Vector3 trans = mat.getTranslation();
    
    // Assert
    EXPECT_FLOAT_EQ(trans.x, 1.0f);
    EXPECT_FLOAT_EQ(trans.y, 2.0f);
    EXPECT_FLOAT_EQ(trans.z, 3.0f);
}

TEST_F(Matrix4Test, GetScale) {
    // Arrange
    Matrix4 mat = Matrix4::scaling(2.0f, 3.0f, 4.0f);
    
    // Act
    Vector3 scale = mat.getScale();
    
    // Assert
    EXPECT_TRUE(nearlyEqual(scale.x, 2.0f));
    EXPECT_TRUE(nearlyEqual(scale.y, 3.0f));
    EXPECT_TRUE(nearlyEqual(scale.z, 4.0f));
}

TEST_F(Matrix4Test, Decompose) {
    // Arrange
    Matrix4 mat = Matrix4::translation(1, 2, 3) * Matrix4::scaling(2, 3, 4);
    Vector3 trans, scale;
    Quaternion rot;
    
    // Act
    mat.decompose(trans, rot, scale);
    
    // Assert
    EXPECT_TRUE(nearlyEqual(trans.x, 1.0f));
    EXPECT_TRUE(nearlyEqual(scale.x, 2.0f));
}

// ============================================================================
// Граничные случаи
// ============================================================================

TEST_F(Matrix4Test, CombinedTransformations) {
    // Arrange
    Matrix4 translate = Matrix4::translation(1, 2, 3);
    Matrix4 rotate = Matrix4::rotationY(M_PI / 4.0f);
    Matrix4 scale = Matrix4::scaling(2, 2, 2);
    
    // Act
    Matrix4 combined = translate * rotate * scale;
    Vector3 point(1, 0, 0);
    Vector3 result = combined * point;
    
    // Assert - результат должен быть валидным
    EXPECT_FALSE(std::isnan(result.x));
    EXPECT_FALSE(std::isnan(result.y));
    EXPECT_FALSE(std::isnan(result.z));
}

TEST_F(Matrix4Test, DataAccess) {
    // Arrange
    Matrix4 mat = Matrix4::translation(1, 2, 3);
    
    // Act
    const float* data = mat.data();
    
    // Assert
    EXPECT_NE(data, nullptr);
    EXPECT_FLOAT_EQ(data[3], 1.0f);  // m[0][3]
}

int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
