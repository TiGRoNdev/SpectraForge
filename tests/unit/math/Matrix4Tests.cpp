#include "SpectraForge/Math/Matrix4.h"
#include "SpectraForge/Math/Vector4.h"
#include "TestFramework.h"

using namespace SpectraForge::Testing;
using namespace SpectraForge::Math;

/**
 * @brief Unit тесты для Matrix4
 *
 * Тестирует 4x4 матричные операции согласно архитектуре SpectraForge
 */
class Matrix4Test : public SpectraForgeTest {
  protected:
    void SetUp() override {
        SpectraForgeTest::SetUp();

        // Создаем тестовые матрицы
        identity = Matrix4::identity();

        // Матрица поворота на 90 градусов вокруг Z
        rotationZ = Matrix4::rotationZ(3.14159265f / 2.0f);

        // Матрица масштабирования
        scale = Matrix4::scale(2.0f, 3.0f, 4.0f);

        // Матрица переноса
        translation = Matrix4::translation(1.0f, 2.0f, 3.0f);

        // Тестовая матрица
        testMatrix = Matrix4(1.0f,
                             2.0f,
                             3.0f,
                             4.0f,
                             5.0f,
                             6.0f,
                             7.0f,
                             8.0f,
                             9.0f,
                             10.0f,
                             11.0f,
                             12.0f,
                             13.0f,
                             14.0f,
                             15.0f,
                             16.0f);

        // Тестовый вектор
        testVector = Vector4(1.0f, 2.0f, 3.0f, 1.0f);
    }

    Matrix4 identity, rotationZ, scale, translation, testMatrix;
    Vector4 testVector;
};

// Тесты конструкторов
TEST_F(Matrix4Test, DefaultConstructor) {
    Matrix4 m;
    for (int i = 0; i < 4; ++i) {
        for (int j = 0; j < 4; ++j) {
            EXPECT_FLOAT_EQ(m(i, j), 0.0f);
        }
    }
}

TEST_F(Matrix4Test, IdentityMatrix) {
    EXPECT_FLOAT_EQ(identity(0, 0), 1.0f);
    EXPECT_FLOAT_EQ(identity(1, 1), 1.0f);
    EXPECT_FLOAT_EQ(identity(2, 2), 1.0f);
    EXPECT_FLOAT_EQ(identity(3, 3), 1.0f);

    // Проверяем, что недиагональные элементы равны 0
    for (int i = 0; i < 4; ++i) {
        for (int j = 0; j < 4; ++j) {
            if (i != j) {
                EXPECT_FLOAT_EQ(identity(i, j), 0.0f);
            }
        }
    }
}

TEST_F(Matrix4Test, ParameterizedConstructor) {
    Matrix4 m(1.0f,
              2.0f,
              3.0f,
              4.0f,
              5.0f,
              6.0f,
              7.0f,
              8.0f,
              9.0f,
              10.0f,
              11.0f,
              12.0f,
              13.0f,
              14.0f,
              15.0f,
              16.0f);

    EXPECT_FLOAT_EQ(m(0, 0), 1.0f);
    EXPECT_FLOAT_EQ(m(0, 1), 2.0f);
    EXPECT_FLOAT_EQ(m(0, 2), 3.0f);
    EXPECT_FLOAT_EQ(m(0, 3), 4.0f);
    EXPECT_FLOAT_EQ(m(1, 0), 5.0f);
    EXPECT_FLOAT_EQ(m(3, 3), 16.0f);
}

// Тесты матричных операций
TEST_F(Matrix4Test, MatrixMultiplication) {
    Matrix4 result = identity * testMatrix;

    // Умножение на единичную матрицу должно дать исходную матрицу
    for (int i = 0; i < 4; ++i) {
        for (int j = 0; j < 4; ++j) {
            EXPECT_FLOAT_EQ(result(i, j), testMatrix(i, j));
        }
    }
}

TEST_F(Matrix4Test, MatrixVectorMultiplication) {
    Vector4 result = identity * testVector;

    // Умножение на единичную матрицу должно дать исходный вектор
    EXPECT_FLOAT_EQ(result.x, testVector.x);
    EXPECT_FLOAT_EQ(result.y, testVector.y);
    EXPECT_FLOAT_EQ(result.z, testVector.z);
    EXPECT_FLOAT_EQ(result.w, testVector.w);
}

