/**
 * @file math_quaternion_test.cpp
 * @brief Комплексные тесты для класса Quaternion
 */

#include <gtest/gtest.h>
#include <SpectraForge/Math/Quaternion.h>
#include <SpectraForge/Math/Vector3.h>
#include <SpectraForge/Math/Matrix4.h>
#include <cmath>

using namespace SpectraForge::Math;

class QuaternionTest : public ::testing::Test {
protected:
    void SetUp() override {
        identity = Quaternion::identity();
        epsilon = 0.001f;
    }

    bool nearlyEqual(float a, float b) {
        return std::abs(a - b) < epsilon;
    }

    bool quaternionsEqual(const Quaternion& a, const Quaternion& b) {
        return nearlyEqual(a.w, b.w) &&
               nearlyEqual(a.x, b.x) &&
               nearlyEqual(a.y, b.y) &&
               nearlyEqual(a.z, b.z);
    }

    Quaternion identity;
    float epsilon;
};

// ============================================================================
// Конструкторы
// ============================================================================

TEST_F(QuaternionTest, DefaultConstructor) {
    // Arrange & Act
    Quaternion q;
    
    // Assert - должен быть identity
    EXPECT_TRUE(quaternionsEqual(q, identity));
}

TEST_F(QuaternionTest, ParameterConstructor) {
    // Arrange & Act
    Quaternion q(1.0f, 2.0f, 3.0f, 4.0f);
    
    // Assert
    EXPECT_FLOAT_EQ(q.w, 1.0f);
    EXPECT_FLOAT_EQ(q.x, 2.0f);
    EXPECT_FLOAT_EQ(q.y, 3.0f);
    EXPECT_FLOAT_EQ(q.z, 4.0f);
}

TEST_F(QuaternionTest, AxisAngleConstructor) {
    // Arrange
    Vector3 axis(0.0f, 1.0f, 0.0f);  // Y axis
    float angle = M_PI / 2.0f;  // 90 degrees
    
    // Act
    Quaternion q(axis, angle);
    
    // Assert - кватернион должен быть нормализован
    EXPECT_TRUE(nearlyEqual(q.magnitude(), 1.0f));
}

TEST_F(QuaternionTest, EulerAnglesConstructor) {
    // Arrange
    Vector3 euler(0.0f, M_PI / 2.0f, 0.0f);
    
    // Act
    Quaternion q(euler);
    
    // Assert
    EXPECT_TRUE(nearlyEqual(q.magnitude(), 1.0f));
}

// ============================================================================
// Операторы
// ============================================================================

TEST_F(QuaternionTest, Addition) {
    // Arrange
    Quaternion q1(1, 2, 3, 4);
    Quaternion q2(5, 6, 7, 8);
    
    // Act
    Quaternion result = q1 + q2;
    
    // Assert
    EXPECT_FLOAT_EQ(result.w, 6.0f);
    EXPECT_FLOAT_EQ(result.x, 8.0f);
    EXPECT_FLOAT_EQ(result.y, 10.0f);
    EXPECT_FLOAT_EQ(result.z, 12.0f);
}

TEST_F(QuaternionTest, Subtraction) {
    // Arrange
    Quaternion q1(5, 6, 7, 8);
    Quaternion q2(1, 2, 3, 4);
    
    // Act
    Quaternion result = q1 - q2;
    
    // Assert
    EXPECT_FLOAT_EQ(result.w, 4.0f);
    EXPECT_FLOAT_EQ(result.x, 4.0f);
    EXPECT_FLOAT_EQ(result.y, 4.0f);
    EXPECT_FLOAT_EQ(result.z, 4.0f);
}

TEST_F(QuaternionTest, MultiplicationComposition) {
    // Arrange - два поворота на 90 градусов вокруг Y
    Quaternion q1 = Quaternion::fromAxisAngle(Vector3::unitY(), M_PI / 2.0f);
    Quaternion q2 = Quaternion::fromAxisAngle(Vector3::unitY(), M_PI / 2.0f);
    
    // Act
    Quaternion result = q1 * q2;
    
    // Assert - должен получиться поворот на 180 градусов
    EXPECT_TRUE(nearlyEqual(result.angle(), M_PI));
}

