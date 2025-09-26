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

#include "Engine3D/Rendering/OptimalRenderer3D.h"
#include "Engine3D/Rendering/Gaussian3D.h"
#include "Engine3D/Rendering/Mesh3D.h"
#include "Engine3D/Math/Vector3.h"
#include "Engine3D/Math/Matrix4.h"
#include <iostream>
#include <memory>
#include <vector>
#include <chrono>
#include <thread>

using namespace Engine3D::Rendering;
using namespace Engine3D::Math;

/**
 * @brief Создает демонстрационную сцену с гауссианами и мешами
 */
OptimalRenderer3D::SceneData createDemoScene() {
    std::cout << "Создание демонстрационной сцены..." << std::endl;
    
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
    scene.transforms.push_back(Matrix4::translation(-3.0f, 0.0f, 0.0f)); // Куб
    scene.transforms.push_back(Matrix4::translation(3.0f, 0.0f, 0.0f));  // Сфера
    scene.transforms.push_back(Matrix4::translation(0.0f, -2.0f, 0.0f)); // Плоскость
    scene.transforms.push_back(Matrix4::translation(0.0f, 2.0f, -2.0f)); // Цилиндр
    scene.transforms.push_back(Matrix4::translation(0.0f, 2.0f, 2.0f));  // Конус
    
    // Вычисляем границы сцены
    scene.calculateBounds();
    
    std::cout << "Создана демо-сцена с " << scene.gaussianFields.size() 
              << " полями гауссианов и " << scene.meshes.size() << " мешами" << std::endl;
    
    return scene;
}

/**
 * @brief Создает сложную сцену для бенчмарка
 */
OptimalRenderer3D::SceneData createBenchmarkScene() {
    std::cout << "Создание сцены для бенчмарка..." << std::endl;
    
    OptimalRenderer3D::SceneData scene;
    
    // Создаем множество объектов для нагрузочного тестирования
    for (int i = 0; i < 10; ++i) {
        for (int j = 0; j < 10; ++j) {
            // Создаем разнообразные меши
            std::shared_ptr<Mesh3D> mesh;
            switch ((i + j) % 4) {
                case 0: mesh = Mesh3D::createCube(1.0f); break;
                case 1: mesh = Mesh3D::createSphere(0.8f, 16); break;
                case 2: mesh = Mesh3D::createCylinder(0.5f, 1.5f, 12); break;
                case 3: mesh = Mesh3D::createCone(0.6f, 1.2f, 12); break;
            }
            
            // Преобразуем в гауссианы с различной плотностью
            int gaussianDensity = 1000 + (i * j * 100) % 3000;
            auto gaussianField = GaussianField3D::createFromMesh(*mesh, gaussianDensity);
            scene.gaussianFields.push_back(gaussianField);
            
            // Также добавляем некоторые меши для гибридного рендеринга
            if ((i + j) % 3 == 0) {
                scene.meshes.push_back(mesh);
                
                Matrix4 transform = Matrix4::translation(
                    (i - 5) * 3.0f, 
                    0.0f, 
                    (j - 5) * 3.0f
                ) * Matrix4::rotationY((i + j) * 0.5f);
                
                scene.transforms.push_back(transform);
            }
        }
    }
    
    // Вычисляем границы сцены
    scene.calculateBounds();
    
    std::cout << "Создана бенчмарк-сцена с " << scene.gaussianFields.size() 
              << " полями гауссианов и " << scene.meshes.size() << " мешами" << std::endl;
    
    return scene;
}

/**
 * @brief Демонстрация основных возможностей оптимального рендерера
 */
void demonstrateBasicRendering() {
    std::cout << "\n=== ДЕМОНСТРАЦИЯ ОСНОВНОГО РЕНДЕРИНГА ===" << std::endl;
    
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
        std::cerr << "Ошибка создания оптимального рендерера!" << std::endl;
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
    std::cout << "\nРендеринг демонстрационных кадров..." << std::endl;
    
    for (int frame = 0; frame < 10; ++frame) {
        // Поворачиваем камеру вокруг сцены
        float angle = frame * 0.2f;
        camera.position = Vector3(
            scene.sceneCenter.x + std::cos(angle) * 15.0f,
            scene.sceneCenter.y + 5.0f,
            scene.sceneCenter.z + std::sin(angle) * 15.0f
        );
        
        // Обновляем скорость камеры для motion-based эффектов
        static Vector3 lastPos = camera.position;
        camera.velocity = camera.position - lastPos;
        camera.isMoving = camera.velocity.magnitude() > 0.1f;
        lastPos = camera.position;
        
        // Основной вызов рендеринга по псевдо-алгоритму
        renderer->renderOptimal3D(scene, camera, hwConfig);
        
        std::cout << "Кадр " << (frame + 1) << "/10 завершен" << std::endl;
        
        // Небольшая пауза для демонстрации
        std::this_thread::sleep_for(std::chrono::milliseconds(16)); // ~60 FPS
    }
    
    // Выводим финальные метрики
    const auto& metrics = renderer->getPerformanceMetrics();
    metrics.print();
    
    renderer->endProfiling();
    
    std::cout << "=== ДЕМОНСТРАЦИЯ ЗАВЕРШЕНА ===" << std::endl;
}