TEST_F(Matrix4Test, TranslationMatrix) {
    Vector4 result = translation * testVector;

    EXPECT_FLOAT_EQ(result.x, testVector.x + 1.0f);
    EXPECT_FLOAT_EQ(result.y, testVector.y + 2.0f);
    EXPECT_FLOAT_EQ(result.z, testVector.z + 3.0f);
    EXPECT_FLOAT_EQ(result.w, testVector.w);
}

TEST_F(Matrix4Test, ScaleMatrix) {
    Vector4 result = scale * testVector;

    EXPECT_FLOAT_EQ(result.x, testVector.x * 2.0f);
    EXPECT_FLOAT_EQ(result.y, testVector.y * 3.0f);
    EXPECT_FLOAT_EQ(result.z, testVector.z * 4.0f);
    EXPECT_FLOAT_EQ(result.w, testVector.w);
}

TEST_F(Matrix4Test, RotationMatrix) {
    Vector4 original(1.0f, 0.0f, 0.0f, 1.0f);
    Vector4 result = rotationZ * original;

    // Поворот на 90 градусов: (1,0,0) -> (0,1,0)
    EXPECT_NEAR(result.x, 0.0f, 1e-6f);
    EXPECT_NEAR(result.y, 1.0f, 1e-6f);
    EXPECT_NEAR(result.z, 0.0f, 1e-6f);
    EXPECT_FLOAT_EQ(result.w, 1.0f);
}

// Тесты детерминанта
TEST_F(Matrix4Test, DeterminantIdentity) {
    float det = identity.determinant();
    EXPECT_FLOAT_EQ(det, 1.0f);
}

TEST_F(Matrix4Test, DeterminantZero) {
    Matrix4 zeroMatrix;
    float det = zeroMatrix.determinant();
    EXPECT_FLOAT_EQ(det, 0.0f);
}

// Тесты обратной матрицы
TEST_F(Matrix4Test, InverseIdentity) {
    Matrix4 inv = identity.inverse();

    for (int i = 0; i < 4; ++i) {
        for (int j = 0; j < 4; ++j) {
            EXPECT_NEAR(inv(i, j), identity(i, j), 1e-6f);
        }
    }
}

TEST_F(Matrix4Test, InverseTranslation) {
    Matrix4 inv = translation.inverse();
    Matrix4 result = translation * inv;

    // Произведение матрицы на обратную должно дать единичную матрицу
    for (int i = 0; i < 4; ++i) {
        for (int j = 0; j < 4; ++j) {
            EXPECT_NEAR(result(i, j), identity(i, j), 1e-6f);
        }
    }
}

// Тесты транспонирования
TEST_F(Matrix4Test, Transpose) {
    Matrix4 transposed = testMatrix.transpose();

    for (int i = 0; i < 4; ++i) {
        for (int j = 0; j < 4; ++j) {
            EXPECT_FLOAT_EQ(transposed(i, j), testMatrix(j, i));
        }
    }
}

TEST_F(Matrix4Test, TransposeIdentity) {
    Matrix4 transposed = identity.transpose();

    for (int i = 0; i < 4; ++i) {
        for (int j = 0; j < 4; ++j) {
            EXPECT_FLOAT_EQ(transposed(i, j), identity(i, j));
        }
    }
}

// Тесты операторов присваивания
TEST_F(Matrix4Test, AssignmentOperator) {
    Matrix4 m;
    m = testMatrix;

    for (int i = 0; i < 4; ++i) {
        for (int j = 0; j < 4; ++j) {
            EXPECT_FLOAT_EQ(m(i, j), testMatrix(i, j));
        }
    }
}

TEST_F(Matrix4Test, MultiplicationAssignment) {
    Matrix4 m = identity;
    m *= testMatrix;

    for (int i = 0; i < 4; ++i) {
        for (int j = 0; j < 4; ++j) {
            EXPECT_FLOAT_EQ(m(i, j), testMatrix(i, j));
        }
    }
}

// Тесты операторов сравнения
TEST_F(Matrix4Test, Equality) {
    Matrix4 m = testMatrix;
    EXPECT_TRUE(m == testMatrix);
    EXPECT_FALSE(m == identity);
}

TEST_F(Matrix4Test, Inequality) {
    EXPECT_TRUE(testMatrix != identity);
    EXPECT_FALSE(testMatrix != testMatrix);
}

// Тесты специальных матриц
TEST_F(Matrix4Test, PerspectiveProjection) {
    float fov = 3.14159265f / 4.0f;  // 45 градусов
    float aspect = 16.0f / 9.0f;
    float near = 0.1f;
    float far = 100.0f;

    Matrix4 proj = Matrix4::perspective(fov, aspect, near, far);

    // Проверяем основные свойства проекционной матрицы
    EXPECT_FLOAT_EQ(proj(3, 3), 0.0f);
    EXPECT_FLOAT_EQ(proj(2, 3), -1.0f);
}

