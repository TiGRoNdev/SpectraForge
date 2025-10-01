/**
 * @file optix_raytracer_demo.cpp
 * @brief Демонстрация работы OptiX Ray Tracer
 *
 * Эта программа демонстрирует использование OptiX инфраструктуры
 * для рендеринга вторичных эффектов: отражений, теней и глобального освещения.
 */

#include <chrono>
#include <iostream>
#include <memory>
#include <vector>
#include "HyperEngine/Core/Console.h"
#include "HyperEngine/Core/SafeConsole.h"

#ifdef VULKAN_RENDERER_OPTIX_SUPPORT
#include <cuda_runtime.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "HyperEngine/OptiX/OptiXRayTracer.h"
#endif

using namespace std;
using namespace glm;
using namespace HyperEngine::Core;

#ifdef VULKAN_RENDERER_OPTIX_SUPPORT
using namespace HyperEngine::OptiX;

/**
 * @brief Создание тестовой геометрии - простой куб
 */
SceneGeometry createTestCube() {
    SceneGeometry geometry = {};

    // Вершины куба
    static float vertices[] = {
        // Передняя грань
        -1.0f,
        -1.0f,
        1.0f,  // 0
        1.0f,
        -1.0f,
        1.0f,  // 1
        1.0f,
        1.0f,
        1.0f,  // 2
        -1.0f,
        1.0f,
        1.0f,  // 3

        // Задняя грань
        -1.0f,
        -1.0f,
        -1.0f,  // 4
        1.0f,
        -1.0f,
        -1.0f,  // 5
        1.0f,
        1.0f,
        -1.0f,  // 6
        -1.0f,
        1.0f,
        -1.0f  // 7
    };

    // Индексы треугольников
    static uint32_t indices[] = {// Передняя грань
                                 0,
                                 1,
                                 2,
                                 2,
                                 3,
                                 0,
                                 // Задняя грань
                                 4,
                                 6,
                                 5,
                                 6,
                                 4,
                                 7,
                                 // Левая грань
                                 4,
                                 0,
                                 3,
                                 3,
                                 7,
                                 4,
                                 // Правая грань
                                 1,
                                 5,
                                 6,
                                 6,
                                 2,
                                 1,
                                 // Верхняя грань
                                 3,
                                 2,
                                 6,
                                 6,
                                 7,
                                 3,
                                 // Нижняя грань
                                 4,
                                 5,
                                 1,
                                 1,
                                 0,
                                 4};

    geometry.vertices = vertices;
    geometry.indices = indices;
    geometry.vertexCount = 8;
    geometry.triangleCount = 12;
    geometry.vertexStride = 3 * sizeof(float);

    return geometry;
}

/**
 * @brief Создание параметров камеры для тестирования
 */
LaunchParams createTestCameraParams(uint32_t width, uint32_t height) {
    LaunchParams params = {};

    // Настройка матриц камеры
    params.viewMatrix = lookAt(vec3(4.0f, 3.0f, 3.0f),  // Позиция камеры
                               vec3(0.0f, 0.0f, 0.0f),  // Точка взгляда
                               vec3(0.0f, 1.0f, 0.0f)   // Вектор вверх
    );

    params.projMatrix = perspective(radians(45.0f),                // FOV
                                    float(width) / float(height),  // Aspect ratio
                                    0.1f,                          // Near plane
                                    100.0f                         // Far plane
    );

    // Параметры камеры
    params.cameraPos = vec3(4.0f, 3.0f, 3.0f);

    // Параметры освещения
    params.lightPos = vec3(2.0f, 4.0f, 2.0f);
    params.lightColor = vec3(1.0f, 1.0f, 1.0f);
    params.lightIntensity = 1.0f;

    // Параметры рендеринга
    params.width = width;
    params.height = height;
    params.maxDepth = 3;

    return params;
}

/**
 * @brief Тестирование производительности OptiX
 */
void benchmarkPerformance(OptiXRayTracer& rayTracer,
                          const SceneGeometry& geometry,
                          uint32_t width,
                          uint32_t height) {
    SAFE_PRINT_LINE("\n=== ТЕСТИРОВАНИЕ ПРОИЗВОДИТЕЛЬНОСТИ ===");

    LaunchParams params = createTestCameraParams(width, height);

    const int numFrames = 100;
    auto startTime = chrono::high_resolution_clock::now();

    for (int frame = 0; frame < numFrames; ++frame) {
        // Небольшие изменения в каждом кадре для реалистичности
        params.cameraPos.x = 4.0f + 0.5f * sin(frame * 0.1f);

        RawEffects effects = rayTracer.traceRays(params);

        if (frame % 20 == 0) {
            std::cout << "Кадр " << frame << "/" << numFrames << " завершен" << std::endl;
        }
    }

    auto endTime = chrono::high_resolution_clock::now();
    auto duration = chrono::duration_cast<chrono::milliseconds>(endTime - startTime);

    float fps = float(numFrames) / (duration.count() / 1000.0f);
    float frameTime = duration.count() / float(numFrames);

    SAFE_PRINT_LINE("Результаты производительности:");
    std::cout << "  Разрешение: " << width << "x" << height << std::endl;
    std::cout << "  Кадров: " << numFrames << std::endl;
    std::cout << "  Общее время: " << duration.count() << " мс" << std::endl;
    std::cout << "  Среднее время кадра: " << frameTime << " мс" << std::endl;
    std::cout << "  FPS: " << fps << std::endl;
}

/**
 * @brief Тестирование Shader Execution Reordering
 */