/**
 * @brief Демонстрация различных уровней качества
 */
void demonstrateQualityLevels() {
    std::cout << "\n=== ДЕМОНСТРАЦИЯ УРОВНЕЙ КАЧЕСТВА ===" << std::endl;
    
    auto renderer = OptimalRendererFactory::createOptimalRenderer();
    if (!renderer) return;
    
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
        std::cout << "\n--- Тестирование уровня качества " << quality << " ---" << std::endl;
        
        renderer->setQualityLevel(quality);
        
        // Рендерим несколько кадров для каждого уровня
        std::vector<float> frameTimes;
        for (int frame = 0; frame < 5; ++frame) {
            auto startTime = std::chrono::high_resolution_clock::now();
            
            renderer->renderOptimal3D(scene, camera, hwConfig);
            
            auto endTime = std::chrono::high_resolution_clock::now();
            auto duration = std::chrono::duration_cast<std::chrono::microseconds>(endTime - startTime);
            frameTimes.push_back(duration.count() / 1000.0f);
        }
        
        // Вычисляем средний frametime
        float avgFrameTime = 0.0f;
        for (float time : frameTimes) avgFrameTime += time;
        avgFrameTime /= frameTimes.size();
        
        std::cout << "Уровень качества " << quality 
                  << ": средний frametime = " << avgFrameTime << "мс"
                  << " (~" << (1000.0f / avgFrameTime) << " FPS)" << std::endl;
    }
    
    std::cout << "=== ТЕСТ КАЧЕСТВА ЗАВЕРШЕН ===" << std::endl;
}

/**
 * @brief Демонстрация адаптивной производительности
 */
void demonstrateAdaptivePerformance() {
    std::cout << "\n=== ДЕМОНСТРАЦИЯ АДАПТИВНОЙ ПРОИЗВОДИТЕЛЬНОСТИ ===" << std::endl;
    
    auto renderer = OptimalRendererFactory::createOptimalRenderer();
    if (!renderer) return;
    
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
    renderer->setPerformanceTarget(120.0f); // Высокая планка для демонстрации адаптации
    renderer->setQualityLevel(5); // Начинаем с максимального качества
    
    std::cout << "Начинаем с максимального качества и целевого FPS 120..." << std::endl;
    std::cout << "Система автоматически адаптирует качество для достижения цели." << std::endl;
    
    // Рендерим с адаптацией
    for (int frame = 0; frame < 30; ++frame) {
        // Движение камеры для динамической нагрузки
        float angle = frame * 0.1f;
        camera.position = Vector3(
            scene.sceneCenter.x + std::cos(angle) * 25.0f,
            scene.sceneCenter.y + 8.0f + std::sin(angle * 2) * 3.0f,
            scene.sceneCenter.z + std::sin(angle) * 25.0f
        );
        
        renderer->renderOptimal3D(scene, camera, hwConfig);
        
        const auto& metrics = renderer->getPerformanceMetrics();
        float currentFPS = 1000.0f / metrics.totalFrameTime;
        
        if (frame % 5 == 0) {
            std::cout << "Кадр " << frame << ": frametime=" << metrics.totalFrameTime 
                      << "мс, FPS=" << currentFPS
                      << ", ускорение=" << metrics.renderingSpeedup << "x" << std::endl;
        }
    }
    
    std::cout << "=== АДАПТИВНАЯ ПРОИЗВОДИТЕЛЬНОСТЬ ЗАВЕРШЕНА ===" << std::endl;
}

/**
 * @brief Полный бенчмарк системы
 */
