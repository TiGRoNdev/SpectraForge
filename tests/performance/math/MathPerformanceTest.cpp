#include <benchmark/benchmark.h>
#include <random>
#include <vector>
#include "Engine3D/Math/Matrix4.h"
#include "Engine3D/Math/Quaternion.h"
#include "Engine3D/Math/Vector3.h"

using namespace Engine3D::Math;

// Глобальные переменные для тестов
std::random_device rd;
std::mt19937 gen(rd());
std::uniform_real_distribution<float> dist(-100.0f, 100.0f);

// Функции для генерации тестовых данных
Vector3 randomVector3() {
    return Vector3(dist(gen), dist(gen), dist(gen));
}

Matrix4 randomMatrix4() {
    Matrix4 m;
    for (int i = 0; i < 4; ++i) {
        for (int j = 0; j < 4; ++j) {
            m(i, j) = dist(gen);
        }
    }
    return m;
}

Quaternion randomQuaternion() {
    return Quaternion(dist(gen), dist(gen), dist(gen), dist(gen)).normalized();
}

// Бенчмарки для Vector3
static void BM_Vector3_Addition(benchmark::State& state) {
    Vector3 v1 = randomVector3();
    Vector3 v2 = randomVector3();

    for (auto _ : state) {
        Vector3 result = v1 + v2;
        benchmark::DoNotOptimize(result);
    }

    state.SetItemsProcessed(state.iterations());
}
BENCHMARK(BM_Vector3_Addition);

static void BM_Vector3_DotProduct(benchmark::State& state) {
    Vector3 v1 = randomVector3();
    Vector3 v2 = randomVector3();

    for (auto _ : state) {
        float result = v1.dot(v2);
        benchmark::DoNotOptimize(result);
    }

    state.SetItemsProcessed(state.iterations());
}
BENCHMARK(BM_Vector3_DotProduct);

static void BM_Vector3_CrossProduct(benchmark::State& state) {
    Vector3 v1 = randomVector3();
    Vector3 v2 = randomVector3();

    for (auto _ : state) {
        Vector3 result = v1.cross(v2);
        benchmark::DoNotOptimize(result);
    }

    state.SetItemsProcessed(state.iterations());
}
BENCHMARK(BM_Vector3_CrossProduct);

static void BM_Vector3_Magnitude(benchmark::State& state) {
    Vector3 v = randomVector3();

    for (auto _ : state) {
        float result = v.magnitude();
        benchmark::DoNotOptimize(result);
    }

    state.SetItemsProcessed(state.iterations());
}
BENCHMARK(BM_Vector3_Magnitude);

static void BM_Vector3_Normalization(benchmark::State& state) {
    Vector3 v = randomVector3();

    for (auto _ : state) {
        Vector3 result = v.normalized();
        benchmark::DoNotOptimize(result);
    }

    state.SetItemsProcessed(state.iterations());
}
BENCHMARK(BM_Vector3_Normalization);

static void BM_Vector3_Distance(benchmark::State& state) {
    Vector3 v1 = randomVector3();
    Vector3 v2 = randomVector3();

    for (auto _ : state) {
        float result = v1.distance(v2);
        benchmark::DoNotOptimize(result);
    }

    state.SetItemsProcessed(state.iterations());
}
BENCHMARK(BM_Vector3_Distance);

static void BM_Vector3_Lerp(benchmark::State& state) {
    Vector3 v1 = randomVector3();
    Vector3 v2 = randomVector3();
    float t = 0.5f;

    for (auto _ : state) {
        Vector3 result = v1.lerp(v2, t);
        benchmark::DoNotOptimize(result);
    }

    state.SetItemsProcessed(state.iterations());
}
BENCHMARK(BM_Vector3_Lerp);

// Бенчмарки для Matrix4
static void BM_Matrix4_Multiplication(benchmark::State& state) {
    Matrix4 m1 = randomMatrix4();
    Matrix4 m2 = randomMatrix4();

    for (auto _ : state) {
        Matrix4 result = m1 * m2;
        benchmark::DoNotOptimize(result);
    }

    state.SetItemsProcessed(state.iterations());
}
BENCHMARK(BM_Matrix4_Multiplication);

static void BM_Matrix4_VectorMultiplication(benchmark::State& state) {
    Matrix4 m = randomMatrix4();
    Vector3 v = randomVector3();

    for (auto _ : state) {
        Vector3 result = m * v;
        benchmark::DoNotOptimize(result);
    }

    state.SetItemsProcessed(state.iterations());
}
BENCHMARK(BM_Matrix4_VectorMultiplication);

