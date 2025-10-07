/**
 * @file math_vector3_test.cpp
 * @brief Комплексные тесты для класса Vector3 (AAA pattern)
 * 
 * Покрывает все методы и граничные случаи для достижения >98% покрытия
 */

#include <gtest/gtest.h>
#include <SpectraForge/Math/Vector3.h>
#include <cmath>

using namespace SpectraForge::Math;

// ============================================================================
// Test Fixture для Vector3
// ============================================================================

class Vector3Test : public ::testing::Test {
protected:
    void SetUp() override {
        // Arrange - подготовка тестовых данных
        zero = Vector3::zero();
        one = Vector3::one();
        unitX = Vector3::unitX();
        unitY = Vector3::unitY();
        unitZ = Vector3::unitZ();
        
        v1 = Vector3(1.0f, 2.0f, 3.0f);
        v2 = Vector3(4.0f, 5.0f, 6.0f);
        v3 = Vector3(-1.0f, -2.0f, -3.0f);
    }

    // Вспомогательная функция для сравнения float с погрешностью
    bool nearlyEqual(float a, float b, float epsilon = 0.0001f) {
        return std::abs(a - b) < epsilon;
    }

    bool vectorsEqual(const Vector3& a, const Vector3& b, float epsilon = 0.0001f) {
        return nearlyEqual(a.x, b.x, epsilon) &&
               nearlyEqual(a.y, b.y, epsilon) &&
               nearlyEqual(a.z, b.z, epsilon);
    }

    // Тестовые векторы
    Vector3 zero, one, unitX, unitY, unitZ;
    Vector3 v1, v2, v3;
};

// ============================================================================
// Тесты конструкторов
// ============================================================================

TEST_F(Vector3Test, DefaultConstructor) {
    // Arrange & Act
    Vector3 v;
    
    // Assert
    EXPECT_FLOAT_EQ(v.x, 0.0f);
    EXPECT_FLOAT_EQ(v.y, 0.0f);
    EXPECT_FLOAT_EQ(v.z, 0.0f);
}

TEST_F(Vector3Test, ParameterizedConstructor) {
    // Arrange & Act
    Vector3 v(1.5f, 2.5f, 3.5f);
    
    // Assert
    EXPECT_FLOAT_EQ(v.x, 1.5f);
    EXPECT_FLOAT_EQ(v.y, 2.5f);
    EXPECT_FLOAT_EQ(v.z, 3.5f);
}

TEST_F(Vector3Test, CopyConstructor) {
    // Arrange
    Vector3 original(1.0f, 2.0f, 3.0f);
    
    // Act
    Vector3 copy(original);
    
    // Assert
    EXPECT_FLOAT_EQ(copy.x, original.x);
    EXPECT_FLOAT_EQ(copy.y, original.y);
    EXPECT_FLOAT_EQ(copy.z, original.z);
}

// ============================================================================
// Тесты операторов присваивания
// ============================================================================

TEST_F(Vector3Test, AssignmentOperator) {
    // Arrange
    Vector3 v;
    
    // Act
    v = v1;
    
    // Assert
    EXPECT_FLOAT_EQ(v.x, v1.x);
    EXPECT_FLOAT_EQ(v.y, v1.y);
    EXPECT_FLOAT_EQ(v.z, v1.z);
}

// ============================================================================
// Тесты арифметических операторов
// ============================================================================

TEST_F(Vector3Test, AdditionOperator) {
    // Arrange & Act
    Vector3 result = v1 + v2;
    
    // Assert
    EXPECT_FLOAT_EQ(result.x, 5.0f);
    EXPECT_FLOAT_EQ(result.y, 7.0f);
    EXPECT_FLOAT_EQ(result.z, 9.0f);
}

TEST_F(Vector3Test, SubtractionOperator) {
    // Arrange & Act
    Vector3 result = v2 - v1;
    
    // Assert
    EXPECT_FLOAT_EQ(result.x, 3.0f);
    EXPECT_FLOAT_EQ(result.y, 3.0f);
    EXPECT_FLOAT_EQ(result.z, 3.0f);
}

TEST_F(Vector3Test, UnaryMinusOperator) {
    // Arrange & Act
    Vector3 result = -v1;
    
    // Assert
    EXPECT_FLOAT_EQ(result.x, -1.0f);
    EXPECT_FLOAT_EQ(result.y, -2.0f);
    EXPECT_FLOAT_EQ(result.z, -3.0f);
}

TEST_F(Vector3Test, MultiplicationByScalar) {
    // Arrange & Act
    Vector3 result = v1 * 2.0f;
    
    // Assert
    EXPECT_FLOAT_EQ(result.x, 2.0f);
    EXPECT_FLOAT_EQ(result.y, 4.0f);
    EXPECT_FLOAT_EQ(result.z, 6.0f);
}