void runFullBenchmark() {
    std::cout << "\n=== ПОЛНЫЙ БЕНЧМАРК СИСТЕМЫ ===" << std::endl;
    
    auto renderer = OptimalRendererFactory::createOptimalRenderer();
    if (!renderer) return;
    
    // Создаем различные сцены для тестирования
    std::vector<OptimalRenderer3D::SceneData> testScenes;
    
    // Простая сцена
    testScenes.push_back(createDemoScene());
    
    // Сложная сцена
    testScenes.push_back(createBenchmarkScene());
    
    const char* sceneNames[] = {"Простая сцена", "Сложная сцена"};
    
    for (size_t sceneIdx = 0; sceneIdx < testScenes.size(); ++sceneIdx) {
        std::cout << "\n--- Бенчмарк: " << sceneNames[sceneIdx] << " ---" << std::endl;
        
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
                
                std::cout << "\nКачество " << quality 
                          << ", эффекты " << (effects ? "вкл" : "выкл") << ":" << std::endl;
                
                // Запускаем короткий бенчмарк
                renderer->runBenchmark(scene, 20);
            }
        }
    }
    
    std::cout << "=== ПОЛНЫЙ БЕНЧМАРК ЗАВЕРШЕН ===" << std::endl;
}

/**
 * @brief Демонстрация сохранения и загрузки профилей
 */
void demonstrateProfiles() {
    std::cout << "\n=== ДЕМОНСТРАЦИЯ ПРОФИЛЕЙ ===" << std::endl;
    
    auto renderer = OptimalRendererFactory::createOptimalRenderer();
    if (!renderer) return;
    
    // Настраиваем пользовательский профиль
    renderer->setQualityLevel(4);
    renderer->setPerformanceTarget(90.0f);
    renderer->enableGlobalIllumination(true);
    renderer->enableReflections(true);
    renderer->enableShadows(false);
    renderer->enableDenoising(true);
    renderer->enableUpscaling(true, 1.5f);
    
    std::cout << "Настроен пользовательский профиль:" << std::endl;
    std::cout << "  Качество: 4" << std::endl;
    std::cout << "  Целевой FPS: 90" << std::endl;
    std::cout << "  GI: включено, Отражения: включены" << std::endl;
    std::cout << "  Тени: выключены, Деноизинг: включен" << std::endl;
    std::cout << "  Масштабирование: 1.5x" << std::endl;
    
    // Сохраняем профиль
    renderer->saveProfile("custom_profile.txt");
    std::cout << "\nПрофиль сохранен в custom_profile.txt" << std::endl;
    
    // Изменяем настройки
    renderer->setQualityLevel(1);
    renderer->enableGlobalIllumination(false);
    renderer->enableUpscaling(false);
    std::cout << "Настройки изменены для демонстрации" << std::endl;
    
    // Загружаем сохраненный профиль
    if (renderer->loadProfile("custom_profile.txt")) {
        std::cout << "Профиль успешно загружен!" << std::endl;
        std::cout << "Настройки восстановлены из файла." << std::endl;
    }
    
    std::cout << "=== ДЕМОНСТРАЦИЯ ПРОФИЛЕЙ ЗАВЕРШЕНА ===" << std::endl;
}

/**
 * @brief Главная функция демонстрации
 */
int main() {
    std::cout << "=== ДЕМОНСТРАЦИЯ ОПТИМАЛЬНОГО РЕНДЕРЕРА 3D ===" << std::endl;
    std::cout << "Реализация псевдо-алгоритма из 3DRenderer_Whitelist.md" << std::endl;
    std::cout << "Интегрирует современные технологии 2025 года:\n"
              << "  - 3D Gaussian Splatting для представления сцены\n"
              << "  - Гибридный рендеринг (растеризация + RT)\n"
              << "  - AI деноизинг и нейронное масштабирование\n"
              << "  - Адаптивные оптимизации производительности\n" << std::endl;
    
    try {
        // Запускаем различные демонстрации
        demonstrateBasicRendering();
        demonstrateQualityLevels();
        demonstrateAdaptivePerformance();
        demonstrateProfiles();
        
        // Запрашиваем пользователя о запуске полного бенчмарка
        std::cout << "\nЖелаете запустить полный бенчмарк? (может занять несколько минут) [y/N]: ";
        char response;
        std::cin >> response;
        
        if (response == 'y' || response == 'Y') {
            runFullBenchmark();
        }
        
        std::cout << "\n=== ВСЕ ДЕМОНСТРАЦИИ ЗАВЕРШЕНЫ УСПЕШНО ===" << std::endl;
        std::cout << "Оптимальный рендерер готов к использованию!" << std::endl;
        
    } catch (const std::exception& e) {
        std::cerr << "Ошибка во время демонстрации: " << e.what() << std::endl;
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