TEST_F(QuaternionTest, ScalarMultiplication) {
    // Arrange
    Quaternion q(1, 2, 3, 4);
    
    // Act
    Quaternion result = q * 2.0f;
    
    // Assert
    EXPECT_FLOAT_EQ(result.w, 2.0f);
    EXPECT_FLOAT_EQ(result.x, 4.0f);
    EXPECT_FLOAT_EQ(result.y, 6.0f);
    EXPECT_FLOAT_EQ(result.z, 8.0f);
}

TEST_F(QuaternionTest, Division) {
    // Arrange
    Quaternion q(2, 4, 6, 8);
    
    // Act
    Quaternion result = q / 2.0f;
    
    // Assert
    EXPECT_FLOAT_EQ(result.w, 1.0f);
    EXPECT_FLOAT_EQ(result.x, 2.0f);
    EXPECT_FLOAT_EQ(result.y, 3.0f);
    EXPECT_FLOAT_EQ(result.z, 4.0f);
}

TEST_F(QuaternionTest, EqualityOperator) {
    // Arrange
    Quaternion q1(1, 2, 3, 4);
    Quaternion q2(1, 2, 3, 4);
    
    // Act & Assert
    EXPECT_TRUE(q1 == q2);
}

TEST_F(QuaternionTest, InequalityOperator) {
    // Arrange
    Quaternion q1(1, 2, 3, 4);
    Quaternion q2(5, 6, 7, 8);
    
    // Act & Assert
    EXPECT_TRUE(q1 != q2);
}

// ============================================================================
// Основные операции
// ============================================================================

TEST_F(QuaternionTest, Magnitude) {
    // Arrange
    Quaternion q(1, 0, 0, 0);
    
    // Act
    float mag = q.magnitude();
    
    // Assert
    EXPECT_FLOAT_EQ(mag, 1.0f);
}

TEST_F(QuaternionTest, MagnitudeSquared) {
    // Arrange
    Quaternion q(1, 1, 1, 1);
    
    // Act
    float magSq = q.magnitudeSquared();
    
    // Assert
    EXPECT_FLOAT_EQ(magSq, 4.0f);
}

TEST_F(QuaternionTest, Normalize) {
    // Arrange
    Quaternion q(1, 2, 3, 4);
    
    // Act
    q.normalize();
    
    // Assert
    EXPECT_TRUE(nearlyEqual(q.magnitude(), 1.0f));
}

TEST_F(QuaternionTest, Normalized) {
    // Arrange
    Quaternion q(1, 2, 3, 4);
    float originalMag = q.magnitude();
    
    // Act
    Quaternion normalized = q.normalized();
    
    // Assert
    EXPECT_TRUE(nearlyEqual(normalized.magnitude(), 1.0f));
    EXPECT_FLOAT_EQ(q.magnitude(), originalMag);  // Оригинал не изменился
}

TEST_F(QuaternionTest, Conjugate) {
    // Arrange
    Quaternion q(1, 2, 3, 4);
    
    // Act
    Quaternion conj = q.conjugate();
    
    // Assert
    EXPECT_FLOAT_EQ(conj.w, 1.0f);
    EXPECT_FLOAT_EQ(conj.x, -2.0f);
    EXPECT_FLOAT_EQ(conj.y, -3.0f);
    EXPECT_FLOAT_EQ(conj.z, -4.0f);
}

TEST_F(QuaternionTest, Inverse) {
    // Arrange
    Quaternion q = Quaternion::fromAxisAngle(Vector3::unitY(), M_PI / 4.0f);
    
    // Act
    Quaternion inv = q.inverse();
    Quaternion product = q * inv;
    
    // Assert - произведение должно дать identity
    EXPECT_TRUE(quaternionsEqual(product, identity));
}

TEST_F(QuaternionTest, DotProduct) {
    // Arrange
    Quaternion q1(1, 2, 3, 4);
    Quaternion q2(5, 6, 7, 8);
    
    // Act
    float dot = q1.dot(q2);
    
    // Assert
    // 1*5 + 2*6 + 3*7 + 4*8 = 5 + 12 + 21 + 32 = 70
    EXPECT_FLOAT_EQ(dot, 70.0f);
}

// ============================================================================
// Повороты
// ============================================================================