static void BM_Matrix4_Transpose(benchmark::State& state) {
    Matrix4 m = randomMatrix4();

    for (auto _ : state) {
        Matrix4 result = m.transpose();
        benchmark::DoNotOptimize(result);
    }

    state.SetItemsProcessed(state.iterations());
}
BENCHMARK(BM_Matrix4_Transpose);

static void BM_Matrix4_Determinant(benchmark::State& state) {
    Matrix4 m = randomMatrix4();

    for (auto _ : state) {
        float result = m.determinant();
        benchmark::DoNotOptimize(result);
    }

    state.SetItemsProcessed(state.iterations());
}
BENCHMARK(BM_Matrix4_Determinant);

static void BM_Matrix4_Inverse(benchmark::State& state) {
    Matrix4 m = Matrix4::scaling(2.0f, 3.0f, 4.0f);  // Обратимая матрица

    for (auto _ : state) {
        Matrix4 result = m.inverse();
        benchmark::DoNotOptimize(result);
    }

    state.SetItemsProcessed(state.iterations());
}
BENCHMARK(BM_Matrix4_Inverse);

static void BM_Matrix4_Translation(benchmark::State& state) {
    Vector3 translation = randomVector3();

    for (auto _ : state) {
        Matrix4 result = Matrix4::translation(translation);
        benchmark::DoNotOptimize(result);
    }

    state.SetItemsProcessed(state.iterations());
}
BENCHMARK(BM_Matrix4_Translation);

static void BM_Matrix4_Scaling(benchmark::State& state) {
    Vector3 scale = randomVector3();

    for (auto _ : state) {
        Matrix4 result = Matrix4::scaling(scale);
        benchmark::DoNotOptimize(result);
    }

    state.SetItemsProcessed(state.iterations());
}
BENCHMARK(BM_Matrix4_Scaling);

// Бенчмарки для Quaternion
static void BM_Quaternion_Multiplication(benchmark::State& state) {
    Quaternion q1 = randomQuaternion();
    Quaternion q2 = randomQuaternion();

    for (auto _ : state) {
        Quaternion result = q1 * q2;
        benchmark::DoNotOptimize(result);
    }

    state.SetItemsProcessed(state.iterations());
}
BENCHMARK(BM_Quaternion_Multiplication);

static void BM_Quaternion_Normalization(benchmark::State& state) {
    Quaternion q = randomQuaternion();

    for (auto _ : state) {
        Quaternion result = q.normalized();
        benchmark::DoNotOptimize(result);
    }

    state.SetItemsProcessed(state.iterations());
}
BENCHMARK(BM_Quaternion_Normalization);

static void BM_Quaternion_VectorRotation(benchmark::State& state) {
    Quaternion q = randomQuaternion();
    Vector3 v = randomVector3();

    for (auto _ : state) {
        Vector3 result = q.rotate(v);
        benchmark::DoNotOptimize(result);
    }

    state.SetItemsProcessed(state.iterations());
}
BENCHMARK(BM_Quaternion_VectorRotation);

static void BM_Quaternion_Slerp(benchmark::State& state) {
    Quaternion q1 = randomQuaternion();
    Quaternion q2 = randomQuaternion();
    float t = 0.5f;

    for (auto _ : state) {
        Quaternion result = q1.slerp(q2, t);
        benchmark::DoNotOptimize(result);
    }

    state.SetItemsProcessed(state.iterations());
}
BENCHMARK(BM_Quaternion_Slerp);

static void BM_Quaternion_ToMatrix(benchmark::State& state) {
    Quaternion q = randomQuaternion();

    for (auto _ : state) {
        Matrix4 result = q.toMatrix();
        benchmark::DoNotOptimize(result);
    }

    state.SetItemsProcessed(state.iterations());
}
BENCHMARK(BM_Quaternion_ToMatrix);

static void BM_Quaternion_FromAxisAngle(benchmark::State& state) {
    Vector3 axis = Vector3::unitY();
    float angle = 1.57f;  // PI/2

    for (auto _ : state) {
        Quaternion result = Quaternion::fromAxisAngle(axis, angle);
        benchmark::DoNotOptimize(result);
    }

    state.SetItemsProcessed(state.iterations());
}
BENCHMARK(BM_Quaternion_FromAxisAngle);

// Комплексные бенчмарки для реальных сценариев использования
static void BM_TransformationChain(benchmark::State& state) {
    Vector3 position = randomVector3();
    Quaternion rotation = randomQuaternion();
    Vector3 scale = Vector3(2.0f, 2.0f, 2.0f);
    Vector3 point = randomVector3();

    for (auto _ : state) {
        // Типичная цепочка трансформаций
        Matrix4 scaleMatrix = Matrix4::scaling(scale);
        Matrix4 rotationMatrix = rotation.toMatrix();
        Matrix4 translationMatrix = Matrix4::translation(position);

        Matrix4 transform = translationMatrix * rotationMatrix * scaleMatrix;
        Vector3 result = transform * point;

        benchmark::DoNotOptimize(result);
    }

    state.SetItemsProcessed(state.iterations());
}
BENCHMARK(BM_TransformationChain);

