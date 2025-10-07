/**
 * @file math_vector_extra_test.cpp
 * @brief Тесты для Vector2 и Vector4
 */

#include <gtest/gtest.h>
#include <SpectraForge/Math/Vector2.h>
#include <SpectraForge/Math/Vector4.h>
#include <cmath>

using namespace SpectraForge::Math;

// ============================================================================
// Vector2 Tests
// ============================================================================

class Vector2Test : public ::testing::Test {
protected:
    float epsilon = 0.0001f;
    
    bool nearlyEqual(float a, float b) {
        return std::abs(a - b) < epsilon;
    }
};

TEST_F(Vector2Test, DefaultConstructor) {
    // Arrange & Act
    Vector2 v;
    
    // Assert
    EXPECT_FLOAT_EQ(v.x, 0.0f);
    EXPECT_FLOAT_EQ(v.y, 0.0f);
}

TEST_F(Vector2Test, ParameterConstructor) {
    // Arrange & Act
    Vector2 v(1.5f, 2.5f);
    
    // Assert
    EXPECT_FLOAT_EQ(v.x, 1.5f);
    EXPECT_FLOAT_EQ(v.y, 2.5f);
}

TEST_F(Vector2Test, Addition) {
    // Arrange
    Vector2 v1(1.0f, 2.0f);
    Vector2 v2(3.0f, 4.0f);
    
    // Act
    Vector2 result = v1 + v2;
    
    // Assert
    EXPECT_FLOAT_EQ(result.x, 4.0f);
    EXPECT_FLOAT_EQ(result.y, 6.0f);
}

TEST_F(Vector2Test, Subtraction) {
    // Arrange
    Vector2 v1(5.0f, 6.0f);
    Vector2 v2(2.0f, 3.0f);
    
    // Act
    Vector2 result = v1 - v2;
    
    // Assert
    EXPECT_FLOAT_EQ(result.x, 3.0f);
    EXPECT_FLOAT_EQ(result.y, 3.0f);
}

TEST_F(Vector2Test, ScalarMultiplication) {
    // Arrange
    Vector2 v(2.0f, 3.0f);
    
    // Act
    Vector2 result = v * 2.0f;
    
    // Assert
    EXPECT_FLOAT_EQ(result.x, 4.0f);
    EXPECT_FLOAT_EQ(result.y, 6.0f);
}

TEST_F(Vector2Test, DotProduct) {
    // Arrange
    Vector2 v1(1.0f, 2.0f);
    Vector2 v2(3.0f, 4.0f);
    
    // Act
    float dot = v1.x * v2.x + v1.y * v2.y;
    
    // Assert
    EXPECT_FLOAT_EQ(dot, 11.0f);  // 1*3 + 2*4 = 11
}

TEST_F(Vector2Test, Magnitude) {
    // Arrange
    Vector2 v(3.0f, 4.0f);
    
    // Act
    float mag = std::sqrt(v.x * v.x + v.y * v.y);
    
    // Assert
    EXPECT_FLOAT_EQ(mag, 5.0f);
}

TEST_F(Vector2Test, Normalization) {
    // Arrange
    Vector2 v(3.0f, 4.0f);
    float mag = std::sqrt(v.x * v.x + v.y * v.y);
    
    // Act
    Vector2 normalized(v.x / mag, v.y / mag);
    
    // Assert
    float normalizedMag = std::sqrt(normalized.x * normalized.x + normalized.y * normalized.y);
    EXPECT_TRUE(nearlyEqual(normalizedMag, 1.0f));
}

TEST_F(Vector2Test, Equality) {
    // Arrange
    Vector2 v1(1.0f, 2.0f);
    Vector2 v2(1.0f, 2.0f);
    Vector2 v3(3.0f, 4.0f);
    
    // Act & Assert
    EXPECT_TRUE(v1.x == v2.x && v1.y == v2.y);
    EXPECT_FALSE(v1.x == v3.x && v1.y == v3.y);
}

TEST_F(Vector2Test, ZeroVector) {
    // Arrange & Act
    Vector2 zero;
    
    // Assert
    EXPECT_FLOAT_EQ(zero.x, 0.0f);
    EXPECT_FLOAT_EQ(zero.y, 0.0f);
}

// ============================================================================
// Vector4 Tests
// ============================================================================