TEST_F(Matrix4Test, OrthographicProjection) {
    float left = -1.0f, right = 1.0f;
    float bottom = -1.0f, top = 1.0f;
    float near = 0.1f, far = 100.0f;

    Matrix4 proj = Matrix4::orthographic(left, right, bottom, top, near, far);

    // Проверяем основные свойства ортографической матрицы
    EXPECT_FLOAT_EQ(proj(3, 3), 1.0f);
    EXPECT_FLOAT_EQ(proj(0, 3), 0.0f);
    EXPECT_FLOAT_EQ(proj(1, 3), 0.0f);
    EXPECT_FLOAT_EQ(proj(2, 3), 0.0f);
}

TEST_F(Matrix4Test, LookAtMatrix) {
    Vector3 eye(0.0f, 0.0f, 5.0f);
    Vector3 center(0.0f, 0.0f, 0.0f);
    Vector3 up(0.0f, 1.0f, 0.0f);

    Matrix4 lookAt = Matrix4::lookAt(eye, center, up);

    // Проверяем, что lookAt матрица корректно ориентирует камеру
    Vector4 forward = lookAt * Vector4(0.0f, 0.0f, -1.0f, 0.0f);
    Vector4 right = lookAt * Vector4(1.0f, 0.0f, 0.0f, 0.0f);
    Vector4 upResult = lookAt * Vector4(0.0f, 1.0f, 0.0f, 0.0f);

    // Векторы должны быть ортогональны
    EXPECT_NEAR(forward.dot(right), 0.0f, 1e-6f);
    EXPECT_NEAR(forward.dot(upResult), 0.0f, 1e-6f);
    EXPECT_NEAR(right.dot(upResult), 0.0f, 1e-6f);
}

// Тесты производительности
TEST_F(Matrix4Test, PerformanceMultiplication) {
    const int iterations = 100000;

    EXPECT_PERFORMANCE_UNDER(
        {
            Matrix4 result = identity;
            for (int i = 0; i < iterations; ++i) {
                result = result * testMatrix;
                // Предотвращаем переполнение
                if (i % 1000 == 0) {
                    result = identity;
                }
            }
        },
        50);  // 100K операций за < 50ms
}

// Параметризованные тесты для различных углов поворота
class Matrix4RotationTest : public Matrix4Test, public ::testing::WithParamInterface<float> {};

TEST_P(Matrix4RotationTest, RotationX) {
    float angle = GetParam();
    Matrix4 rotX = Matrix4::rotationX(angle);

    Vector4 original(1.0f, 0.0f, 0.0f, 1.0f);
    Vector4 result = rotX * original;

    // X-компонента не должна изменяться при повороте вокруг X
    EXPECT_NEAR(result.x, 1.0f, 1e-6f);
    EXPECT_FLOAT_EQ(result.w, 1.0f);
}

TEST_P(Matrix4RotationTest, RotationY) {
    float angle = GetParam();
    Matrix4 rotY = Matrix4::rotationY(angle);

    Vector4 original(0.0f, 1.0f, 0.0f, 1.0f);
    Vector4 result = rotY * original;

    // Y-компонента не должна изменяться при повороте вокруг Y
    EXPECT_NEAR(result.y, 1.0f, 1e-6f);
    EXPECT_FLOAT_EQ(result.w, 1.0f);
}

TEST_P(Matrix4RotationTest, RotationZ) {
    float angle = GetParam();
    Matrix4 rotZ = Matrix4::rotationZ(angle);

    Vector4 original(0.0f, 0.0f, 1.0f, 1.0f);
    Vector4 result = rotZ * original;

    // Z-компонента не должна изменяться при повороте вокруг Z
    EXPECT_NEAR(result.z, 1.0f, 1e-6f);
    EXPECT_FLOAT_EQ(result.w, 1.0f);
}

INSTANTIATE_TEST_SUITE_P(RotationTests,
                         Matrix4RotationTest,
                         ::testing::Values(0.0f,
                                           3.14159265f / 6.0f,  // 30 градусов
                                           3.14159265f / 4.0f,  // 45 градусов
                                           3.14159265f / 2.0f,  // 90 градусов
                                           3.14159265f,         // 180 градусов
                                           2.0f * 3.14159265f   // 360 градусов
                                           ));
