/**
 * @file test_optix_integration.cpp
 * @brief Юнит-тесты для OptiX интеграции
 *
 * Этот файл содержит тесты для проверки корректности работы
 * OptiX ray tracer и связанных компонентов.
 */

#include <gtest/gtest.h>
#include <memory>

#ifdef VULKAN_RENDERER_OPTIX_SUPPORT
#include <cuda_runtime.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "Engine3D/OptiX/OptiXRayTracer.h"

using namespace Engine3D::OptiX;
using namespace glm;

class OptiXTest : public ::testing::Test {
  protected:
    void SetUp() override {
        // Инициализация CUDA
        cudaError_t cudaStatus = cudaSetDevice(0);
        ASSERT_EQ(cudaStatus, cudaSuccess) << "Не удалось инициализировать CUDA";

        // Получение CUDA контекста
        CUresult cuResult = cuCtxGetCurrent(&cudaContext);
        if (cuResult != CUDA_SUCCESS || cudaContext == nullptr) {
            CUdevice device;
            cuDeviceGet(&device, 0);
            cuCtxCreate(&cudaContext, 0, device);
        }

        // Создание ray tracer
        rayTracer = std::make_unique<OptiXRayTracer>();
    }

    void TearDown() override { rayTracer.reset(); }

    /**
     * @brief Создание простой тестовой геометрии
     */
    SceneGeometry createTestGeometry() {
        SceneGeometry geometry = {};

        // Простой треугольник
        static float vertices[] = {
            0.0f,
            1.0f,
            0.0f,  // Верхняя вершина
            -1.0f,
            -1.0f,
            0.0f,  // Левая нижняя
            1.0f,
            -1.0f,
            0.0f  // Правая нижняя
        };

        static uint32_t indices[] = {0, 1, 2};

        geometry.vertices = vertices;
        geometry.indices = indices;
        geometry.vertexCount = 3;
        geometry.triangleCount = 1;
        geometry.vertexStride = 3 * sizeof(float);

        return geometry;
    }

    /**
     * @brief Создание тестовых параметров запуска
     */
    LaunchParams createTestParams() {
        LaunchParams params = {};

        params.viewMatrix =
            lookAt(vec3(0.0f, 0.0f, 3.0f), vec3(0.0f, 0.0f, 0.0f), vec3(0.0f, 1.0f, 0.0f));

        params.projMatrix = perspective(radians(45.0f),
                                        1.0f,  // Квадратное изображение для тестов
                                        0.1f,
                                        100.0f);

        params.cameraPos = vec3(0.0f, 0.0f, 3.0f);
        params.lightPos = vec3(1.0f, 1.0f, 1.0f);
        params.lightColor = vec3(1.0f, 1.0f, 1.0f);
        params.lightIntensity = 1.0f;
        params.width = 256;  // Небольшой размер для быстрых тестов
        params.height = 256;
        params.maxDepth = 3;

        return params;
    }

    CUcontext cudaContext = nullptr;
    std::unique_ptr<OptiXRayTracer> rayTracer;
};

/**
 * @brief Тест инициализации OptiX ray tracer
 */
TEST_F(OptiXTest, Initialization) {
    EXPECT_TRUE(rayTracer->init(cudaContext)) << "Не удалось инициализировать OptiX ray tracer";
}

/**
 * @brief Тест построения acceleration structures
 */
TEST_F(OptiXTest, AccelerationStructureBuild) {
    ASSERT_TRUE(rayTracer->init(cudaContext));

    SceneGeometry geometry = createTestGeometry();

    // Не должно генерировать исключений
    EXPECT_NO_THROW(rayTracer->buildAccelerationStructures(geometry));
}

/**
 * @brief Тест базовой трассировки лучей
 */
