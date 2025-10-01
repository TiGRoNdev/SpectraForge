/**
 * @file optimal_renderer_demo.cpp
 * @brief Демонстрация оптимального рендерера 3D на основе алгоритма из 3DRenderer_Whitelist.md
 *
 * Этот пример показывает:
 * - Создание сцены с 3D Gaussian Splatting
 * - Гибридный рендеринг (растеризация + трассировка лучей)
 * - AI деноизинг и нейронное масштабирование
 * - Адаптивные оптимизации производительности
 * - Автоматическое определение конфигурации железа
 */

#include <iostream>
#include "HyperEngine/Core/Console.h"
#include "HyperEngine/Core/SafeConsole.h"
#include "HyperEngine/Math/Matrix4.h"
#include "HyperEngine/Math/Vector3.h"
#include "HyperEngine/Rendering/Gaussian3D.h"
#include "HyperEngine/Rendering/Mesh3D.h"
#include "HyperEngine/Rendering/OptimalRenderer3D.h"

using namespace HyperEngine::Core;
#include <chrono>
#include <memory>
#include <thread>
#include <vector>

using namespace HyperEngine::Rendering;
using namespace HyperEngine::Math;
using namespace HyperEngine::Core;

/**
 * @brief Создает демонстрационную сцену с гауссианами и мешами
 */
OptimalRenderer3D::SceneData createDemoScene() {
    Console::info("Создание демонстрационной сцены...");

    OptimalRenderer3D::SceneData scene;

    // Создаем несколько мешей для преобразования в гауссианы
    auto cube = Mesh3D::createCube(2.0f);
    auto sphere = Mesh3D::createSphere(1.5f, 32);
    auto plane = Mesh3D::createPlane(10.0f, 10.0f);

    // Создаем поля гауссианов из мешей
    auto cubeGaussians = GaussianField3D::createFromMesh(*cube, 5000);
    auto sphereGaussians = GaussianField3D::createFromMesh(*sphere, 8000);
    auto planeGaussians = GaussianField3D::createFromMesh(*plane, 3000);

    scene.gaussianFields.push_back(cubeGaussians);
    scene.gaussianFields.push_back(sphereGaussians);
    scene.gaussianFields.push_back(planeGaussians);

    // Добавляем некоторые традиционные меши для демонстрации гибридного рендеринга
    scene.meshes.push_back(Mesh3D::createCylinder(0.5f, 3.0f, 16));
    scene.meshes.push_back(Mesh3D::createCone(1.0f, 2.0f, 16));

    // Создаем трансформации
    scene.transforms.push_back(Matrix4::translation(-3.0f, 0.0f, 0.0f));  // Куб
    scene.transforms.push_back(Matrix4::translation(3.0f, 0.0f, 0.0f));   // Сфера
    scene.transforms.push_back(Matrix4::translation(0.0f, -2.0f, 0.0f));  // Плоскость
    scene.transforms.push_back(Matrix4::translation(0.0f, 2.0f, -2.0f));  // Цилиндр
    scene.transforms.push_back(Matrix4::translation(0.0f, 2.0f, 2.0f));   // Конус

    // Вычисляем границы сцены
    scene.calculateBounds();

    Console::info("Создана демо-сцена с " + SAFE_TO_STRING(scene.gaussianFields.size())
                  + " полями гауссианов и " + SAFE_TO_STRING(scene.meshes.size()) + " мешами");

    return scene;
}

/**
 * @brief Создает сложную сцену для бенчмарка
 */
