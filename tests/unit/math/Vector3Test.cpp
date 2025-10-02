#include <cmath>
#include "SpectraForge/Math/Vector3.h"
#include "TestFramework.h"

using namespace SpectraForge::Testing;
using namespace SpectraForge::Math;

/**
 * @brief Unit тесты для класса Vector3
 *
 * Тестирует базовые математические операции с 3D векторами
 */
class Vector3Test : public SpectraForgeTest {
  protected:
    void SetUp() override {
        SpectraForgeTest::SetUp();

        // Инициализация тестовых векторов
        v1 = Vector3(1.0f, 2.0f, 3.0f);
        v2 = Vector3(4.0f, 5.0f, 6.0f);
        zero = Vector3::zero();
        unitX = Vector3::unitX();
        unitY = Vector3::unitY();
        unitZ = Vector3::unitZ();
    }

    Vector3 v1, v2, zero, unitX, unitY, unitZ;
};

// Тесты конструкторов
TEST_F(Vector3Test, DefaultConstructor) {
    Vector3 v;
    EXPECT_FLOAT_EQ(v.x, 0.0f);
    EXPECT_FLOAT_EQ(v.y, 0.0f);
    EXPECT_FLOAT_EQ(v.z, 0.0f);
}

TEST_F(Vector3Test, ParameterizedConstructor) {
    EXPECT_FLOAT_EQ(v1.x, 1.0f);
    EXPECT_FLOAT_EQ(v1.y, 2.0f);
    EXPECT_FLOAT_EQ(v1.z, 3.0f);
}

TEST_F(Vector3Test, CopyConstructor) {
    Vector3 copy(v1);
    EXPECT_EQ(copy.x, v1.x);
    EXPECT_EQ(copy.y, v1.y);
    EXPECT_EQ(copy.z, v1.z);
}

// Тесты арифметических операций
TEST_F(Vector3Test, Addition) {
    Vector3 result = v1 + v2;
    EXPECT_FLOAT_EQ(result.x, 5.0f);
    EXPECT_FLOAT_EQ(result.y, 7.0f);
    EXPECT_FLOAT_EQ(result.z, 9.0f);
}

TEST_F(Vector3Test, Subtraction) {
    Vector3 result = v2 - v1;
    EXPECT_FLOAT_EQ(result.x, 3.0f);
    EXPECT_FLOAT_EQ(result.y, 3.0f);
    EXPECT_FLOAT_EQ(result.z, 3.0f);
}

TEST_F(Vector3Test, ScalarMultiplication) {
    Vector3 result = v1 * 2.0f;
    EXPECT_FLOAT_EQ(result.x, 2.0f);
    EXPECT_FLOAT_EQ(result.y, 4.0f);
    EXPECT_FLOAT_EQ(result.z, 6.0f);
}

TEST_F(Vector3Test, ScalarDivision) {
    Vector3 result = v1 / 2.0f;
    EXPECT_FLOAT_EQ(result.x, 0.5f);
    EXPECT_FLOAT_EQ(result.y, 1.0f);
    EXPECT_FLOAT_EQ(result.z, 1.5f);
}

// Тесты операций присваивания
TEST_F(Vector3Test, AdditionAssignment) {
    Vector3 result = v1;
    result += v2;
    EXPECT_FLOAT_EQ(result.x, 5.0f);
    EXPECT_FLOAT_EQ(result.y, 7.0f);
    EXPECT_FLOAT_EQ(result.z, 9.0f);
}

TEST_F(Vector3Test, SubtractionAssignment) {
    Vector3 result = v2;
    result -= v1;
    EXPECT_FLOAT_EQ(result.x, 3.0f);
    EXPECT_FLOAT_EQ(result.y, 3.0f);
    EXPECT_FLOAT_EQ(result.z, 3.0f);
}

TEST_F(Vector3Test, ScalarMultiplicationAssignment) {
    Vector3 result = v1;
    result *= 2.0f;
    EXPECT_FLOAT_EQ(result.x, 2.0f);
    EXPECT_FLOAT_EQ(result.y, 4.0f);
    EXPECT_FLOAT_EQ(result.z, 6.0f);
}

// Тесты векторных операций
TEST_F(Vector3Test, DotProduct) {
    float result = v1.dot(v2);
    EXPECT_FLOAT_EQ(result, 32.0f);  // 1*4 + 2*5 + 3*6 = 32
}

TEST_F(Vector3Test, CrossProduct) {
    Vector3 result = unitX.cross(unitY);
    Vector3 expected = unitZ;
    EXPECT_FLOAT_EQ(result.x, expected.x);
    EXPECT_FLOAT_EQ(result.y, expected.y);
    EXPECT_FLOAT_EQ(result.z, expected.z);
}

TEST_F(Vector3Test, CrossProductAnticommutative) {
    Vector3 result1 = unitX.cross(unitY);
    Vector3 result2 = unitY.cross(unitX);
    EXPECT_FLOAT_EQ(result1.x, -result2.x);
    EXPECT_FLOAT_EQ(result1.y, -result2.y);
    EXPECT_FLOAT_EQ(result1.z, -result2.z);
}

// Тесты длины и нормализации
TEST_F(Vector3Test, Magnitude) {
    float mag = v1.magnitude();
    float expected = std::sqrt(1.0f + 4.0f + 9.0f);  // sqrt(14)
    EXPECT_FLOAT_EQ(mag, expected);
}