class Vector4Test : public ::testing::Test {
protected:
    float epsilon = 0.0001f;
    
    bool nearlyEqual(float a, float b) {
        return std::abs(a - b) < epsilon;
    }
};

TEST_F(Vector4Test, DefaultConstructor) {
    // Arrange & Act
    Vector4 v;
    
    // Assert
    EXPECT_FLOAT_EQ(v.x, 0.0f);
    EXPECT_FLOAT_EQ(v.y, 0.0f);
    EXPECT_FLOAT_EQ(v.z, 0.0f);
    EXPECT_FLOAT_EQ(v.w, 0.0f);
}

TEST_F(Vector4Test, ParameterConstructor) {
    // Arrange & Act
    Vector4 v(1.0f, 2.0f, 3.0f, 4.0f);
    
    // Assert
    EXPECT_FLOAT_EQ(v.x, 1.0f);
    EXPECT_FLOAT_EQ(v.y, 2.0f);
    EXPECT_FLOAT_EQ(v.z, 3.0f);
    EXPECT_FLOAT_EQ(v.w, 4.0f);
}

TEST_F(Vector4Test, Addition) {
    // Arrange
    Vector4 v1(1.0f, 2.0f, 3.0f, 4.0f);
    Vector4 v2(5.0f, 6.0f, 7.0f, 8.0f);
    
    // Act
    Vector4 result = v1 + v2;
    
    // Assert
    EXPECT_FLOAT_EQ(result.x, 6.0f);
    EXPECT_FLOAT_EQ(result.y, 8.0f);
    EXPECT_FLOAT_EQ(result.z, 10.0f);
    EXPECT_FLOAT_EQ(result.w, 12.0f);
}

TEST_F(Vector4Test, Subtraction) {
    // Arrange
    Vector4 v1(10.0f, 9.0f, 8.0f, 7.0f);
    Vector4 v2(1.0f, 2.0f, 3.0f, 4.0f);
    
    // Act
    Vector4 result = v1 - v2;
    
    // Assert
    EXPECT_FLOAT_EQ(result.x, 9.0f);
    EXPECT_FLOAT_EQ(result.y, 7.0f);
    EXPECT_FLOAT_EQ(result.z, 5.0f);
    EXPECT_FLOAT_EQ(result.w, 3.0f);
}

TEST_F(Vector4Test, ScalarMultiplication) {
    // Arrange
    Vector4 v(1.0f, 2.0f, 3.0f, 4.0f);
    
    // Act
    Vector4 result = v * 3.0f;
    
    // Assert
    EXPECT_FLOAT_EQ(result.x, 3.0f);
    EXPECT_FLOAT_EQ(result.y, 6.0f);
    EXPECT_FLOAT_EQ(result.z, 9.0f);
    EXPECT_FLOAT_EQ(result.w, 12.0f);
}

TEST_F(Vector4Test, DivisionByScalar) {
    // Arrange
    Vector4 v(2.0f, 4.0f, 6.0f, 8.0f);
    
    // Act
    Vector4 result = v / 2.0f;
    
    // Assert
    EXPECT_FLOAT_EQ(result.x, 1.0f);
    EXPECT_FLOAT_EQ(result.y, 2.0f);
    EXPECT_FLOAT_EQ(result.z, 3.0f);
    EXPECT_FLOAT_EQ(result.w, 4.0f);
}

TEST_F(Vector4Test, DotProduct) {
    // Arrange
    Vector4 v1(1.0f, 2.0f, 3.0f, 4.0f);
    Vector4 v2(5.0f, 6.0f, 7.0f, 8.0f);
    
    // Act
    float dot = v1.x * v2.x + v1.y * v2.y + v1.z * v2.z + v1.w * v2.w;
    
    // Assert
    // 1*5 + 2*6 + 3*7 + 4*8 = 5 + 12 + 21 + 32 = 70
    EXPECT_FLOAT_EQ(dot, 70.0f);
}

TEST_F(Vector4Test, Magnitude) {
    // Arrange
    Vector4 v(1.0f, 2.0f, 2.0f, 4.0f);
    
    // Act
    float magSq = v.x * v.x + v.y * v.y + v.z * v.z + v.w * v.w;
    float mag = std::sqrt(magSq);
    
    // Assert
    // 1 + 4 + 4 + 16 = 25
    EXPECT_FLOAT_EQ(mag, 5.0f);
}