OptimalRenderer3D::SceneData createBenchmarkScene() {
    Console::info("Создание сцены для бенчмарка...");

    OptimalRenderer3D::SceneData scene;

    // Создаем множество объектов для нагрузочного тестирования
    for (int i = 0; i < 10; ++i) {
        for (int j = 0; j < 10; ++j) {
            // Создаем разнообразные меши
            std::shared_ptr<Mesh3D> mesh;
            switch ((i + j) % 4) {
                case 0:
                    mesh = Mesh3D::createCube(1.0f);
                    break;
                case 1:
                    mesh = Mesh3D::createSphere(0.8f, 16);
                    break;
                case 2:
                    mesh = Mesh3D::createCylinder(0.5f, 1.5f, 12);
                    break;
                case 3:
                    mesh = Mesh3D::createCone(0.6f, 1.2f, 12);
                    break;
            }

            // Преобразуем в гауссианы с различной плотностью
            int gaussianDensity = 1000 + (i * j * 100) % 3000;
            auto gaussianField = GaussianField3D::createFromMesh(*mesh, gaussianDensity);
            scene.gaussianFields.push_back(gaussianField);

            // Также добавляем некоторые меши для гибридного рендеринга
            if ((i + j) % 3 == 0) {
                scene.meshes.push_back(mesh);

                Matrix4 transform = Matrix4::translation((i - 5) * 3.0f, 0.0f, (j - 5) * 3.0f)
                                    * Matrix4::rotationY((i + j) * 0.5f);

                scene.transforms.push_back(transform);
            }
        }
    }

    // Вычисляем границы сцены
    scene.calculateBounds();

    Console::info("Создана бенчмарк-сцена с " + SAFE_TO_STRING(scene.gaussianFields.size())
                  + " полями гауссианов и " + SAFE_TO_STRING(scene.meshes.size()) + " мешами");

    return scene;
}

/**
 * @brief Демонстрация основных возможностей оптимального рендерера
 */
void demonstrateBasicRendering() {
    Console::initialize();
    std::cout << std::endl;
    SAFE_PRINT_LINE("=== ДЕМОНСТРАЦИЯ ОСНОВНОГО РЕНДЕРИНГА ===");

    // Создаем оптимальный рендерер с автоматическим определением настроек
    OptimalRendererFactory::CreationParams params;
    params.width = 1920;
    params.height = 1080;
    params.qualityLevel = 3;
    params.targetFPS = 60.0f;
    params.enableAllEffects = true;
    params.autoOptimize = true;

    auto renderer = OptimalRendererFactory::createOptimalRenderer(params);
    if (!renderer) {
        Console::error("Ошибка создания оптимального рендерера!");
        return;
    }

    // Создаем демо-сцену
    auto scene = createDemoScene();

    // Оптимизируем сцену под текущее железо
    auto hwConfig = OptimalRendererFactory::detectHardware();
    scene.optimizeForHardware(hwConfig);

    // Настраиваем камеру
    OptimalRenderer3D::CameraParams camera;
    camera.position = Vector3(0, 2, 10);
    camera.target = scene.sceneCenter;
    camera.fov = 60.0f;
    camera.aspectRatio = static_cast<float>(params.width) / params.height;

    // Включаем профилирование
    renderer->startProfiling();

    // Рендерим несколько кадров для демонстрации
    Console::info("Рендеринг демонстрационных кадров...");

    for (int frame = 0; frame < 10; ++frame) {
        // Поворачиваем камеру вокруг сцены
        float angle = frame * 0.2f;
        camera.position = Vector3(scene.sceneCenter.x + std::cos(angle) * 15.0f,
                                  scene.sceneCenter.y + 5.0f,
                                  scene.sceneCenter.z + std::sin(angle) * 15.0f);

        // Обновляем скорость камеры для motion-based эффектов
        static Vector3 lastPos = camera.position;
        camera.velocity = camera.position - lastPos;
        camera.isMoving = camera.velocity.magnitude() > 0.1f;
        lastPos = camera.position;

        // Основной вызов рендеринга по псевдо-алгоритму
        renderer->renderOptimal3D(scene, camera, hwConfig);

        Console::info("Кадр " + SAFE_TO_STRING(frame + 1) + "/10 завершен");

        // Небольшая пауза для демонстрации
        std::this_thread::sleep_for(std::chrono::milliseconds(16));  // ~60 FPS
    }

    // Выводим финальные метрики
    const auto& metrics = renderer->getPerformanceMetrics();
    metrics.print();

    renderer->endProfiling();

    SAFE_PRINT_LINE("=== ДЕМОНСТРАЦИЯ ЗАВЕРШЕНА ===");
}