void testSER(OptiXRayTracer& rayTracer) {
    SAFE_PRINT_LINE("\n=== ТЕСТИРОВАНИЕ SHADER EXECUTION REORDERING ===");

    // Тест с различными уровнями когерентности
    vector<CoherencyHints> testCases = {
        {0.1f, 0.1f, 0.1f},  // Низкая когерентность
        {0.5f, 0.5f, 0.5f},  // Средняя когерентность
        {0.9f, 0.9f, 0.9f}   // Высокая когерентность
    };

    for (size_t i = 0; i < testCases.size(); ++i) {
        const auto& hints = testCases[i];
        std::cout << "Тест " << (i + 1) << ": Когерентность лучей=" << hints.rayCoherence
                  << ", материалов=" << hints.materialCoherence
                  << ", геометрии=" << hints.geometryCoherence << std::endl;

        rayTracer.applySER(hints);

        // В реальной реализации здесь был бы замер производительности
        SAFE_PRINT_LINE("  SER настроен успешно");
    }
}

/**
 * @brief Главная функция демо
 */
int main() {
    Console::initialize();
    SAFE_PRINT_LINE("=== ДЕМОНСТРАЦИЯ OPTIX RAY TRACER ===");

    try {
        // Инициализация CUDA
        SAFE_PRINT_LINE("Инициализация CUDA...");
        cudaError_t cudaStatus = cudaSetDevice(0);
        if (cudaStatus != cudaSuccess) {
            SAFE_ERROR("Ошибка инициализации CUDA: " + string(cudaGetErrorString(cudaStatus)));
            return 1;
        }

        // Получение CUDA контекста
        CUcontext cudaContext;
        CUresult cuResult = cuCtxGetCurrent(&cudaContext);
        if (cuResult != CUDA_SUCCESS || cudaContext == nullptr) {
            // Создаем новый контекст если его нет
            CUdevice device;
            cuDeviceGet(&device, 0);
            cuCtxCreate(&cudaContext, 0, device);
        }

        SAFE_PRINT_LINE("CUDA инициализирован успешно");

        // Создание ray tracer
        SAFE_PRINT_LINE("Создание OptiX Ray Tracer...");
        OptiXRayTracer rayTracer;

        if (!rayTracer.init(cudaContext)) {
            SAFE_ERROR("Ошибка инициализации OptiX Ray Tracer");
            return 1;
        }

        SAFE_PRINT_LINE("OptiX Ray Tracer инициализирован успешно");

        // Создание тестовой геометрии
        SAFE_PRINT_LINE("Создание тестовой геометрии...");
        SceneGeometry geometry = createTestCube();

        // Построение acceleration structures
        SAFE_PRINT_LINE("Построение acceleration structures...");
        rayTracer.buildAccelerationStructures(geometry);

        // Базовое тестирование
        SAFE_PRINT_LINE("\n=== БАЗОВОЕ ТЕСТИРОВАНИЕ ===");
        LaunchParams params = createTestCameraParams(800, 600);

        SAFE_PRINT_LINE("Трассировка тестового кадра...");
        RawEffects effects = rayTracer.traceRays(params);

        SAFE_PRINT_LINE("Результаты трассировки:");
        std::cout << "  Разрешение: " << effects.width << "x" << effects.height << std::endl;
        std::cout << "  Буферы созданы: " << (effects.reflections ? "✓" : "✗") << " Отражения" << std::endl;
        std::cout << "                  " << (effects.shadows ? "✓" : "✗") << " Тени" << std::endl;
        std::cout << "                  " << (effects.globalIllumination ? "✓" : "✗")
                  << " Глобальное освещение" << std::endl;
        std::cout << "                  " << (effects.motionVectors ? "✓" : "✗")
                  << " Motion vectors" << std::endl;
        std::cout << "                  " << (effects.albedo ? "✓" : "✗") << " Альбедо" << std::endl;
        std::cout << "                  " << (effects.normals ? "✓" : "✗") << " Нормали" << std::endl;

        // Тестирование SER
        testSER(rayTracer);

        // Тестирование производительности
        benchmarkPerformance(rayTracer, geometry, 1920, 1080);

        // Тестирование различных разрешений
        SAFE_PRINT_LINE("\n=== ТЕСТИРОВАНИЕ МАСШТАБИРУЕМОСТИ ===");
        vector<pair<uint32_t, uint32_t>> resolutions = {
            {640, 480}, {1280, 720}, {1920, 1080}, {2560, 1440}};

        for (const auto& res : resolutions) {
            std::cout << "Тестирование разрешения " << res.first << "x" << res.second << "..." << std::endl;

            LaunchParams testParams = createTestCameraParams(res.first, res.second);

            auto startTime = chrono::high_resolution_clock::now();
            RawEffects testEffects = rayTracer.traceRays(testParams);
            auto endTime = chrono::high_resolution_clock::now();

            auto duration = chrono::duration_cast<chrono::milliseconds>(endTime - startTime);
            float megapixels = (res.first * res.second) / 1000000.0f;

            std::cout << "  Время: " << duration.count() << " мс, Производительность: "
                      << (megapixels / (duration.count() / 1000.0f)) << " Mpx/s" << std::endl;
        }

        SAFE_PRINT_LINE("\n=== ДЕМОНСТРАЦИЯ ЗАВЕРШЕНА УСПЕШНО ===");

    } catch (const exception& e) {
        std::cerr << "[ERROR] Ошибка во время выполнения демо: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}

#else  // !VULKAN_RENDERER_OPTIX_SUPPORT

int main() {
    Console::initialize();
    SAFE_PRINT_LINE("=== ДЕМОНСТРАЦИЯ OPTIX RAY TRACER ===");
    SAFE_PRINT_LINE("OptiX поддержка не включена в сборку.");
    SAFE_PRINT_LINE(
        "Для активации OptiX установите OptiX SDK и пересоберите проект с BUILD_WITH_OPTIX=ON");
    return 0;
}

#endif  // VULKAN_RENDERER_OPTIX_SUPPORT
