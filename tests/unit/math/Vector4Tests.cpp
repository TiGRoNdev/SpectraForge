#include "SpectraForge/Math/Vector4.h"
#include "TestFramework.h"

using namespace SpectraForge::Testing;
using namespace SpectraForge::Math;

/**
 * @brief Unit тесты для Vector4
 *
 * Тестирует 4D векторные операции согласно архитектуре SpectraForge
 */
class Vector4Test : public SpectraForgeTest {
  protected:
    void SetUp() override {
        SpectraForgeTest::SetUp();

        // Создаем тестовые векторы
        v1 = Vector4(1.0f, 2.0f, 3.0f, 4.0f);
        v2 = Vector4(5.0f, 6.0f, 7.0f, 8.0f);
        v3 = Vector4(0.0f, 0.0f, 0.0f, 0.0f);
        v4 = Vector4(3.0f, 4.0f, 0.0f, 0.0f);  // 3D вектор в 4D пространстве
    }

    Vector4 v1, v2, v3, v4;
};

// Тесты конструкторов
TEST_F(Vector4Test, DefaultConstructor) {
    Vector4 v;
    EXPECT_FLOAT_EQ(v.x, 0.0f);
    EXPECT_FLOAT_EQ(v.y, 0.0f);
    EXPECT_FLOAT_EQ(v.z, 0.0f);
    EXPECT_FLOAT_EQ(v.w, 0.0f);
}

TEST_F(Vector4Test, ParameterizedConstructor) {
    Vector4 v(1.0f, 2.0f, 3.0f, 4.0f);
    EXPECT_FLOAT_EQ(v.x, 1.0f);
    EXPECT_FLOAT_EQ(v.y, 2.0f);
    EXPECT_FLOAT_EQ(v.z, 3.0f);
    EXPECT_FLOAT_EQ(v.w, 4.0f);
}

TEST_F(Vector4Test, CopyConstructor) {
    Vector4 v(v1);
    EXPECT_FLOAT_EQ(v.x, v1.x);
    EXPECT_FLOAT_EQ(v.y, v1.y);
    EXPECT_FLOAT_EQ(v.z, v1.z);
    EXPECT_FLOAT_EQ(v.w, v1.w);
}

// Тесты арифметических операций
TEST_F(Vector4Test, Addition) {
    Vector4 result = v1 + v2;
    EXPECT_FLOAT_EQ(result.x, 6.0f);
    EXPECT_FLOAT_EQ(result.y, 8.0f);
    EXPECT_FLOAT_EQ(result.z, 10.0f);
    EXPECT_FLOAT_EQ(result.w, 12.0f);
}

TEST_F(Vector4Test, Subtraction) {
    Vector4 result = v2 - v1;
    EXPECT_FLOAT_EQ(result.x, 4.0f);
    EXPECT_FLOAT_EQ(result.y, 4.0f);
    EXPECT_FLOAT_EQ(result.z, 4.0f);
    EXPECT_FLOAT_EQ(result.w, 4.0f);
}

TEST_F(Vector4Test, ScalarMultiplication) {
    Vector4 result = v1 * 2.0f;
    EXPECT_FLOAT_EQ(result.x, 2.0f);
    EXPECT_FLOAT_EQ(result.y, 4.0f);
    EXPECT_FLOAT_EQ(result.z, 6.0f);
    EXPECT_FLOAT_EQ(result.w, 8.0f);
}

TEST_F(Vector4Test, ScalarDivision) {
    Vector4 result = v1 / 2.0f;
    EXPECT_FLOAT_EQ(result.x, 0.5f);
    EXPECT_FLOAT_EQ(result.y, 1.0f);
    EXPECT_FLOAT_EQ(result.z, 1.5f);
    EXPECT_FLOAT_EQ(result.w, 2.0f);
}

// Тесты скалярного произведения
TEST_F(Vector4Test, DotProduct) {
    float result = v1.dot(v2);
    float expected = 1.0f * 5.0f + 2.0f * 6.0f + 3.0f * 7.0f + 4.0f * 8.0f;
    EXPECT_FLOAT_EQ(result, expected);
}

TEST_F(Vector4Test, DotProductWithZero) {
    float result = v1.dot(v3);
    EXPECT_FLOAT_EQ(result, 0.0f);
}

// Тесты длины вектора
TEST_F(Vector4Test, Magnitude) {
    float result = v4.magnitude();
    float expected = 5.0f;  // sqrt(3^2 + 4^2 + 0^2 + 0^2) = 5
    EXPECT_FLOAT_EQ(result, expected);
}

TEST_F(Vector4Test, MagnitudeZero) {
    float result = v3.magnitude();
    EXPECT_FLOAT_EQ(result, 0.0f);
}

TEST_F(Vector4Test, MagnitudeSquared) {
    float result = v4.magnitudeSquared();
    float expected = 25.0f;  // 3^2 + 4^2 + 0^2 + 0^2 = 25
    EXPECT_FLOAT_EQ(result, expected);
}

// Тесты нормализации
TEST_F(Vector4Test, Normalize) {
    Vector4 normalized = v4.normalized();
    float magnitude = normalized.magnitude();
    EXPECT_NEAR(magnitude, 1.0f, 1e-6f);
}