TEST_F(QuaternionTest, RotateVector) {
    // Arrange
    Quaternion q = Quaternion::fromAxisAngle(Vector3::unitY(), M_PI / 2.0f);
    Vector3 v(1, 0, 0);  // X axis
    
    // Act
    Vector3 rotated = q.rotate(v);
    
    // Assert - X должен повернуться в -Z
    EXPECT_TRUE(nearlyEqual(rotated.x, 0.0f));
    EXPECT_TRUE(nearlyEqual(rotated.z, -1.0f));
}

TEST_F(QuaternionTest, ToMatrix) {
    // Arrange
    Quaternion q = Quaternion::fromAxisAngle(Vector3::unitY(), M_PI / 2.0f);
    
    // Act
    Matrix4 mat = q.toMatrix();
    Vector3 v(1, 0, 0);
    Vector3 rotated = mat * v;
    
    // Assert
    EXPECT_TRUE(nearlyEqual(rotated.x, 0.0f));
}

TEST_F(QuaternionTest, ToEulerAngles) {
    // Arrange
    Quaternion q = Quaternion::fromAxisAngle(Vector3::unitY(), M_PI / 2.0f);
    
    // Act
    Vector3 euler = q.toEulerAngles();
    
    // Assert - должен быть ненулевой yaw
    EXPECT_FALSE(nearlyEqual(euler.y, 0.0f));
}

TEST_F(QuaternionTest, ToAxisAngle) {
    // Arrange
    Vector3 originalAxis(0, 1, 0);
    float originalAngle = M_PI / 2.0f;
    Quaternion q = Quaternion::fromAxisAngle(originalAxis, originalAngle);
    
    // Act
    Vector3 axis;
    float angle;
    q.toAxisAngle(axis, angle);
    
    // Assert
    EXPECT_TRUE(nearlyEqual(angle, originalAngle));
    EXPECT_TRUE(nearlyEqual(axis.y, 1.0f));
}

// ============================================================================
// Создание поворотов
// ============================================================================

TEST_F(QuaternionTest, FromAxisAngle) {
    // Arrange & Act
    Quaternion q = Quaternion::fromAxisAngle(Vector3::unitZ(), M_PI);
    
    // Assert
    EXPECT_TRUE(nearlyEqual(q.magnitude(), 1.0f));
}

TEST_F(QuaternionTest, FromEulerAngles) {
    // Arrange & Act
    Quaternion q = Quaternion::fromEulerAngles(0.0f, M_PI / 2.0f, 0.0f);
    
    // Assert
    EXPECT_TRUE(nearlyEqual(q.magnitude(), 1.0f));
}

TEST_F(QuaternionTest, FromMatrix) {
    // Arrange
    Matrix4 mat = Matrix4::rotationY(M_PI / 2.0f);
    
    // Act
    Quaternion q = Quaternion::fromMatrix(mat);
    
    // Assert
    EXPECT_TRUE(nearlyEqual(q.magnitude(), 1.0f));
}

TEST_F(QuaternionTest, LookRotation) {
    // Arrange
    Vector3 forward(0, 0, -1);
    Vector3 up(0, 1, 0);
    
    // Act
    Quaternion q = Quaternion::lookRotation(forward, up);
    
    // Assert
    EXPECT_TRUE(nearlyEqual(q.magnitude(), 1.0f));
}

// ============================================================================
// Интерполяция
// ============================================================================

TEST_F(QuaternionTest, Slerp) {
    // Arrange
    Quaternion q1 = Quaternion::identity();
    Quaternion q2 = Quaternion::fromAxisAngle(Vector3::unitY(), M_PI);
    
    // Act
    Quaternion mid = q1.slerp(q2, 0.5f);
    
    // Assert
    EXPECT_TRUE(nearlyEqual(mid.magnitude(), 1.0f));
}

TEST_F(QuaternionTest, SlerpBoundaries) {
    // Arrange
    Quaternion q1 = Quaternion::identity();
    Quaternion q2 = Quaternion::fromAxisAngle(Vector3::unitY(), M_PI / 2.0f);
    
    // Act
    Quaternion at0 = q1.slerp(q2, 0.0f);
    Quaternion at1 = q1.slerp(q2, 1.0f);
    
    // Assert
    EXPECT_TRUE(quaternionsEqual(at0, q1));
    EXPECT_TRUE(quaternionsEqual(at1, q2));
}