TEST_F(Vector3Test, ScalarMultiplicationLeftSide) {
    // Arrange & Act
    Vector3 result = 2.0f * v1;
    
    // Assert
    EXPECT_FLOAT_EQ(result.x, 2.0f);
    EXPECT_FLOAT_EQ(result.y, 4.0f);
    EXPECT_FLOAT_EQ(result.z, 6.0f);
}

TEST_F(Vector3Test, DivisionByScalar) {
    // Arrange & Act
    Vector3 result = v1 / 2.0f;
    
    // Assert
    EXPECT_FLOAT_EQ(result.x, 0.5f);
    EXPECT_FLOAT_EQ(result.y, 1.0f);
    EXPECT_FLOAT_EQ(result.z, 1.5f);
}

TEST_F(Vector3Test, CompoundAddition) {
    // Arrange
    Vector3 v = v1;
    
    // Act
    v += v2;
    
    // Assert
    EXPECT_FLOAT_EQ(v.x, 5.0f);
    EXPECT_FLOAT_EQ(v.y, 7.0f);
    EXPECT_FLOAT_EQ(v.z, 9.0f);
}

TEST_F(Vector3Test, CompoundSubtraction) {
    // Arrange
    Vector3 v = v2;
    
    // Act
    v -= v1;
    
    // Assert
    EXPECT_FLOAT_EQ(v.x, 3.0f);
    EXPECT_FLOAT_EQ(v.y, 3.0f);
    EXPECT_FLOAT_EQ(v.z, 3.0f);
}

TEST_F(Vector3Test, CompoundMultiplication) {
    // Arrange
    Vector3 v = v1;
    
    // Act
    v *= 2.0f;
    
    // Assert
    EXPECT_FLOAT_EQ(v.x, 2.0f);
    EXPECT_FLOAT_EQ(v.y, 4.0f);
    EXPECT_FLOAT_EQ(v.z, 6.0f);
}

TEST_F(Vector3Test, CompoundDivision) {
    // Arrange
    Vector3 v = v1;
    
    // Act
    v /= 2.0f;
    
    // Assert
    EXPECT_FLOAT_EQ(v.x, 0.5f);
    EXPECT_FLOAT_EQ(v.y, 1.0f);
    EXPECT_FLOAT_EQ(v.z, 1.5f);
}

// ============================================================================
// Тесты операторов сравнения
// ============================================================================

TEST_F(Vector3Test, EqualityOperator) {
    // Arrange
    Vector3 v1_copy = v1;
    
    // Act & Assert
    EXPECT_TRUE(v1 == v1_copy);
    EXPECT_FALSE(v1 == v2);
}

TEST_F(Vector3Test, InequalityOperator) {
    // Arrange & Act & Assert
    EXPECT_TRUE(v1 != v2);
    EXPECT_FALSE(v1 != v1);
}

// ============================================================================
// Тесты индексации
// ============================================================================

TEST_F(Vector3Test, IndexOperatorRead) {
    // Arrange & Act & Assert
    EXPECT_FLOAT_EQ(v1[0], 1.0f);
    EXPECT_FLOAT_EQ(v1[1], 2.0f);
    EXPECT_FLOAT_EQ(v1[2], 3.0f);
}

TEST_F(Vector3Test, IndexOperatorWrite) {
    // Arrange
    Vector3 v;
    
    // Act
    v[0] = 1.0f;
    v[1] = 2.0f;
    v[2] = 3.0f;
    
    // Assert
    EXPECT_FLOAT_EQ(v.x, 1.0f);
    EXPECT_FLOAT_EQ(v.y, 2.0f);
    EXPECT_FLOAT_EQ(v.z, 3.0f);
}

// ============================================================================
// Тесты математических операций
// ============================================================================

TEST_F(Vector3Test, DotProduct) {
    // Arrange & Act
    float result = v1.dot(v2);
    
    // Assert
    // 1*4 + 2*5 + 3*6 = 4 + 10 + 18 = 32
    EXPECT_FLOAT_EQ(result, 32.0f);
}

TEST_F(Vector3Test, DotProductStatic) {
    // Arrange & Act
    float result = Vector3::dot(v1, v2);
    
    // Assert
    EXPECT_FLOAT_EQ(result, 32.0f);
}

TEST_F(Vector3Test, DotProductOrthogonal) {
    // Arrange & Act
    float result = unitX.dot(unitY);
    
    // Assert
    EXPECT_FLOAT_EQ(result, 0.0f);
}

TEST_F(Vector3Test, CrossProduct) {
    // Arrange & Act
    Vector3 result = unitX.cross(unitY);
    
    // Assert
    EXPECT_TRUE(vectorsEqual(result, unitZ));
}