TEST_F(Vector4Test, NormalizeZeroVector) {
    Vector4 normalized = v3.normalized();
    // Нормализация нулевого вектора должна возвращать нулевой вектор
    EXPECT_FLOAT_EQ(normalized.x, 0.0f);
    EXPECT_FLOAT_EQ(normalized.y, 0.0f);
    EXPECT_FLOAT_EQ(normalized.z, 0.0f);
    EXPECT_FLOAT_EQ(normalized.w, 0.0f);
}

// Тесты операторов присваивания
TEST_F(Vector4Test, AssignmentOperator) {
    Vector4 v;
    v = v1;
    EXPECT_FLOAT_EQ(v.x, v1.x);
    EXPECT_FLOAT_EQ(v.y, v1.y);
    EXPECT_FLOAT_EQ(v.z, v1.z);
    EXPECT_FLOAT_EQ(v.w, v1.w);
}

TEST_F(Vector4Test, AdditionAssignment) {
    Vector4 v = v1;
    v += v2;
    Vector4 expected = v1 + v2;
    EXPECT_FLOAT_EQ(v.x, expected.x);
    EXPECT_FLOAT_EQ(v.y, expected.y);
    EXPECT_FLOAT_EQ(v.z, expected.z);
    EXPECT_FLOAT_EQ(v.w, expected.w);
}

TEST_F(Vector4Test, SubtractionAssignment) {
    Vector4 v = v2;
    v -= v1;
    Vector4 expected = v2 - v1;
    EXPECT_FLOAT_EQ(v.x, expected.x);
    EXPECT_FLOAT_EQ(v.y, expected.y);
    EXPECT_FLOAT_EQ(v.z, expected.z);
    EXPECT_FLOAT_EQ(v.w, expected.w);
}

TEST_F(Vector4Test, MultiplicationAssignment) {
    Vector4 v = v1;
    v *= 2.0f;
    Vector4 expected = v1 * 2.0f;
    EXPECT_FLOAT_EQ(v.x, expected.x);
    EXPECT_FLOAT_EQ(v.y, expected.y);
    EXPECT_FLOAT_EQ(v.z, expected.z);
    EXPECT_FLOAT_EQ(v.w, expected.w);
}

TEST_F(Vector4Test, DivisionAssignment) {
    Vector4 v = v1;
    v /= 2.0f;
    Vector4 expected = v1 / 2.0f;
    EXPECT_FLOAT_EQ(v.x, expected.x);
    EXPECT_FLOAT_EQ(v.y, expected.y);
    EXPECT_FLOAT_EQ(v.z, expected.z);
    EXPECT_FLOAT_EQ(v.w, expected.w);
}

// Тесты операторов сравнения
TEST_F(Vector4Test, Equality) {
    Vector4 v = v1;
    EXPECT_TRUE(v == v1);
    EXPECT_FALSE(v == v2);
}

TEST_F(Vector4Test, Inequality) {
    EXPECT_TRUE(v1 != v2);
    EXPECT_FALSE(v1 != v1);
}

// Тесты специальных случаев
TEST_F(Vector4Test, DivisionByZero) {
    Vector4 result = v1 / 0.0f;
    // Должно обрабатываться корректно (возвращать inf или nan)
    EXPECT_TRUE(std::isinf(result.x) || std::isnan(result.x));
}

TEST_F(Vector4Test, NegativeValues) {
    Vector4 v(-1.0f, -2.0f, -3.0f, -4.0f);
    float magnitude = v.magnitude();
    EXPECT_FLOAT_EQ(magnitude, std::sqrt(30.0f));  // sqrt(1+4+9+16)
}

// Тесты производительности
TEST_F(Vector4Test, PerformanceOperations) {
    const int iterations = 1000000;

    EXPECT_PERFORMANCE_UNDER(
        {
            Vector4 result = v1;
            for (int i = 0; i < iterations; ++i) {
                result = result + v2;
                result = result * 0.999f;  // Предотвращаем переполнение
            }
        },
        100);  // 1M операций за < 100ms
}

// Параметризованные тесты для различных значений
class Vector4ParameterizedTest
    : public Vector4Test,
      public ::testing::WithParamInterface<std::tuple<float, float, float, float>> {};

TEST_P(Vector4ParameterizedTest, MagnitudeCalculation) {
    auto [x, y, z, w] = GetParam();
    Vector4 v(x, y, z, w);

    float expected = std::sqrt(x * x + y * y + z * z + w * w);
    float result = v.magnitude();

    EXPECT_NEAR(result, expected, 1e-6f);
}

INSTANTIATE_TEST_SUITE_P(MagnitudeTests,
                         Vector4ParameterizedTest,
                         ::testing::Values(std::make_tuple(1.0f, 0.0f, 0.0f, 0.0f),
                                           std::make_tuple(0.0f, 1.0f, 0.0f, 0.0f),
                                           std::make_tuple(0.0f, 0.0f, 1.0f, 0.0f),
                                           std::make_tuple(0.0f, 0.0f, 0.0f, 1.0f),
                                           std::make_tuple(1.0f, 1.0f, 1.0f, 1.0f),
                                           std::make_tuple(3.0f, 4.0f, 0.0f, 0.0f),
                                           std::make_tuple(1.0f, 2.0f, 3.0f, 4.0f)));