TEST_F(QuaternionTest, Lerp) {
    // Arrange
    Quaternion q1 = Quaternion::identity();
    Quaternion q2 = Quaternion::fromAxisAngle(Vector3::unitY(), M_PI);
    
    // Act
    Quaternion mid = q1.lerp(q2, 0.5f);
    
    // Assert - lerp не гарантирует нормализацию
    EXPECT_GT(mid.magnitude(), 0.0f);
}

TEST_F(QuaternionTest, Nlerp) {
    // Arrange
    Quaternion q1 = Quaternion::identity();
    Quaternion q2 = Quaternion::fromAxisAngle(Vector3::unitY(), M_PI);
    
    // Act
    Quaternion mid = q1.nlerp(q2, 0.5f);
    
    // Assert - nlerp должен давать нормализованный результат
    EXPECT_TRUE(nearlyEqual(mid.magnitude(), 1.0f));
}

// ============================================================================
// Утилиты
// ============================================================================

TEST_F(QuaternionTest, SetIdentity) {
    // Arrange
    Quaternion q(1, 2, 3, 4);
    
    // Act
    q.setIdentity();
    
    // Assert
    EXPECT_TRUE(quaternionsEqual(q, identity));
}

TEST_F(QuaternionTest, IsIdentity) {
    // Arrange & Act & Assert
    EXPECT_TRUE(identity.isIdentity());
    
    Quaternion notIdentity(0, 1, 0, 0);
    EXPECT_FALSE(notIdentity.isIdentity());
}

TEST_F(QuaternionTest, IsZero) {
    // Arrange
    Quaternion zero = Quaternion::zero();
    
    // Act & Assert
    EXPECT_TRUE(zero.isZero());
    EXPECT_FALSE(identity.isZero());
}

TEST_F(QuaternionTest, SetMethod) {
    // Arrange
    Quaternion q;
    
    // Act
    q.set(1, 2, 3, 4);
    
    // Assert
    EXPECT_FLOAT_EQ(q.w, 1.0f);
    EXPECT_FLOAT_EQ(q.x, 2.0f);
    EXPECT_FLOAT_EQ(q.y, 3.0f);
    EXPECT_FLOAT_EQ(q.z, 4.0f);
}

TEST_F(QuaternionTest, AngleExtraction) {
    // Arrange
    float originalAngle = M_PI / 3.0f;
    Quaternion q = Quaternion::fromAxisAngle(Vector3::unitY(), originalAngle);
    
    // Act
    float angle = q.angle();
    
    // Assert
    EXPECT_TRUE(nearlyEqual(angle, originalAngle));
}

TEST_F(QuaternionTest, AxisExtraction) {
    // Arrange
    Vector3 originalAxis(0, 1, 0);
    Quaternion q = Quaternion::fromAxisAngle(originalAxis, M_PI / 2.0f);
    
    // Act
    Vector3 axis = q.axis();
    
    // Assert
    EXPECT_TRUE(nearlyEqual(axis.y, 1.0f));
}

// ============================================================================
// Граничные случаи
// ============================================================================

TEST_F(QuaternionTest, IdentityRotation) {
    // Arrange
    Vector3 v(1, 2, 3);
    
    // Act
    Vector3 rotated = identity.rotate(v);
    
    // Assert - identity не должен изменять вектор
    EXPECT_FLOAT_EQ(rotated.x, v.x);
    EXPECT_FLOAT_EQ(rotated.y, v.y);
    EXPECT_FLOAT_EQ(rotated.z, v.z);
}

TEST_F(QuaternionTest, CompositionOfRotations) {
    // Arrange
    Quaternion q1 = Quaternion::fromAxisAngle(Vector3::unitX(), M_PI / 2.0f);
    Quaternion q2 = Quaternion::fromAxisAngle(Vector3::unitY(), M_PI / 2.0f);
    
    // Act
    Quaternion combined = q1 * q2;
    Vector3 v(0, 0, 1);
    Vector3 result1 = q2.rotate(q1.rotate(v));
    Vector3 result2 = combined.rotate(v);
    
    // Assert - композиция должна давать тот же результат
    EXPECT_TRUE(nearlyEqual(result1.x, result2.x));
    EXPECT_TRUE(nearlyEqual(result1.y, result2.y));
    EXPECT_TRUE(nearlyEqual(result1.z, result2.z));
}

int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