/**
 * @brief Демонстрация различных уровней качества
 */
void demonstrateQualityLevels() {
    std::cout << std::endl;
    SAFE_PRINT_LINE("=== ДЕМОНСТРАЦИЯ УРОВНЕЙ КАЧЕСТВА ===");

    auto renderer = OptimalRendererFactory::createOptimalRenderer();
    if (!renderer)
        return;

    auto scene = createDemoScene();
    auto hwConfig = OptimalRendererFactory::detectHardware();
    scene.optimizeForHardware(hwConfig);

    OptimalRenderer3D::CameraParams camera;
    camera.position = Vector3(0, 5, 12);
    camera.target = scene.sceneCenter;
    camera.fov = 60.0f;
    camera.aspectRatio = 16.0f / 9.0f;

    // Тестируем различные уровни качества
    for (int quality = 1; quality <= 5; ++quality) {
        std::cout << std::endl;
        std::cout << "--- Тестирование уровня качества " + SAFE_TO_STRING(quality) + " ---"
                  << std::endl;

        renderer->setQualityLevel(quality);

        // Рендерим несколько кадров для каждого уровня
        std::vector<float> frameTimes;
        for (int frame = 0; frame < 5; ++frame) {
            auto startTime = std::chrono::high_resolution_clock::now();

            renderer->renderOptimal3D(scene, camera, hwConfig);

            auto endTime = std::chrono::high_resolution_clock::now();
            auto duration =
                std::chrono::duration_cast<std::chrono::microseconds>(endTime - startTime);
            frameTimes.push_back(duration.count() / 1000.0f);
        }

        // Вычисляем средний frametime
        float avgFrameTime = 0.0f;
        for (float time : frameTimes)
            avgFrameTime += time;
        avgFrameTime /= frameTimes.size();

        Console::info("Уровень качества " + SAFE_TO_STRING(quality)
                      + ": средний frametime = " + SAFE_TO_STRING(avgFrameTime) + "мс" + " (~"
                      + SAFE_TO_STRING(1000.0f / avgFrameTime) + " FPS)");
    }

    SAFE_PRINT_LINE("=== ТЕСТ КАЧЕСТВА ЗАВЕРШЕН ===");
}

/**
 * @brief Демонстрация адаптивной производительности
 */
void demonstrateAdaptivePerformance() {
    std::cout << std::endl;
    SAFE_PRINT_LINE("=== ДЕМОНСТРАЦИЯ АДАПТИВНОЙ ПРОИЗВОДИТЕЛЬНОСТИ ===");

    auto renderer = OptimalRendererFactory::createOptimalRenderer();
    if (!renderer)
        return;

    // Создаем сложную сцену для нагрузки
    auto scene = createBenchmarkScene();
    auto hwConfig = OptimalRendererFactory::detectHardware();
    scene.optimizeForHardware(hwConfig);

    OptimalRenderer3D::CameraParams camera;
    camera.position = Vector3(0, 10, 20);
    camera.target = scene.sceneCenter;
    camera.fov = 75.0f;
    camera.aspectRatio = 16.0f / 9.0f;

    // Включаем адаптивное качество с высоким целевым FPS
    renderer->enableAdaptiveQuality(true);
    renderer->setPerformanceTarget(120.0f);  // Высокая планка для демонстрации адаптации
    renderer->setQualityLevel(5);  // Начинаем с максимального качества

    Console::info("Начинаем с максимального качества и целевого FPS 120...");
    Console::info("Система автоматически адаптирует качество для достижения цели.");

    // Рендерим с адаптацией
    for (int frame = 0; frame < 30; ++frame) {
        // Движение камеры для динамической нагрузки
        float angle = frame * 0.1f;
        camera.position = Vector3(scene.sceneCenter.x + std::cos(angle) * 25.0f,
                                  scene.sceneCenter.y + 8.0f + std::sin(angle * 2) * 3.0f,
                                  scene.sceneCenter.z + std::sin(angle) * 25.0f);

        renderer->renderOptimal3D(scene, camera, hwConfig);

        const auto& metrics = renderer->getPerformanceMetrics();
        float currentFPS = 1000.0f / metrics.totalFrameTime;

        if (frame % 5 == 0) {
            Console::info("Кадр " + SAFE_TO_STRING(frame)
                          + ": frametime=" + SAFE_TO_STRING(metrics.totalFrameTime)
                          + "мс, FPS=" + SAFE_TO_STRING(currentFPS)
                          + ", ускорение=" + SAFE_TO_STRING(metrics.renderingSpeedup) + "x");
        }
    }

    SAFE_PRINT_LINE("=== АДАПТИВНАЯ ПРОИЗВОДИТЕЛЬНОСТЬ ЗАВЕРШЕНА ===");
}

