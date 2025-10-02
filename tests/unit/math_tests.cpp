#include <gtest/gtest.h>
#include <cmath>
#include "SpectraForge/Math/Math.h"

using namespace SpectraForge::Math;

// Тесты Vector3
class Vector3Test : public ::testing::Test {
  protected:
    void SetUp() override {
        v1 = Vector3(1.0f, 2.0f, 3.0f);
        v2 = Vector3(4.0f, 5.0f, 6.0f);
        zero = Vector3::zero();
        unit_x = Vector3::unitX();
    }

    Vector3 v1, v2, zero, unit_x;
};

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
TEST_F(Vector3Test, DotProduct) {
    float result = v1.dot(v2);
    EXPECT_FLOAT_EQ(result, 32.0f);  // 1*4 + 2*5 + 3*6 = 32
}

TEST_F(Vector3Test, CrossProduct) {
    Vector3 result = unit_x.cross(Vector3::unitY());
    Vector3 expected = Vector3::unitZ();
    EXPECT_FLOAT_EQ(result.x, expected.x);
    EXPECT_FLOAT_EQ(result.y, expected.y);
    EXPECT_FLOAT_EQ(result.z, expected.z);
}

TEST_F(Vector3Test, Magnitude) {
    float mag = v1.magnitude();
    float expected = std::sqrt(1.0f + 4.0f + 9.0f);  // sqrt(14)
    EXPECT_FLOAT_EQ(mag, expected);
}

TEST_F(Vector3Test, Normalization) {
    Vector3 result = v1.normalized();
    EXPECT_NEAR(result.magnitude(), 1.0f, 1e-6f);
}

TEST_F(Vector3Test, StaticMethods) {
    EXPECT_TRUE(zero.isZero());
    EXPECT_FLOAT_EQ(unit_x.magnitude(), 1.0f);
    EXPECT_EQ(unit_x, Vector3(1.0f, 0.0f, 0.0f));
}

// Тесты Matrix4
class Matrix4Test : public ::testing::Test {
  protected:
    void SetUp() override {
        identity = Matrix4::identity();
        zero_matrix = Matrix4::zero();
    }

    Matrix4 identity, zero_matrix;
};

TEST_F(Matrix4Test, DefaultConstructor) {
    Matrix4 m;
    // Конструктор по умолчанию создает нулевую матрицу
    for (int i = 0; i < 4; ++i) {
        for (int j = 0; j < 4; ++j) {
            EXPECT_FLOAT_EQ(m(i, j), 0.0f);
        }
    }
}

TEST_F(Matrix4Test, Identity) {
    Vector3 v(1.0f, 2.0f, 3.0f);
    Vector3 result = identity * v;
    EXPECT_FLOAT_EQ(result.x, v.x);
    EXPECT_FLOAT_EQ(result.y, v.y);
    EXPECT_FLOAT_EQ(result.z, v.z);
}

TEST_F(Matrix4Test, Translation) {
    Vector3 translation(5.0f, 10.0f, 15.0f);
    Matrix4 trans = Matrix4::translation(translation);
    Vector3 point(1.0f, 2.0f, 3.0f);
    Vector3 result = trans * point;

    EXPECT_FLOAT_EQ(result.x, 6.0f);
    EXPECT_FLOAT_EQ(result.y, 12.0f);
    EXPECT_FLOAT_EQ(result.z, 18.0f);
}

TEST_F(Matrix4Test, Scaling) {
    Vector3 scale(2.0f, 3.0f, 4.0f);
    Matrix4 scaleMatrix = Matrix4::scaling(scale);
    Vector3 point(1.0f, 2.0f, 3.0f);
    Vector3 result = scaleMatrix * point;

    EXPECT_FLOAT_EQ(result.x, 2.0f);
    EXPECT_FLOAT_EQ(result.y, 6.0f);
    EXPECT_FLOAT_EQ(result.z, 12.0f);
}

TEST_F(Matrix4Test, Multiplication) {
    Matrix4 m1 = Matrix4::translation(1.0f, 2.0f, 3.0f);
    Matrix4 m2 = Matrix4::scaling(2.0f, 2.0f, 2.0f);
    Matrix4 result = m1 * m2;

    Vector3 point(1.0f, 1.0f, 1.0f);
    Vector3 transformed = result * point;

    // Сначала масштабирование, потом сдвиг
    EXPECT_FLOAT_EQ(transformed.x, 3.0f);  // 1*2 + 1 = 3
    EXPECT_FLOAT_EQ(transformed.y, 4.0f);  // 1*2 + 2 = 4
    EXPECT_FLOAT_EQ(transformed.z, 5.0f);  // 1*2 + 3 = 5
}

TEST_F(Matrix4Test, Determinant) {
    Matrix4 m = Matrix4::identity();
    EXPECT_FLOAT_EQ(m.determinant(), 1.0f);

    Matrix4 scale = Matrix4::scaling(2.0f, 3.0f, 4.0f);
    EXPECT_FLOAT_EQ(scale.determinant(), 24.0f);  // 2*3*4 = 24
}