TEST_F(Vector3Test, MagnitudeSquared) {
    float magSq = v1.magnitudeSquared();
    EXPECT_FLOAT_EQ(magSq, 14.0f);  // 1 + 4 + 9 = 14
}

TEST_F(Vector3Test, Normalization) {
    Vector3 result = v1.normalized();
    EXPECT_NORMALIZED(result);

    // Проверяем, что направление сохранилось
    Vector3 expected = v1 / v1.magnitude();
    EXPECT_NEAR(result.x, expected.x, 1e-6f);
    EXPECT_NEAR(result.y, expected.y, 1e-6f);
    EXPECT_NEAR(result.z, expected.z, 1e-6f);
}

TEST_F(Vector3Test, NormalizeInPlace) {
    Vector3 result = v1;
    result.normalize();
    EXPECT_NORMALIZED(result);
}

TEST_F(Vector3Test, NormalizeZeroVector) {
    Vector3 result = zero.normalized();
    // Нормализация нулевого вектора должна возвращать нулевой вектор
    EXPECT_TRUE(result.isZero());
}

// Тесты расстояния
TEST_F(Vector3Test, Distance) {
    float dist = v1.distance(v2);
    Vector3 diff = v2 - v1;
    float expectedDist = diff.magnitude();
    EXPECT_FLOAT_EQ(dist, expectedDist);
}

TEST_F(Vector3Test, DistanceSquared) {
    float distSq = v1.distanceSquared(v2);
    Vector3 diff = v2 - v1;
    float expectedDistSq = diff.magnitudeSquared();
    EXPECT_FLOAT_EQ(distSq, expectedDistSq);
}

// Тесты интерполяции
TEST_F(Vector3Test, LinearInterpolation) {
    Vector3 mid = v1.lerp(v2, 0.5f);
    Vector3 expected = (v1 + v2) * 0.5f;
    EXPECT_NEAR(mid.x, expected.x, 1e-6f);
    EXPECT_NEAR(mid.y, expected.y, 1e-6f);
    EXPECT_NEAR(mid.z, expected.z, 1e-6f);
}

TEST_F(Vector3Test, LerpBoundaryValues) {
    Vector3 start = v1.lerp(v2, 0.0f);
    Vector3 end = v1.lerp(v2, 1.0f);

    EXPECT_EQ(start, v1);
    EXPECT_EQ(end, v2);
}

// Тесты статических методов
TEST_F(Vector3Test, StaticZero) {
    EXPECT_TRUE(zero.isZero());
    EXPECT_FLOAT_EQ(zero.magnitude(), 0.0f);
}

TEST_F(Vector3Test, StaticUnitVectors) {
    EXPECT_NORMALIZED(unitX);
    EXPECT_NORMALIZED(unitY);
    EXPECT_NORMALIZED(unitZ);

    EXPECT_EQ(unitX, Vector3(1.0f, 0.0f, 0.0f));
    EXPECT_EQ(unitY, Vector3(0.0f, 1.0f, 0.0f));
    EXPECT_EQ(unitZ, Vector3(0.0f, 0.0f, 1.0f));
}

// Тесты операторов сравнения
TEST_F(Vector3Test, EqualityOperator) {
    Vector3 copy = v1;
    EXPECT_EQ(v1, copy);
    EXPECT_NE(v1, v2);
}

TEST_F(Vector3Test, EqualityWithTolerance) {
    Vector3 almost = Vector3(1.0000001f, 2.0000001f, 3.0000001f);
    // Точное равенство должно быть false
    EXPECT_NE(v1, almost);

    // Но они должны быть почти равны
    EXPECT_NEAR(v1.x, almost.x, 1e-6f);
    EXPECT_NEAR(v1.y, almost.y, 1e-6f);
    EXPECT_NEAR(v1.z, almost.z, 1e-6f);
}

// Тесты специальных случаев
TEST_F(Vector3Test, DivisionByZero) {
    EXPECT_THROW(v1 / 0.0f, std::runtime_error);
}

TEST_F(Vector3Test, NaNHandling) {
    Vector3 nanVector(std::numeric_limits<float>::quiet_NaN(), 0.0f, 0.0f);
    EXPECT_TRUE(std::isnan(nanVector.magnitude()));
}

// Тесты производительности
TEST_F(Vector3Test, DotProductPerformance) {
    const int iterations = 10000;

    EXPECT_PERFORMANCE_UNDER(
        {
            for (int i = 0; i < iterations; ++i) {
                volatile float result = v1.dot(v2);
                (void)result;  // Подавление предупреждения
            }
        },
        10);  // Должно выполниться за < 10ms
}

TEST_F(Vector3Test, CrossProductPerformance) {
    const int iterations = 10000;

    EXPECT_PERFORMANCE_UNDER(
        {
            for (int i = 0; i < iterations; ++i) {
                Vector3 result = v1.cross(v2);
                (void)result;  // Подавление предупреждения
            }
        },
        10);  // Должно выполниться за < 10ms
}

// Тесты граничных значений
TEST_F(Vector3Test, LargeValues) {
    Vector3 large(1e6f, 1e6f, 1e6f);
    Vector3 normalized = large.normalized();
    EXPECT_NORMALIZED(normalized);
}

TEST_F(Vector3Test, SmallValues) {
    Vector3 small(1e-6f, 1e-6f, 1e-6f);
    Vector3 normalized = small.normalized();
    EXPECT_NORMALIZED(normalized);
}