/**
 * @brief Полный бенчмарк системы
 */
void runFullBenchmark() {
    std::cout << std::endl;
    SAFE_PRINT_LINE("=== ПОЛНЫЙ БЕНЧМАРК СИСТЕМЫ ===");

    auto renderer = OptimalRendererFactory::createOptimalRenderer();
    if (!renderer)
        return;

    // Создаем различные сцены для тестирования
    std::vector<OptimalRenderer3D::SceneData> testScenes;

    // Простая сцена
    testScenes.push_back(createDemoScene());

    // Сложная сцена
    testScenes.push_back(createBenchmarkScene());

    const char* sceneNames[] = {"Простая сцена", "Сложная сцена"};

    for (size_t sceneIdx = 0; sceneIdx < testScenes.size(); ++sceneIdx) {
        std::cout << std::endl;
        std::cout << "--- Бенчмарк: " + std::string(sceneNames[sceneIdx]) + " ---" << std::endl;

        auto& scene = testScenes[sceneIdx];
        auto hwConfig = OptimalRendererFactory::detectHardware();
        scene.optimizeForHardware(hwConfig);

        // Тестируем различные настройки
        std::vector<int> qualityLevels = {1, 3, 5};
        std::vector<bool> effectSettings = {false, true};

        for (int quality : qualityLevels) {
            for (bool effects : effectSettings) {
                renderer->setQualityLevel(quality);
                renderer->enableGlobalIllumination(effects);
                renderer->enableReflections(effects);
                renderer->enableShadows(effects);
                renderer->enableDenoising(effects);

                std::cout << std::endl;
                Console::info("Качество " + SAFE_TO_STRING(quality) + ", эффекты "
                              + (effects ? "вкл" : "выкл") + ":");

                // Запускаем короткий бенчмарк
                renderer->runBenchmark(scene, 20);
            }
        }
    }

    SAFE_PRINT_LINE("=== ПОЛНЫЙ БЕНЧМАРК ЗАВЕРШЕН ===");
}

/**
 * @brief Демонстрация сохранения и загрузки профилей
 */
void demonstrateProfiles() {
    std::cout << std::endl;
    SAFE_PRINT_LINE("=== ДЕМОНСТРАЦИЯ ПРОФИЛЕЙ ===");

    auto renderer = OptimalRendererFactory::createOptimalRenderer();
    if (!renderer)
        return;

    // Настраиваем пользовательский профиль
    renderer->setQualityLevel(4);
    renderer->setPerformanceTarget(90.0f);
    renderer->enableGlobalIllumination(true);
    renderer->enableReflections(true);
    renderer->enableShadows(false);
    renderer->enableDenoising(true);
    renderer->enableUpscaling(true, 1.5f);

    Console::info("Настроен пользовательский профиль:");
    std::cout << "  📊 Качество: " << SAFE_TO_STRING(4) << std::endl;
    std::cout << "  🎯 Целевой FPS: " << SAFE_TO_STRING(90) << std::endl;
    SAFE_PRINT_LINE("  💡 GI: включено, Отражения: включены");
    SAFE_PRINT_LINE("  🌑 Тени: выключены, Деноизинг: включен");
    std::cout << "  📏 Масштабирование: " << SAFE_TO_STRING(1.5) << "x" << std::endl;

    // Сохраняем профиль
    renderer->saveProfile("custom_profile.txt");
    Console::info("Профиль сохранен в custom_profile.txt");

    // Изменяем настройки
    renderer->setQualityLevel(1);
    renderer->enableGlobalIllumination(false);
    renderer->enableUpscaling(false);
    Console::info("Настройки изменены для демонстрации");

    // Загружаем сохраненный профиль
    if (renderer->loadProfile("custom_profile.txt")) {
        Console::info("Профиль успешно загружен!");
        Console::info("Настройки восстановлены из файла.");
    }

    SAFE_PRINT_LINE("=== ДЕМОНСТРАЦИЯ ПРОФИЛЕЙ ЗАВЕРШЕНА ===");
}