TEST_F(Matrix4Test, Inverse) {
    Matrix4 m = Matrix4::scaling(2.0f, 3.0f, 4.0f);
    Matrix4 inv = m.inverse();
    Matrix4 result = m * inv;

    // Результат должен быть близок к единичной матрице
    for (int i = 0; i < 4; ++i) {
        for (int j = 0; j < 4; ++j) {
            if (i == j) {
                EXPECT_NEAR(result(i, j), 1.0f, 1e-6f);
            } else {
                EXPECT_NEAR(result(i, j), 0.0f, 1e-6f);
            }
        }
    }
}

// Тесты Quaternion
class QuaternionTest : public ::testing::Test {
  protected:
    void SetUp() override {
        identity = Quaternion::identity();
        zero_quat = Quaternion::zero();
    }

    Quaternion identity, zero_quat;
};

TEST_F(QuaternionTest, DefaultConstructor) {
    Quaternion q;
    EXPECT_FLOAT_EQ(q.w, 1.0f);
    EXPECT_FLOAT_EQ(q.x, 0.0f);
    EXPECT_FLOAT_EQ(q.y, 0.0f);
    EXPECT_FLOAT_EQ(q.z, 0.0f);
}

TEST_F(QuaternionTest, Identity) {
    EXPECT_TRUE(identity.isIdentity());
    EXPECT_FLOAT_EQ(identity.magnitude(), 1.0f);
}

TEST_F(QuaternionTest, AxisAngleConstruction) {
    Vector3 axis = Vector3::unitY();
    float angle = PI / 2.0f;  // 90 градусов
    Quaternion q = Quaternion::fromAxisAngle(axis, angle);

    EXPECT_NEAR(q.magnitude(), 1.0f, 1e-6f);

    // Поворот точки вокруг Y на 90 градусов
    Vector3 point = Vector3::unitX();
    Vector3 rotated = q.rotate(point);
    Vector3 expected = -Vector3::unitZ();  // В левосторонней системе координат

    EXPECT_NEAR(rotated.x, expected.x, 1e-6f);
    EXPECT_NEAR(rotated.y, expected.y, 1e-6f);
    EXPECT_NEAR(rotated.z, expected.z, 1e-6f);
}

TEST_F(QuaternionTest, Multiplication) {
    // Два поворота на 90 градусов должны дать поворот на 180 градусов
    Vector3 axis = Vector3::unitY();
    float angle = PI / 2.0f;
    Quaternion q1 = Quaternion::fromAxisAngle(axis, angle);
    Quaternion q2 = Quaternion::fromAxisAngle(axis, angle);
    Quaternion result = q1 * q2;

    Vector3 point = Vector3::unitX();
    Vector3 rotated = result.rotate(point);
    Vector3 expected = -Vector3::unitX();  // Поворот на 180 градусов

    EXPECT_NEAR(rotated.x, expected.x, 1e-6f);
    EXPECT_NEAR(rotated.y, expected.y, 1e-6f);
    EXPECT_NEAR(rotated.z, expected.z, 1e-6f);
}

TEST_F(QuaternionTest, Normalization) {
    Quaternion q(2.0f, 3.0f, 4.0f, 5.0f);
    q.normalize();
    EXPECT_NEAR(q.magnitude(), 1.0f, 1e-6f);
}

TEST_F(QuaternionTest, Conjugate) {
    Quaternion q(1.0f, 2.0f, 3.0f, 4.0f);
    Quaternion conj = q.conjugate();

    EXPECT_FLOAT_EQ(conj.w, q.w);
    EXPECT_FLOAT_EQ(conj.x, -q.x);
    EXPECT_FLOAT_EQ(conj.y, -q.y);
    EXPECT_FLOAT_EQ(conj.z, -q.z);
}

TEST_F(QuaternionTest, Slerp) {
    Quaternion q1 = Quaternion::identity();
    Quaternion q2 = Quaternion::fromAxisAngle(Vector3::unitY(), PI / 2.0f);

    Quaternion mid = q1.slerp(q2, 0.5f);

    // Результат должен быть нормализован
    EXPECT_NEAR(mid.magnitude(), 1.0f, 1e-6f);

    // Результат должен быть между q1 и q2
    Vector3 point = Vector3::unitX();
    Vector3 rotated = mid.rotate(point);

    // Поворот на 45 градусов должен дать вектор (cos(45), 0, -sin(45)) в левосторонней системе
    float cos45 = std::cos(PI / 4.0f);
    float sin45 = std::sin(PI / 4.0f);

    EXPECT_NEAR(rotated.x, cos45, 1e-5f);
    EXPECT_NEAR(rotated.y, 0.0f, 1e-6f);
    EXPECT_NEAR(rotated.z, -sin45, 1e-5f);
}

// Функция main для запуска тестов
int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