TEST_F(OptiXTest, BasicRayTracing) {
    ASSERT_TRUE(rayTracer->init(cudaContext));

    SceneGeometry geometry = createTestGeometry();
    rayTracer->buildAccelerationStructures(geometry);

    LaunchParams params = createTestParams();

    RawEffects effects;
    EXPECT_NO_THROW(effects = rayTracer->traceRays(params));

    // Проверяем, что буферы созданы
    EXPECT_NE(effects.reflections, nullptr) << "Буфер отражений не создан";
    EXPECT_NE(effects.shadows, nullptr) << "Буфер теней не создан";
    EXPECT_NE(effects.globalIllumination, nullptr) << "Буфер глобального освещения не создан";
    EXPECT_NE(effects.motionVectors, nullptr) << "Буфер motion vectors не создан";
    EXPECT_NE(effects.albedo, nullptr) << "Буфер альбедо не создан";
    EXPECT_NE(effects.normals, nullptr) << "Буфер нормалей не создан";

    // Проверяем размеры
    EXPECT_EQ(effects.width, params.width);
    EXPECT_EQ(effects.height, params.height);
}

/**
 * @brief Тест применения Shader Execution Reordering
 */
TEST_F(OptiXTest, ShaderExecutionReordering) {
    ASSERT_TRUE(rayTracer->init(cudaContext));

    CoherencyHints hints = {0.5f, 0.5f, 0.5f};

    // Не должно генерировать исключений
    EXPECT_NO_THROW(rayTracer->applySER(hints));
}

/**
 * @brief Тест изменения максимальной глубины трассировки
 */
TEST_F(OptiXTest, MaxTraceDepthSetting) {
    ASSERT_TRUE(rayTracer->init(cudaContext));

    uint32_t testDepth = 5;
    EXPECT_NO_THROW(rayTracer->setMaxTraceDepth(testDepth));
}

/**
 * @brief Тест корректного завершения работы
 */
TEST_F(OptiXTest, ProperShutdown) {
    ASSERT_TRUE(rayTracer->init(cudaContext));

    SceneGeometry geometry = createTestGeometry();
    rayTracer->buildAccelerationStructures(geometry);

    LaunchParams params = createTestParams();
    rayTracer->traceRays(params);

    // Не должно генерировать исключений при завершении
    EXPECT_NO_THROW(rayTracer->shutdown());
}

/**
 * @brief Тест обработки различных размеров изображения
 */
TEST_F(OptiXTest, DifferentImageSizes) {
    ASSERT_TRUE(rayTracer->init(cudaContext));

    SceneGeometry geometry = createTestGeometry();
    rayTracer->buildAccelerationStructures(geometry);

    // Тестируем различные размеры
    std::vector<std::pair<uint32_t, uint32_t>> sizes = {
        {64, 64}, {128, 128}, {256, 128}, {512, 256}};

    for (const auto& size : sizes) {
        LaunchParams params = createTestParams();
        params.width = size.first;
        params.height = size.second;

        RawEffects effects;
        EXPECT_NO_THROW(effects = rayTracer->traceRays(params));
        EXPECT_EQ(effects.width, size.first);
        EXPECT_EQ(effects.height, size.second);
    }
}

/**
 * @brief Тест производительности базовой трассировки
 */
TEST_F(OptiXTest, PerformanceBenchmark) {
    ASSERT_TRUE(rayTracer->init(cudaContext));

    SceneGeometry geometry = createTestGeometry();
    rayTracer->buildAccelerationStructures(geometry);

    LaunchParams params = createTestParams();
    params.width = 512;
    params.height = 512;

    const int numFrames = 10;
    auto startTime = std::chrono::high_resolution_clock::now();

    for (int i = 0; i < numFrames; ++i) {
        rayTracer->traceRays(params);
    }

    auto endTime = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime);

    // Проверяем, что среднее время кадра разумно (менее 100 мс для 512x512)
    float avgFrameTime = duration.count() / float(numFrames);
    EXPECT_LT(avgFrameTime, 100.0f)
        << "Среднее время кадра слишком велико: " << avgFrameTime << " мс";

    std::cout << "Среднее время кадра: " << avgFrameTime << " мс" << std::endl;
    std::cout << "FPS: " << (1000.0f / avgFrameTime) << std::endl;
}

#else  // !VULKAN_RENDERER_OPTIX_SUPPORT

/**
 * @brief Заглушка для тестов когда OptiX не поддерживается
 */
TEST(OptiXStubTest, OptiXNotSupported) {
    GTEST_SKIP() << "OptiX поддержка не включена в сборку";
}

#endif  // VULKAN_RENDERER_OPTIX_SUPPORT