/**
 * @brief Главная функция демонстрации
 */
int main() {
    // Инициализируем консоль с поддержкой UTF-8
    if (!Console::initialize()) {
        SAFE_ERROR("Ошибка инициализации консоли!");
        return 1;
    }

    SAFE_PRINT_LINE("=== ДЕМОНСТРАЦИЯ ОПТИМАЛЬНОГО РЕНДЕРЕРА 3D ===");
    SAFE_PRINT_LINE("Реализация псевдо-алгоритма из 3DRenderer_Whitelist.md");
    SAFE_PRINT_LINE("Интегрирует современные технологии 2025 года:");
    SAFE_PRINT_LINE("  🔹 3D Gaussian Splatting для представления сцены");
    SAFE_PRINT_LINE("  🔹 Гибридный рендеринг (растеризация + RT)");
    SAFE_PRINT_LINE("  🔹 AI деноизинг и нейронное масштабирование");
    SAFE_PRINT_LINE("  🔹 Адаптивные оптимизации производительности");
    std::cout << std::endl;

    try {
        // Запускаем различные демонстрации
        demonstrateBasicRendering();
        demonstrateQualityLevels();
        demonstrateAdaptivePerformance();
        demonstrateProfiles();

        // Запрашиваем пользователя о запуске полного бенчмарка
        std::cout << std::endl;
        std::cout << "Желаете запустить полный бенчмарк? (может занять несколько минут) [y/N]: ";
        char response;
        std::cin >> response;

        if (response == 'y' || response == 'Y') {
            runFullBenchmark();
        }

        std::cout << std::endl;
        SAFE_PRINT_LINE("=== ВСЕ ДЕМОНСТРАЦИИ ЗАВЕРШЕНЫ УСПЕШНО ===");
        SAFE_PRINT_LINE("🚀 Оптимальный рендерер готов к использованию!");

    } catch (const std::exception& e) {
        Console::error("Ошибка во время демонстрации: " + std::string(e.what()));
        return 1;
    }

    return 0;
}

/*
 * Дополнительные примеры использования:
 *
 * // Создание рендерера для конкретной конфигурации
 * OptimalRenderer3D::HardwareConfig customConfig;
 * customConfig.supportsRayTracing = true;
 * customConfig.supportsNeural = true;
 * customConfig.performanceRating = 2.5f;
 *
 * auto renderer = std::make_unique<OptimalRenderer3D>();
 * renderer->initialize(1920, 1080, customConfig);
 *
 * // Тонкая настройка эффектов
 * renderer->enableGlobalIllumination(true);
 * renderer->enableReflections(true);
 * renderer->enableShadows(true);
 * renderer->enableDenoising(true);
 * renderer->enableUpscaling(true, 2.0f);
 *
 * // Использование в игровом цикле
 * while (gameRunning) {
 *     // Обновление сцены
 *     updateScene(sceneData);
 *     updateCamera(camera);
 *
 *     // Основной рендеринг
 *     renderer->renderOptimal3D(sceneData, camera, hwConfig);
 *
 *     // Проверка производительности
 *     const auto& metrics = renderer->getPerformanceMetrics();
 *     if (metrics.totalFrameTime > targetFrameTime) {
 *         // Автоматическая адаптация или ручная корректировка
 *     }
 * }
 */