static void BM_BulkVectorOperations(benchmark::State& state) {
    const size_t vectorCount = static_cast<size_t>(state.range(0));
    std::vector<Vector3> vectors1(vectorCount);
    std::vector<Vector3> vectors2(vectorCount);
    std::vector<Vector3> results(vectorCount);

    // Инициализация данных
    for (size_t i = 0; i < vectorCount; ++i) {
        vectors1[i] = randomVector3();
        vectors2[i] = randomVector3();
    }

    for (auto _ : state) {
        // Массовые операции с векторами
        for (size_t i = 0; i < vectorCount; ++i) {
            results[i] = vectors1[i] + vectors2[i];
        }
        benchmark::DoNotOptimize(results.data());
    }

    state.SetItemsProcessed(state.iterations() * vectorCount);
    state.SetBytesProcessed(state.iterations() * vectorCount * sizeof(Vector3) * 3);
}
BENCHMARK(BM_BulkVectorOperations)->Range(100, 10000);

static void BM_BulkMatrixMultiplication(benchmark::State& state) {
    const size_t matrixCount = static_cast<size_t>(state.range(0));
    std::vector<Matrix4> matrices1(matrixCount);
    std::vector<Matrix4> matrices2(matrixCount);
    std::vector<Matrix4> results(matrixCount);

    // Инициализация данных
    for (size_t i = 0; i < matrixCount; ++i) {
        matrices1[i] = randomMatrix4();
        matrices2[i] = randomMatrix4();
    }

    for (auto _ : state) {
        // Массовые операции с матрицами
        for (size_t i = 0; i < matrixCount; ++i) {
            results[i] = matrices1[i] * matrices2[i];
        }
        benchmark::DoNotOptimize(results.data());
    }

    state.SetItemsProcessed(state.iterations() * matrixCount);
    state.SetBytesProcessed(state.iterations() * matrixCount * sizeof(Matrix4) * 3);
}
BENCHMARK(BM_BulkMatrixMultiplication)->Range(10, 1000);

// Бенчмарк для анимационной системы
static void BM_AnimationInterpolation(benchmark::State& state) {
    const size_t frameCount = static_cast<size_t>(state.range(0));
    std::vector<Quaternion> keyframes(frameCount);
    std::vector<float> times(frameCount);

    // Создание ключевых кадров
    for (size_t i = 0; i < frameCount; ++i) {
        keyframes[i] = randomQuaternion();
        times[i] = static_cast<float>(i) / static_cast<float>(frameCount - 1);
    }

    for (auto _ : state) {
        // Интерполяция между кадрами
        for (size_t i = 0; i < frameCount - 1; ++i) {
            float t = 0.5f;  // Половина между кадрами
            Quaternion result = keyframes[i].slerp(keyframes[i + 1], t);
            benchmark::DoNotOptimize(result);
        }
    }

    state.SetItemsProcessed(state.iterations() * (frameCount - 1));
}
BENCHMARK(BM_AnimationInterpolation)->Range(10, 1000);

// Бенчмарк для физических расчетов
static void BM_PhysicsCalculations(benchmark::State& state) {
    Vector3 position = randomVector3();
    Vector3 velocity = randomVector3();
    Vector3 acceleration = Vector3(0.0f, -9.81f, 0.0f);  // Гравитация
    float deltaTime = 1.0f / 60.0f;                      // 60 FPS

    for (auto _ : state) {
        // Интеграция Эйлера
        velocity = velocity + acceleration * deltaTime;
        position = position + velocity * deltaTime;

        // Расчет новой скорости с учетом затухания
        velocity = velocity * 0.99f;

        benchmark::DoNotOptimize(position);
        benchmark::DoNotOptimize(velocity);
    }

    state.SetItemsProcessed(state.iterations());
}
BENCHMARK(BM_PhysicsCalculations);

// Кастомная main функция для настройки бенчмарков
int main(int argc, char** argv) {
    // Настройка бенчмарков
    benchmark::Initialize(&argc, argv);

    // Информация о системе
    benchmark::AddCustomContext("CPU", "Test CPU");
    benchmark::AddCustomContext("Math Library", "HyperEngine Math");

    if (benchmark::ReportUnrecognizedArguments(argc, argv)) {
        return 1;
    }

    benchmark::RunSpecifiedBenchmarks();
    benchmark::Shutdown();

    return 0;
}