TEST_F(Vector3Test, CrossProductStatic) {
    // Arrange & Act
    Vector3 result = Vector3::cross(unitX, unitY);
    
    // Assert
    EXPECT_TRUE(vectorsEqual(result, unitZ));
}

TEST_F(Vector3Test, CrossProductAntiCommutative) {
    // Arrange & Act
    Vector3 result1 = v1.cross(v2);
    Vector3 result2 = v2.cross(v1);
    
    // Assert
    EXPECT_TRUE(vectorsEqual(result1, -result2));
}

TEST_F(Vector3Test, Magnitude) {
    // Arrange
    Vector3 v(3.0f, 4.0f, 0.0f);
    
    // Act
    float mag = v.magnitude();
    
    // Assert
    EXPECT_FLOAT_EQ(mag, 5.0f);
}

TEST_F(Vector3Test, MagnitudeSquared) {
    // Arrange
    Vector3 v(3.0f, 4.0f, 0.0f);
    
    // Act
    float magSq = v.magnitudeSquared();
    
    // Assert
    EXPECT_FLOAT_EQ(magSq, 25.0f);
}

TEST_F(Vector3Test, Normalize) {
    // Arrange
    Vector3 v(3.0f, 4.0f, 0.0f);
    
    // Act
    v.normalize();
    
    // Assert
    EXPECT_TRUE(nearlyEqual(v.magnitude(), 1.0f));
    EXPECT_TRUE(nearlyEqual(v.x, 0.6f, 0.01f));
    EXPECT_TRUE(nearlyEqual(v.y, 0.8f, 0.01f));
}

TEST_F(Vector3Test, Normalized) {
    // Arrange
    Vector3 v(3.0f, 4.0f, 0.0f);
    
    // Act
    Vector3 normalized = v.normalized();
    
    // Assert
    EXPECT_TRUE(nearlyEqual(normalized.magnitude(), 1.0f));
    EXPECT_FLOAT_EQ(v.magnitude(), 5.0f);  // Оригинал не изменился
}

TEST_F(Vector3Test, Distance) {
    // Arrange
    Vector3 a(0.0f, 0.0f, 0.0f);
    Vector3 b(3.0f, 4.0f, 0.0f);
    
    // Act
    float dist = a.distance(b);
    
    // Assert
    EXPECT_FLOAT_EQ(dist, 5.0f);
}

TEST_F(Vector3Test, DistanceSquared) {
    // Arrange
    Vector3 a(0.0f, 0.0f, 0.0f);
    Vector3 b(3.0f, 4.0f, 0.0f);
    
    // Act
    float distSq = a.distanceSquared(b);
    
    // Assert
    EXPECT_FLOAT_EQ(distSq, 25.0f);
}

// ============================================================================
// Тесты утилит
// ============================================================================

TEST_F(Vector3Test, SetMethod) {
    // Arrange
    Vector3 v;
    
    // Act
    v.set(1.0f, 2.0f, 3.0f);
    
    // Assert
    EXPECT_FLOAT_EQ(v.x, 1.0f);
    EXPECT_FLOAT_EQ(v.y, 2.0f);
    EXPECT_FLOAT_EQ(v.z, 3.0f);
}

TEST_F(Vector3Test, SetZero) {
    // Arrange
    Vector3 v = v1;
    
    // Act
    v.setZero();
    
    // Assert
    EXPECT_FLOAT_EQ(v.x, 0.0f);
    EXPECT_FLOAT_EQ(v.y, 0.0f);
    EXPECT_FLOAT_EQ(v.z, 0.0f);
}

TEST_F(Vector3Test, IsZero) {
    // Arrange & Act & Assert
    EXPECT_TRUE(zero.isZero());
    EXPECT_FALSE(v1.isZero());
}

TEST_F(Vector3Test, Lerp) {
    // Arrange
    Vector3 a(0.0f, 0.0f, 0.0f);
    Vector3 b(10.0f, 10.0f, 10.0f);
    
    // Act
    Vector3 mid = a.lerp(b, 0.5f);
    
    // Assert
    EXPECT_FLOAT_EQ(mid.x, 5.0f);
    EXPECT_FLOAT_EQ(mid.y, 5.0f);
    EXPECT_FLOAT_EQ(mid.z, 5.0f);
}

TEST_F(Vector3Test, LerpBoundaries) {
    // Arrange
    Vector3 a(0.0f, 0.0f, 0.0f);
    Vector3 b(10.0f, 10.0f, 10.0f);
    
    // Act
    Vector3 at0 = a.lerp(b, 0.0f);
    Vector3 at1 = a.lerp(b, 1.0f);
    
    // Assert
    EXPECT_TRUE(vectorsEqual(at0, a));
    EXPECT_TRUE(vectorsEqual(at1, b));
}