TEST_F(Vector4Test, Normalization) {
    // Arrange
    Vector4 v(1.0f, 2.0f, 2.0f, 4.0f);
    float mag = std::sqrt(v.x * v.x + v.y * v.y + v.z * v.z + v.w * v.w);
    
    // Act
    Vector4 normalized(v.x / mag, v.y / mag, v.z / mag, v.w / mag);
    
    // Assert
    float normalizedMag = std::sqrt(
        normalized.x * normalized.x + 
        normalized.y * normalized.y + 
        normalized.z * normalized.z + 
        normalized.w * normalized.w
    );
    EXPECT_TRUE(nearlyEqual(normalizedMag, 1.0f));
}

TEST_F(Vector4Test, Equality) {
    // Arrange
    Vector4 v1(1.0f, 2.0f, 3.0f, 4.0f);
    Vector4 v2(1.0f, 2.0f, 3.0f, 4.0f);
    Vector4 v3(5.0f, 6.0f, 7.0f, 8.0f);
    
    // Act & Assert
    bool equal1 = (v1.x == v2.x && v1.y == v2.y && v1.z == v2.z && v1.w == v2.w);
    bool equal2 = (v1.x == v3.x && v1.y == v3.y && v1.z == v3.z && v1.w == v3.w);
    
    EXPECT_TRUE(equal1);
    EXPECT_FALSE(equal2);
}

TEST_F(Vector4Test, CompoundAddition) {
    // Arrange
    Vector4 v1(1.0f, 2.0f, 3.0f, 4.0f);
    Vector4 v2(5.0f, 6.0f, 7.0f, 8.0f);
    
    // Act
    v1 += v2;
    
    // Assert
    EXPECT_FLOAT_EQ(v1.x, 6.0f);
    EXPECT_FLOAT_EQ(v1.y, 8.0f);
    EXPECT_FLOAT_EQ(v1.z, 10.0f);
    EXPECT_FLOAT_EQ(v1.w, 12.0f);
}

TEST_F(Vector4Test, CompoundSubtraction) {
    // Arrange
    Vector4 v1(10.0f, 9.0f, 8.0f, 7.0f);
    Vector4 v2(1.0f, 2.0f, 3.0f, 4.0f);
    
    // Act
    v1 -= v2;
    
    // Assert
    EXPECT_FLOAT_EQ(v1.x, 9.0f);
    EXPECT_FLOAT_EQ(v1.y, 7.0f);
    EXPECT_FLOAT_EQ(v1.z, 5.0f);
    EXPECT_FLOAT_EQ(v1.w, 3.0f);
}

TEST_F(Vector4Test, IndexOperator) {
    // Arrange
    Vector4 v(1.0f, 2.0f, 3.0f, 4.0f);
    
    // Act & Assert
    EXPECT_FLOAT_EQ(v[0], 1.0f);
    EXPECT_FLOAT_EQ(v[1], 2.0f);
    EXPECT_FLOAT_EQ(v[2], 3.0f);
    EXPECT_FLOAT_EQ(v[3], 4.0f);
}

TEST_F(Vector4Test, HomogeneousCoordinate) {
    // Arrange - точка в пространстве
    Vector4 point(1.0f, 2.0f, 3.0f, 1.0f);
    
    // Arrange - направление в пространстве
    Vector4 direction(1.0f, 2.0f, 3.0f, 0.0f);
    
    // Assert
    EXPECT_FLOAT_EQ(point.w, 1.0f);
    EXPECT_FLOAT_EQ(direction.w, 0.0f);
}

TEST_F(Vector4Test, ZeroVector) {
    // Arrange & Act
    Vector4 zero;
    
    // Assert
    EXPECT_FLOAT_EQ(zero.x, 0.0f);
    EXPECT_FLOAT_EQ(zero.y, 0.0f);
    EXPECT_FLOAT_EQ(zero.z, 0.0f);
    EXPECT_FLOAT_EQ(zero.w, 0.0f);
}

TEST_F(Vector4Test, LargeValues) {
    // Arrange
    Vector4 large(1e30f, 1e30f, 1e30f, 1e30f);
    
    // Act
    Vector4 result = large + large;
    
    // Assert - проверка на overflow
    EXPECT_FALSE(std::isinf(result.x));
}

int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