TEST_F(Vector3Test, Reflect) {
    // Arrange
    Vector3 incident(1.0f, -1.0f, 0.0f);
    Vector3 normal(0.0f, 1.0f, 0.0f);
    
    // Act
    Vector3 reflected = incident.reflect(normal);
    
    // Assert
    EXPECT_TRUE(vectorsEqual(reflected, Vector3(1.0f, 1.0f, 0.0f)));
}

TEST_F(Vector3Test, Project) {
    // Arrange
    Vector3 v(3.0f, 4.0f, 0.0f);
    Vector3 onto(1.0f, 0.0f, 0.0f);
    
    // Act
    Vector3 projected = v.project(onto);
    
    // Assert
    EXPECT_TRUE(vectorsEqual(projected, Vector3(3.0f, 0.0f, 0.0f)));
}

TEST_F(Vector3Test, Angle) {
    // Arrange & Act
    float angle = unitX.angle(unitY);
    
    // Assert
    EXPECT_TRUE(nearlyEqual(angle, M_PI / 2.0f, 0.01f));  // 90 градусов
}

// ============================================================================
// Тесты статических методов
// ============================================================================

TEST_F(Vector3Test, StaticZero) {
    // Arrange & Act
    Vector3 v = Vector3::zero();
    
    // Assert
    EXPECT_TRUE(v.isZero());
}

TEST_F(Vector3Test, StaticOne) {
    // Arrange & Act
    Vector3 v = Vector3::one();
    
    // Assert
    EXPECT_FLOAT_EQ(v.x, 1.0f);
    EXPECT_FLOAT_EQ(v.y, 1.0f);
    EXPECT_FLOAT_EQ(v.z, 1.0f);
}

TEST_F(Vector3Test, StaticDirections) {
    // Arrange & Act & Assert
    EXPECT_TRUE(vectorsEqual(Vector3::unitX(), Vector3(1.0f, 0.0f, 0.0f)));
    EXPECT_TRUE(vectorsEqual(Vector3::unitY(), Vector3(0.0f, 1.0f, 0.0f)));
    EXPECT_TRUE(vectorsEqual(Vector3::unitZ(), Vector3(0.0f, 0.0f, 1.0f)));
    EXPECT_TRUE(vectorsEqual(Vector3::forward(), Vector3(0.0f, 0.0f, -1.0f)));
    EXPECT_TRUE(vectorsEqual(Vector3::back(), Vector3(0.0f, 0.0f, 1.0f)));
    EXPECT_TRUE(vectorsEqual(Vector3::left(), Vector3(-1.0f, 0.0f, 0.0f)));
    EXPECT_TRUE(vectorsEqual(Vector3::right(), Vector3(1.0f, 0.0f, 0.0f)));
    EXPECT_TRUE(vectorsEqual(Vector3::up(), Vector3(0.0f, 1.0f, 0.0f)));
    EXPECT_TRUE(vectorsEqual(Vector3::down(), Vector3(0.0f, -1.0f, 0.0f)));
}

// ============================================================================
// Граничные случаи и проверка на ошибки
// ============================================================================

TEST_F(Vector3Test, NormalizeZeroVector) {
    // Arrange
    Vector3 v = Vector3::zero();
    
    // Act
    v.normalize();
    
    // Assert - zero vector после нормализации остается zero
    EXPECT_TRUE(v.isZero() || std::isnan(v.x));
}

TEST_F(Vector3Test, DivisionByZero) {
    // Arrange & Act
    Vector3 result = v1 / 0.0f;
    
    // Assert - результат должен быть inf или nan
    EXPECT_TRUE(std::isinf(result.x) || std::isnan(result.x));
}

TEST_F(Vector3Test, LargeValues) {
    // Arrange
    Vector3 large(1e30f, 1e30f, 1e30f);
    
    // Act
    float mag = large.magnitude();
    
    // Assert - проверка на overflow
    EXPECT_FALSE(std::isinf(mag));
}

TEST_F(Vector3Test, SmallValues) {
    // Arrange
    Vector3 small(1e-30f, 1e-30f, 1e-30f);
    
    // Act
    float mag = small.magnitude();
    
    // Assert
    EXPECT_TRUE(mag > 0.0f);
}

// ============================================================================
// Тесты производительности (опционально)
// ============================================================================

TEST_F(Vector3Test, PerformanceDotProduct) {
    // Arrange
    const int iterations = 1000000;
    float sum = 0.0f;
    
    // Act
    for (int i = 0; i < iterations; ++i) {
        sum += v1.dot(v2);
    }
    
    // Assert
    EXPECT_GT(sum, 0.0f);
}

// ============================================================================
// Main функция
// ============================================================================

int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
