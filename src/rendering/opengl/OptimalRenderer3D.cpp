#include "HyperEngine/Rendering/OptimalRenderer3D.h"
#include <algorithm>
#include <chrono>
#include <cmath>
#include <fstream>
#include <functional>
#include <iostream>
#include "HyperEngine/Core/Console.h"
#include "HyperEngine/Core/SafeConsole.h"
#include "HyperEngine/Math/Matrix4.h"
#include "HyperEngine/Rendering/Camera3D.h"

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

using namespace HyperEngine::Math;
using namespace HyperEngine::Rendering;
using namespace HyperEngine::Core;

namespace HyperEngine {
namespace Rendering {

// === HardwareConfig Implementation ===

void OptimalRenderer3D::HardwareConfig::autoDetect() {
    SAFE_PRINT_LINE("Автоматическое определение конфигурации оборудования...");

    // TODO: Реальное определение возможностей GPU
    // Сейчас используем примерные значения для демонстрации

    // Примерное определение поддержки RT
    supportsRayTracing = (performanceRating > 1.5f);

    // Примерное определение поддержки нейронных операций
    supportsNeural = (gpuMemoryMB >= 6144 && performanceRating > 1.0f);

    // Mesh шейдеры для современных GPU
    supportsMeshShaders = (performanceRating > 1.2f);

    // Variable Rate Shading
    supportsVRS = (performanceRating > 1.0f);

    std::cout << "Определена конфигурация: RT=" << (supportsRayTracing ? "Y" : "N") << ", Neural="
              << (supportsNeural ? "Y" : "N") << ", MeshShaders="
              << (supportsMeshShaders ? "Y" : "N") << ", VRS=" << (supportsVRS ? "Y" : "N")
              << ", Memory=" << gpuMemoryMB << "MB, Performance=" << performanceRating
              << std::endl;
}

// === SceneData Implementation ===

void OptimalRenderer3D::SceneData::calculateBounds() {
    if (gaussianFields.empty() && meshes.empty()) {
        sceneCenter = Vector3(0, 0, 0);
        sceneRadius = 10.0f;
        return;
    }

    Vector3 minBounds(1e6f, 1e6f, 1e6f);
    Vector3 maxBounds(-1e6f, -1e6f, -1e6f);

    // Обработка гауссианов
    for (const auto& field : gaussianFields) {
        if (field) {
            for (const auto& gaussian : field->getGaussians()) {
                Vector3 gaussianMin = gaussian.position - gaussian.scale * 3.0f;
                Vector3 gaussianMax = gaussian.position + gaussian.scale * 3.0f;

                minBounds = Vector3(std::min(minBounds.x, gaussianMin.x),
                                    std::min(minBounds.y, gaussianMin.y),
                                    std::min(minBounds.z, gaussianMin.z));
                maxBounds = Vector3(std::max(maxBounds.x, gaussianMax.x),
                                    std::max(maxBounds.y, gaussianMax.y),
                                    std::max(maxBounds.z, gaussianMax.z));
            }
        }
    }

    // Обработка мешей
    for (size_t i = 0; i < meshes.size() && i < transforms.size(); ++i) {
        if (meshes[i]) {
            Vector3 meshCenter = meshes[i]->getCenter();
            Vector3 meshSize = meshes[i]->getSize();

            // Трансформируем bounds
            Vector3 transformedCenter = transforms[i].transformPoint(meshCenter);
            Vector3 transformedSize = transforms[i].transformVector(meshSize);

            Vector3 meshMin = transformedCenter - transformedSize * 0.5f;
            Vector3 meshMax = transformedCenter + transformedSize * 0.5f;

            minBounds = Vector3(std::min(minBounds.x, meshMin.x),
                                std::min(minBounds.y, meshMin.y),
                                std::min(minBounds.z, meshMin.z));
            maxBounds = Vector3(std::max(maxBounds.x, meshMax.x),
                                std::max(maxBounds.y, meshMax.y),
                                std::max(maxBounds.z, meshMax.z));
        }
    }

    sceneCenter = (minBounds + maxBounds) * 0.5f;
    sceneRadius = (maxBounds - minBounds).magnitude() * 0.5f;

    std::cout << "Границы сцены: центр(" << sceneCenter.x << ", " << sceneCenter.y << ", "
              << sceneCenter.z << "), радиус=" << sceneRadius << std::endl;
}

void OptimalRenderer3D::SceneData::optimizeForHardware(const HardwareConfig& config) {
    SAFE_PRINT_LINE("Оптимизация сцены для конфигурации оборудования...");

    // Определяем уровень сложности на основе количества объектов
    size_t totalGaussians = 0;
    for (const auto& field : gaussianFields) {
        if (field)
            totalGaussians += field->getGaussianCount();
    }

    size_t totalTriangles = 0;
    for (const auto& mesh : meshes) {
        if (mesh)
            totalTriangles += mesh->getTriangleCount();
    }

    // Классифицируем сложность сцены
    if (totalGaussians > 100000 || totalTriangles > 1000000) {
        complexityLevel = 5;  // Очень сложная
    } else if (totalGaussians > 50000 || totalTriangles > 500000) {
        complexityLevel = 4;  // Сложная
    } else if (totalGaussians > 20000 || totalTriangles > 200000) {
        complexityLevel = 3;  // Средняя
    } else if (totalGaussians > 5000 || totalTriangles > 50000) {
        complexityLevel = 2;  // Простая
    } else {
        complexityLevel = 1;  // Очень простая
    }

    std::cout << "Уровень сложности сцены: " << complexityLevel << " (" << totalGaussians
              << " гауссианов, " << totalTriangles << " треугольников)" << std::endl;

    // Адаптируем количество точек обзора для оптимизации
    int viewPointCount =
        std::max(1, std::min(8, static_cast<int>(config.performanceRating * complexityLevel)));

    if (multiViewImages.size() != static_cast<size_t>(viewPointCount)) {
        multiViewImages.clear();

        // Генерируем точки обзора вокруг центра сцены
        for (int i = 0; i < viewPointCount; ++i) {
            float angle = (i * 2.0f * M_PI) / viewPointCount;
            float distance = sceneRadius * 2.0f;

            Vector3 viewPoint(sceneCenter.x + std::cos(angle) * distance,
                              sceneCenter.y + sceneRadius * 0.5f,
                              sceneCenter.z + std::sin(angle) * distance);

            multiViewImages.push_back(viewPoint);
        }
    }

    std::cout << "Создано " << multiViewImages.size() << " точек обзора для оптимизации"
              << std::endl;
}

// === OptimalRenderer3D Implementation ===

OptimalRenderer3D::OptimalRenderer3D()
    : initialized(false),
      renderWidth(1920),
      renderHeight(1080),
      targetWidth(1920),
      targetHeight(1080),
      qualityLevel(3),
      targetFPS(60.0f),
      adaptiveQualityEnabled(true),
      globalIlluminationEnabled(true),
      reflectionsEnabled(true),
      shadowsEnabled(true),
      denoisingEnabled(true),
      upscalingEnabled(false),
      upscalingFactor(1.0f),
      profilingEnabled(false) {
    metrics.reset();
    optimizationCache.invalidate();

    SAFE_PRINT_LINE("Создан оптимальный рендерер 3D");
}

OptimalRenderer3D::~OptimalRenderer3D() {
    cleanup();
}

bool OptimalRenderer3D::initialize(int width, int height, const HardwareConfig& hwConfig) {
    if (initialized) {
        SAFE_PRINT_LINE("Оптимальный рендерер уже инициализирован");
        return true;
    }

    std::cout << "Инициализация оптимального рендерера (" << width << "x" << height << ")..."
              << std::endl;

    currentHardware = hwConfig;
    targetWidth = width;
    targetHeight = height;

    // Адаптируем разрешение рендеринга под железо
    SAFE_PRINT_LINE("Вызов adaptToHardware()...");
    adaptToHardware();
    SAFE_PRINT_LINE("adaptToHardware() завершен");

    // Создаем гибридный рендерер
    SAFE_PRINT_LINE("Создание HybridRenderer3D...");
    hybridRenderer = std::make_unique<HybridRenderer3D>();
    SAFE_PRINT_LINE("HybridRenderer3D создан, вызов initialize...");
    if (!hybridRenderer->initialize(renderWidth, renderHeight)) {
        SAFE_ERROR("Ошибка инициализации гибридного рендерера!");
        return false;
    }

    // Создаем систему глобального освещения
    if (currentHardware.supportsRayTracing) {
        globalIllumination = std::make_unique<ReSTIRGlobalIllumination>();
        globalIllumination->initialize(renderWidth, renderHeight);
    }

    // Создаем AI деноизер
    if (currentHardware.supportsNeural || denoisingEnabled) {
        denoiser = std::make_unique<AIDenoiser>();
        denoiser->initialize();
    }

    // Создаем нейронный масштабировщик
    if (currentHardware.supportsNeural && upscalingEnabled) {
        upscaler = std::make_unique<NeuralUpscaler>();
        upscaler->initialize();
    }

    initialized = true;

    std::cout << "Оптимальный рендерер успешно инициализирован:\n  Разрешение рендеринга: "
              << renderWidth << "x" << renderHeight << "\n  Целевое разрешение: " << targetWidth
              << "x" << targetHeight
              << "\n  Глобальное освещение: " << (globalIllumination ? "Да" : "Нет")
              << "\n  AI деноизинг: " << (denoiser ? "Да" : "Нет")
              << "\n  Нейронное масштабирование: " << (upscaler ? "Да" : "Нет") << std::endl;

    return true;
}

void OptimalRenderer3D::cleanup() {
    if (!initialized)
        return;

    SAFE_PRINT_LINE("Очистка оптимального рендерера...");

    // Очищаем компоненты
    if (upscaler) {
        upscaler->cleanup();
        upscaler.reset();
    }

    if (denoiser) {
        denoiser->cleanup();
        denoiser.reset();
    }

    if (globalIllumination) {
        globalIllumination->cleanup();
        globalIllumination.reset();
    }

    if (hybridRenderer) {
        hybridRenderer->cleanup();
        hybridRenderer.reset();
    }

    optimizationCache.invalidate();
    initialized = false;

    SAFE_PRINT_LINE("Оптимальный рендерер очищен");
}

void OptimalRenderer3D::renderOptimal3D(const SceneData& sceneData,
                                        const CameraParams& cameraParams,
                                        const HardwareConfig& hardwareConfig) {
    if (!initialized) {
        SAFE_ERROR("Оптимальный рендерер не инициализирован!");
        return;
    }

    if (profilingEnabled) {
        SAFE_PRINT_LINE("\n=== НАЧАЛО ОПТИМАЛЬНОГО РЕНДЕРИНГА ===");
    }

    auto frameStartTime = std::chrono::high_resolution_clock::now();

    // Копируем данные для возможной модификации
    SceneData workingSceneData = sceneData;
    lastCameraParams = cameraParams;

    // Этап 1: Scene Representation Optimization
    metrics.sceneOptimizationTime = measureExecutionTime(
        [&]() { optimizeSceneRepresentation(workingSceneData, hardwareConfig); });

    // Опциональная интеграция NeRF
    if (hardwareConfig.supportsNeural) {
        metrics.sceneOptimizationTime +=
            measureExecutionTime([&]() { integrateNeRFVariants(workingSceneData); });
    }

    // Этап 2: Geometry and Primary Visibility
    metrics.rasterizationTime = measureExecutionTime(
        [&]() { executeGeometryAndVisibility(workingSceneData, cameraParams); });

    // Этап 3: Advanced Lighting Computation
    metrics.rayTracingTime =
        measureExecutionTime([&]() { executeAdvancedLighting(workingSceneData); });

    // Этап 4: Denoising and Refinement
    metrics.denoisingTime = measureExecutionTime([&]() { executeDenoisingAndRefinement(); });

    // Этап 5: Post-Processing and Output
    metrics.upscalingTime =
        measureExecutionTime([&]() { executePostProcessingAndOutput(targetWidth, targetHeight); });

    // Обновляем метрики
    auto frameEndTime = std::chrono::high_resolution_clock::now();
    auto frameDuration =
        std::chrono::duration_cast<std::chrono::microseconds>(frameEndTime - frameStartTime);
    metrics.totalFrameTime = frameDuration.count() / 1000.0f;

    updateMetrics();

    // Адаптируем качество если включено
    if (adaptiveQualityEnabled) {
        optimizeForFrameRate();
    }

    if (profilingEnabled) {
        metrics.print();
        SAFE_PRINT_LINE("=== КОНЕЦ ОПТИМАЛЬНОГО РЕНДЕРИНГА ===\n");
    }
}

void OptimalRenderer3D::setQualityLevel(int level) {
    qualityLevel = std::max(1, std::min(5, level));
    std::cout << "Установлен уровень качества: " << qualityLevel << std::endl;

    // Адаптируем настройки рендеринга под уровень качества
    if (hybridRenderer) {
        HybridRenderer3D::RenderSettings settings = hybridRenderer->getRenderSettings();

        switch (qualityLevel) {
            case 1:  // Low
                settings.gaussianQuality = 0.3f;
                settings.rayTracingSamples = 1;
                settings.enableRayTracedReflections = false;
                settings.enableNeuralUpscaling = false;
                break;
            case 2:  // Medium
                settings.gaussianQuality = 0.5f;
                settings.rayTracingSamples = 2;
                settings.enableRayTracedReflections = false;
                settings.enableNeuralUpscaling = false;
                break;
            case 3:  // High
                settings.gaussianQuality = 0.7f;
                settings.rayTracingSamples = 4;
                settings.enableRayTracedReflections = true;
                settings.enableNeuralUpscaling = upscalingEnabled;
                break;
            case 4:  // Ultra
                settings.gaussianQuality = 0.9f;
                settings.rayTracingSamples = 8;
                settings.enableRayTracedReflections = true;
                settings.enableNeuralUpscaling = upscalingEnabled;
                break;
            case 5:  // Extreme
                settings.gaussianQuality = 1.0f;
                settings.rayTracingSamples = 16;
                settings.enableRayTracedReflections = true;
                settings.enableNeuralUpscaling = upscalingEnabled;
                break;
        }

        hybridRenderer->setRenderSettings(settings);
    }
}

void OptimalRenderer3D::setPerformanceTarget(float fps) {
    targetFPS = fps;
    std::cout << "Установлен целевой FPS: " << fps << std::endl;

    if (hybridRenderer) {
        hybridRenderer->setTargetFrameRate(fps);
    }
}

void OptimalRenderer3D::enableAdaptiveQuality(bool enable) {
    adaptiveQualityEnabled = enable;
    std::cout << "Адаптивное качество " << (enable ? "включено" : "выключено") << std::endl;

    if (hybridRenderer) {
        hybridRenderer->enableDynamicQuality(enable);
    }
}

// Методы управления эффектами

void OptimalRenderer3D::enableGlobalIllumination(bool enable) {
    globalIlluminationEnabled = enable;
    std::cout << "Глобальное освещение " << (enable ? "включено" : "выключено") << std::endl;

    if (hybridRenderer) {
        auto settings = hybridRenderer->getRenderSettings();
        settings.enableRayTracedGI = enable;
        hybridRenderer->setRenderSettings(settings);
    }
}

void OptimalRenderer3D::enableReflections(bool enable) {
    reflectionsEnabled = enable;
    std::cout << "Отражения " << (enable ? "включены" : "выключены") << std::endl;

    if (hybridRenderer) {
        auto settings = hybridRenderer->getRenderSettings();
        settings.enableRayTracedReflections = enable;
        hybridRenderer->setRenderSettings(settings);
    }
}

void OptimalRenderer3D::enableShadows(bool enable) {
    shadowsEnabled = enable;
    std::cout << "Тени " << (enable ? "включены" : "выключены") << std::endl;

    if (hybridRenderer) {
        auto settings = hybridRenderer->getRenderSettings();
        settings.enableRayTracedShadows = enable;
        hybridRenderer->setRenderSettings(settings);
    }
}

void OptimalRenderer3D::enableDenoising(bool enable) {
    denoisingEnabled = enable;
    std::cout << "Деноизинг " << (enable ? "включен" : "выключен") << std::endl;

    if (hybridRenderer) {
        auto settings = hybridRenderer->getRenderSettings();
        settings.enableDenoising = enable;
        hybridRenderer->setRenderSettings(settings);
    }
}

void OptimalRenderer3D::enableUpscaling(bool enable, float factor) {
    upscalingEnabled = enable;
    upscalingFactor = factor;
    std::cout << "Масштабирование " << (enable ? "включено" : "выключено")
              << " (коэффициент: " << factor << ")" << std::endl;

    if (enable && factor > 1.0f) {
        // Уменьшаем разрешение рендеринга для последующего масштабирования
        renderWidth = static_cast<int>(targetWidth / factor);
        renderHeight = static_cast<int>(targetHeight / factor);
    } else {
        renderWidth = targetWidth;
        renderHeight = targetHeight;
    }

    if (hybridRenderer) {
        auto settings = hybridRenderer->getRenderSettings();
        settings.enableNeuralUpscaling = enable;
        settings.upscalingFactor = factor;
        hybridRenderer->setRenderSettings(settings);
    }
}

// Protected методы - этапы псевдо-алгоритма

void OptimalRenderer3D::optimizeSceneRepresentation(SceneData& sceneData,
                                                    const HardwareConfig& hwConfig) {
    SAFE_PRINT_LINE("Этап 1: Оптимизация представления сцены...");

    // Проверяем кэш оптимизации
    bool cacheValid =
        optimizationCache.cacheValid
        && optimizationCache.lastViewPoints.size() == sceneData.multiViewImages.size();

    if (cacheValid) {
        for (size_t i = 0; i < sceneData.multiViewImages.size(); ++i) {
            float distance =
                (sceneData.multiViewImages[i] - optimizationCache.lastViewPoints[i]).magnitude();
            if (distance > sceneData.sceneRadius * 0.1f) {  // 10% от радиуса сцены
                cacheValid = false;
                break;
            }
        }
    }

    if (!cacheValid) {
        SAFE_PRINT_LINE("Обновление кэша оптимизации...");

        // Оптимизируем гауссианы для текущих точек обзора
        for (auto& field : sceneData.gaussianFields) {
            if (field) {
                field->optimizeGaussians(sceneData.multiViewImages);

                // Применяем адаптивный LOD
                if (hwConfig.performanceRating < 1.0f) {
                    field->setLevelOfDetail(sceneData.sceneRadius,
                                            static_cast<int>(10000 * hwConfig.performanceRating));
                }

                metrics.gaussiansProcessed += static_cast<int>(field->getGaussianCount());
            }
        }

        // Обновляем кэш
        optimizationCache.lastViewPoints = sceneData.multiViewImages;
        optimizationCache.sceneComplexity = sceneData.complexityLevel;
        optimizationCache.cacheValid = true;
    }

    std::cout << "Обработано " << metrics.gaussiansProcessed << " гауссианов" << std::endl;
}

void OptimalRenderer3D::integrateNeRFVariants(SceneData& sceneData) {
    if (!currentHardware.supportsNeural)
        return;

    SAFE_PRINT_LINE("Интеграция NeRF вариантов...");

    // TODO: Реализация интеграции NeRF для улучшения качества view synthesis
    // В сложных сценах с большим количеством точек обзора

    if (sceneData.complexityLevel >= 4 && sceneData.multiViewImages.size() >= 6) {
        SAFE_PRINT_LINE("Применение NeRF оптимизации для сложной сцены...");
        // Гибридный подход: NeRF для областей с недостаточной плотностью гауссианов
    }
}

void OptimalRenderer3D::executeGeometryAndVisibility(const SceneData& sceneData,
                                                     const CameraParams& cameraParams) {
    SAFE_PRINT_LINE("Этап 2: Быстрая растеризация для первичной геометрии и видимости...");

    if (!hybridRenderer) {
        SAFE_ERROR("Гибридный рендерер не инициализирован!");
        return;
    }

    // Настраиваем камеру
    auto camera = std::make_shared<Camera3D>();
    camera->setPerspective(
        cameraParams.fov, cameraParams.aspectRatio, cameraParams.nearPlane, cameraParams.farPlane);
    camera->lookAt(cameraParams.position, cameraParams.target, cameraParams.up);

    hybridRenderer->setMainCamera(camera);
    hybridRenderer->beginFrame();

    // Выполняем растеризацию гауссианов
    hybridRenderer->renderScene(sceneData.gaussianFields, sceneData.meshes, sceneData.transforms);

    // Обновляем статистику
    const auto& hybridStats = hybridRenderer->getHybridRenderStats();
    metrics.gaussiansProcessed = hybridStats.gaussiansRendered;
    metrics.trianglesRendered = hybridStats.trianglesRendered;
}

void OptimalRenderer3D::executeAdvancedLighting(const SceneData& sceneData) {
    SAFE_PRINT_LINE("Этап 3: Селективная трассировка лучей для продвинутого освещения...");

    if (!globalIlluminationEnabled && !reflectionsEnabled && !shadowsEnabled) {
        SAFE_PRINT_LINE("Все эффекты трассировки лучей отключены, пропускаем этап");
        return;
    }

    // Применяем ReSTIR для глобального освещения
    if (globalIllumination && globalIlluminationEnabled) {
        // Создаем view матрицу из параметров камеры
        Matrix4 viewMatrix = Matrix4::lookAt(
            lastCameraParams.position, lastCameraParams.target, lastCameraParams.up);
        Matrix4 projMatrix = Matrix4::perspective(lastCameraParams.fov,
                                                  lastCameraParams.aspectRatio,
                                                  lastCameraParams.nearPlane,
                                                  lastCameraParams.farPlane);

        // TODO: Передача правильных буферов
        globalIllumination->computeGlobalIllumination(
            viewMatrix, projMatrix, 0, 0, 0  // TODO: Правильные буферы
        );

        const auto& giStats = globalIllumination->getStats();
        metrics.rayTracingTime += giStats.computeTime;
        metrics.raysTraced += static_cast<int>(giStats.lightSamples);
    }

    std::cout << "Трассировка лучей завершена (время: " << metrics.rayTracingTime << "мс)"
              << std::endl;
}

void OptimalRenderer3D::executeDenoisingAndRefinement() {
    SAFE_PRINT_LINE("Этап 4: AI деноизинг и уточнение...");

    if (!denoiser || !denoisingEnabled) {
        SAFE_PRINT_LINE("Деноизинг отключен или недоступен");
        return;
    }

    // TODO: Применение деноизинга к ray-traced буферам
    denoiser->denoise(0, 0);  // TODO: Правильные буферы

    const auto& denoiseStats = denoiser->getStats();
    metrics.denoisingTime = denoiseStats.denoiseTime;

    std::cout << "Деноизинг завершен (время: " << metrics.denoisingTime
              << "мс, снижение шума: " << (denoiseStats.noiseReduction * 100) << "%)" << std::endl;
}

void OptimalRenderer3D::executePostProcessingAndOutput(int outputWidth, int outputHeight) {
    SAFE_PRINT_LINE("Этап 5: Пост-обработка и нейронное масштабирование...");

    // Нейронное масштабирование если включено
    if (upscaler && upscalingEnabled && upscalingFactor > 1.0f) {
        upscaler->upscale(0, 0, renderWidth, renderHeight, outputWidth, outputHeight);

        const auto& upscaleStats = upscaler->getStats();
        metrics.upscalingTime = upscaleStats.upscaleTime;

        std::cout << "Масштабирование " << renderWidth << "x" << renderHeight << " -> "
                  << targetWidth << "x" << targetHeight
                  << " завершено (время: " << metrics.upscalingTime << "мс)" << std::endl;
    }

    // Дополнительные пост-эффекты
    metrics.postProcessingTime = 0.5f;  // Примерное время

    if (hybridRenderer) {
        hybridRenderer->endFrame();
    }
}

// Вспомогательные методы

void OptimalRenderer3D::adaptToHardware() {
    // Адаптируем разрешение рендеринга под производительность железа
    float performanceScale = std::sqrt(currentHardware.performanceRating);

    if (currentHardware.performanceRating < 0.5f) {
        // Слабое железо - рендерим в меньшем разрешении
        renderWidth = static_cast<int>(targetWidth * 0.7f);
        renderHeight = static_cast<int>(targetHeight * 0.7f);
        upscalingEnabled = true;
        upscalingFactor = 1.43f;  // ~0.7 * 1.43 = 1.0
    } else if (currentHardware.performanceRating > 2.0f) {
        // Мощное железо - можем рендерить в полном разрешении
        renderWidth = targetWidth;
        renderHeight = targetHeight;
    } else {
        // Среднее железо - адаптивное разрешение
        renderWidth = static_cast<int>(targetWidth * performanceScale);
        renderHeight = static_cast<int>(targetHeight * performanceScale);

        if (performanceScale < 0.9f) {
            upscalingEnabled = true;
            upscalingFactor = 1.0f / performanceScale;
        }
    }

    std::cout << "Адаптация под железо: разрешение рендеринга " << renderWidth << "x"
              << renderHeight << " (масштаб производительности: " << performanceScale << ")"
              << std::endl;
}

void OptimalRenderer3D::optimizeForFrameRate() {
    frameTimeHistory.push_back(metrics.totalFrameTime);
    if (frameTimeHistory.size() > 30) {  // Храним историю за последние 30 кадров
        frameTimeHistory.erase(frameTimeHistory.begin());
    }

    if (frameTimeHistory.size() < 5)
        return;  // Недостаточно данных

    // Вычисляем средний frametime
    float averageFrameTime = 0.0f;
    for (float time : frameTimeHistory) {
        averageFrameTime += time;
    }
    averageFrameTime /= frameTimeHistory.size();

    float targetFrameTime = 1000.0f / targetFPS;
    float frameTimeDiff = averageFrameTime - targetFrameTime;

    // Если отклонение больше 10%, адаптируем качество
    if (std::abs(frameTimeDiff) > targetFrameTime * 0.1f) {
        if (frameTimeDiff > 0) {
            // Слишком медленно - снижаем качество
            if (qualityLevel > 1) {
                setQualityLevel(qualityLevel - 1);
                std::cout << "Автоматическое снижение качества до уровня " << qualityLevel
                          << " (средний frametime: " << averageFrameTime << "мс)" << std::endl;
            }
        } else {
            // Достаточно быстро - можем повысить качество
            if (qualityLevel < 5) {
                setQualityLevel(qualityLevel + 1);
                std::cout << "Автоматическое повышение качества до уровня " << qualityLevel
                          << " (средний frametime: " << averageFrameTime << "мс)" << std::endl;
            }
        }
    }
}

void OptimalRenderer3D::balanceQualityAndPerformance() {
    // Анализируем, какие этапы занимают больше всего времени
    float totalTime = metrics.getTotalTime();
    if (totalTime <= 0.0f)
        return;

    float rasterizationRatio = metrics.rasterizationTime / totalTime;
    float rayTracingRatio = metrics.rayTracingTime / totalTime;
    float denoisingRatio = metrics.denoisingTime / totalTime;
    float upscalingRatio = metrics.upscalingTime / totalTime;

    std::cout << "Анализ производительности: растеризация=" << (rasterizationRatio * 100) << ", трассировка="
              << (rayTracingRatio * 100) << "%, деноизинг=" << (denoisingRatio * 100) << "%, масштабирование="
              << (upscalingRatio * 100) << "%" << std::endl;

    // Адаптируем настройки на основе узких мест
    if (hybridRenderer) {
        auto settings = hybridRenderer->getRenderSettings();

        if (rayTracingRatio > 0.4f) {
            // Трассировка лучей - узкое место
            settings.rayTracingSamples = std::max(1, settings.rayTracingSamples - 1);
            std::cout << "Снижение количества ray-tracing samples до " << settings.rayTracingSamples
                      << std::endl;
        }

        if (rasterizationRatio > 0.5f) {
            // Растеризация - узкое место
            settings.gaussianQuality *= 0.9f;
            std::cout << "Снижение качества гауссианов до " << settings.gaussianQuality
                      << std::endl;
        }

        hybridRenderer->setRenderSettings(settings);
    }
}

float OptimalRenderer3D::measureExecutionTime(const std::function<void()>& func) {
    auto startTime = std::chrono::high_resolution_clock::now();
    func();
    auto endTime = std::chrono::high_resolution_clock::now();

    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(endTime - startTime);
    return duration.count() / 1000.0f;  // Возвращаем в миллисекундах
}

void OptimalRenderer3D::updateMetrics() {
    // Вычисляем ускорение по сравнению с базовым рендерингом
    float baselineTime =
        (metrics.gaussiansProcessed * 0.001f) + (metrics.trianglesRendered * 0.0001f);
    metrics.renderingSpeedup = baselineTime > 0 ? baselineTime / metrics.totalFrameTime : 1.0f;

    // Эффективность памяти (упрощенная оценка)
    float memoryUsed = (metrics.gaussiansProcessed * 64.0f + metrics.trianglesRendered * 48.0f)
                       / (1024.0f * 1024.0f);
    metrics.memoryEfficiency =
        currentHardware.gpuMemoryMB > 0 ? (1.0f - memoryUsed / currentHardware.gpuMemoryMB) : 0.5f;

    // Качество визуализации (на основе включенных эффектов)
    float qualityScore = 0.5f;  // Базовое качество
    if (globalIlluminationEnabled)
        qualityScore += 0.2f;
    if (reflectionsEnabled)
        qualityScore += 0.15f;
    if (shadowsEnabled)
        qualityScore += 0.1f;
    if (denoisingEnabled)
        qualityScore += 0.05f;
    metrics.visualQuality = std::min(1.0f, qualityScore);

    // Временная стабильность (на основе вариации frametime)
    if (frameTimeHistory.size() > 1) {
        float variance = 0.0f;
        float mean = 0.0f;
        for (float time : frameTimeHistory)
            mean += time;
        mean /= frameTimeHistory.size();

        for (float time : frameTimeHistory) {
            variance += (time - mean) * (time - mean);
        }
        variance /= frameTimeHistory.size();

        metrics.temporalStability = 1.0f / (1.0f + variance * 0.01f);
    } else {
        metrics.temporalStability = 1.0f;
    }
}

// Профилирование и бенчмарки

void OptimalRenderer3D::runBenchmark(const SceneData& testScene, int frames) {
    std::cout << "Запуск бенчмарка (" << frames << " кадров)..." << std::endl;

    auto benchmarkStart = std::chrono::high_resolution_clock::now();

    std::vector<float> benchmarkTimes;
    benchmarkTimes.reserve(frames);

    CameraParams benchmarkCamera;
    benchmarkCamera.position = Vector3(0, 0, testScene.sceneRadius * 2);
    benchmarkCamera.target = testScene.sceneCenter;

    for (int frame = 0; frame < frames; ++frame) {
        // Поворачиваем камеру для динамической сцены
        float angle = (frame * 2.0f * M_PI) / frames;
        benchmarkCamera.position =
            Vector3(testScene.sceneCenter.x + std::cos(angle) * testScene.sceneRadius * 2,
                    testScene.sceneCenter.y + testScene.sceneRadius * 0.5f,
                    testScene.sceneCenter.z + std::sin(angle) * testScene.sceneRadius * 2);

        renderOptimal3D(testScene, benchmarkCamera, currentHardware);
        benchmarkTimes.push_back(metrics.totalFrameTime);

        if (frame % 10 == 0) {
            std::cout << "Кадр " << frame << "/" << frames << " (время: " << metrics.totalFrameTime
                      << "мс)" << std::endl;
        }
    }

    auto benchmarkEnd = std::chrono::high_resolution_clock::now();
    auto totalDuration =
        std::chrono::duration_cast<std::chrono::milliseconds>(benchmarkEnd - benchmarkStart);

    // Анализ результатов
    float totalTime = 0.0f;
    float minTime = benchmarkTimes[0];
    float maxTime = benchmarkTimes[0];

    for (float time : benchmarkTimes) {
        totalTime += time;
        minTime = std::min(minTime, time);
        maxTime = std::max(maxTime, time);
    }

    float averageTime = totalTime / benchmarkTimes.size();
    float averageFPS = 1000.0f / averageTime;

    SAFE_PRINT_LINE("\n=== РЕЗУЛЬТАТЫ БЕНЧМАРКА ===");
    SAFE_PRINT_LINE("Общее время: " + SAFE_TO_STRING(totalDuration.count()) + "мс");
    SAFE_PRINT_LINE("Средний frametime: " + SAFE_TO_STRING(averageTime) + "мс");
    SAFE_PRINT_LINE("Средний FPS: " + SAFE_TO_STRING(averageFPS));
    SAFE_PRINT_LINE("Минимальный frametime: " + SAFE_TO_STRING(minTime) + "мс ("
                    + SAFE_TO_STRING(1000.0f / minTime) + " FPS)");
    SAFE_PRINT_LINE("Максимальный frametime: " + SAFE_TO_STRING(maxTime) + "мс ("
                    + SAFE_TO_STRING(1000.0f / maxTime) + " FPS)");
    SAFE_PRINT_LINE("Ускорение рендеринга: " + SAFE_TO_STRING(metrics.renderingSpeedup) + "x");
    SAFE_PRINT_LINE("Эффективность памяти: " + SAFE_TO_STRING(metrics.memoryEfficiency * 100)
                    + "%");
    SAFE_PRINT_LINE("Визуальное качество: " + SAFE_TO_STRING(metrics.visualQuality * 100) + "%");
    SAFE_PRINT_LINE("===========================");
}

void OptimalRenderer3D::startProfiling() {
    profilingEnabled = true;
    SAFE_PRINT_LINE("Профилирование включено");
}

void OptimalRenderer3D::endProfiling() {
    profilingEnabled = false;
    SAFE_PRINT_LINE("Профилирование отключено");
}

// Сохранение/загрузка профилей

void OptimalRenderer3D::saveProfile(const std::string& filename) const {
    std::ofstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Ошибка открытия файла для сохранения профиля: " << filename << std::endl;
        return;
    }

    file << "# Профиль оптимального рендерера\n";
    file << "qualityLevel=" << qualityLevel << "\n";
    file << "targetFPS=" << targetFPS << "\n";
    file << "globalIllumination=" << (globalIlluminationEnabled ? 1 : 0) << "\n";
    file << "reflections=" << (reflectionsEnabled ? 1 : 0) << "\n";
    file << "shadows=" << (shadowsEnabled ? 1 : 0) << "\n";
    file << "denoising=" << (denoisingEnabled ? 1 : 0) << "\n";
    file << "upscaling=" << (upscalingEnabled ? 1 : 0) << "\n";
    file << "upscalingFactor=" << upscalingFactor << "\n";

    file.close();
    std::cout << "Профиль сохранен в " << filename << std::endl;
}

bool OptimalRenderer3D::loadProfile(const std::string& filename) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Ошибка открытия файла профиля: " << filename << std::endl;
        return false;
    }

    std::string line;
    while (std::getline(file, line)) {
        if (line.empty() || line[0] == '#')
            continue;

        size_t pos = line.find('=');
        if (pos == std::string::npos)
            continue;

        std::string key = line.substr(0, pos);
        std::string value = line.substr(pos + 1);

        if (key == "qualityLevel") {
            setQualityLevel(std::stoi(value));
        } else if (key == "targetFPS") {
            setPerformanceTarget(std::stof(value));
        } else if (key == "globalIllumination") {
            enableGlobalIllumination(std::stoi(value) != 0);
        } else if (key == "reflections") {
            enableReflections(std::stoi(value) != 0);
        } else if (key == "shadows") {
            enableShadows(std::stoi(value) != 0);
        } else if (key == "denoising") {
            enableDenoising(std::stoi(value) != 0);
        } else if (key == "upscaling") {
            bool enable = std::stoi(value) != 0;
            enableUpscaling(enable, upscalingFactor);
        } else if (key == "upscalingFactor") {
            upscalingFactor = std::stof(value);
            if (upscalingEnabled) {
                enableUpscaling(true, upscalingFactor);
            }
        }
    }

    file.close();
    std::cout << "Профиль загружен из " << filename << std::endl;
    return true;
}

// === PerformanceMetrics Implementation ===

void OptimalRenderer3D::PerformanceMetrics::reset() {
    sceneOptimizationTime = rasterizationTime = rayTracingTime = 0.0f;
    denoisingTime = upscalingTime = postProcessingTime = totalFrameTime = 0.0f;
    renderingSpeedup = memoryEfficiency = 1.0f;
    gaussiansProcessed = raysTraced = trianglesRendered = 0;
    visualQuality = temporalStability = 1.0f;
}

float OptimalRenderer3D::PerformanceMetrics::getTotalTime() const {
    return sceneOptimizationTime + rasterizationTime + rayTracingTime + denoisingTime
           + upscalingTime + postProcessingTime;
}

void OptimalRenderer3D::PerformanceMetrics::print() const {
    SAFE_PRINT_LINE("\n--- МЕТРИКИ ПРОИЗВОДИТЕЛЬНОСТИ ---");
    std::cout << "Оптимизация сцены: " << sceneOptimizationTime << "мс" << std::endl;
    std::cout << "Растеризация: " << rasterizationTime << "мс" << std::endl;
    std::cout << "Трассировка лучей: " << rayTracingTime << "мс" << std::endl;
    std::cout << "Деноизинг: " << denoisingTime << "мс" << std::endl;
    std::cout << "Масштабирование: " << upscalingTime << "мс" << std::endl;
    std::cout << "Пост-обработка: " << postProcessingTime << "мс" << std::endl;
    std::cout << "Общее время кадра: " << totalFrameTime << "мс" << std::endl;
    std::cout << "Обработано гауссианов: " << gaussiansProcessed << std::endl;
    std::cout << "Обработано треугольников: " << trianglesRendered << std::endl;
    SAFE_PRINT_LINE("Трассировано лучей: " + SAFE_TO_STRING(raysTraced));
    SAFE_PRINT_LINE("Ускорение рендеринга: " + SAFE_TO_STRING(renderingSpeedup) + "x");
    SAFE_PRINT_LINE("Эффективность памяти: " + SAFE_TO_STRING(memoryEfficiency * 100) + "%");
    SAFE_PRINT_LINE("Визуальное качество: " + SAFE_TO_STRING(visualQuality * 100) + "%");
    SAFE_PRINT_LINE("Временная стабильность: " + SAFE_TO_STRING(temporalStability * 100) + "%");
    SAFE_PRINT_LINE("--------------------------------");
}

// === OptimalRendererFactory Implementation ===

std::unique_ptr<OptimalRenderer3D> OptimalRendererFactory::createOptimalRenderer(
    const CreationParams& params) {
    std::cout << "Создание оптимального рендерера с параметрами:\n  Разрешение: " << params.width
              << "x" << params.height << "\n  Качество: " << params.qualityLevel
              << "\n  Целевой FPS: " << params.targetFPS << std::endl;

    // Определяем конфигурацию железа
    auto hwConfig = detectHardware();

    // Создаем рендерер
    auto renderer = std::make_unique<OptimalRenderer3D>();

    if (!renderer->initialize(params.width, params.height, hwConfig)) {
        SAFE_ERROR("Ошибка инициализации оптимального рендерера!");
        return nullptr;
    }

    // Применяем настройки
    renderer->setQualityLevel(params.qualityLevel);
    renderer->setPerformanceTarget(params.targetFPS);
    renderer->enableAdaptiveQuality(params.autoOptimize);

    if (params.enableAllEffects) {
        renderer->enableGlobalIllumination(hwConfig.supportsRayTracing);
        renderer->enableReflections(hwConfig.supportsRayTracing);
        renderer->enableShadows(true);
        renderer->enableDenoising(hwConfig.supportsNeural);
        renderer->enableUpscaling(hwConfig.supportsNeural && hwConfig.performanceRating < 1.0f,
                                  2.0f);
    }

    SAFE_PRINT_LINE("Оптимальный рендерер успешно создан");
    return renderer;
}

std::unique_ptr<OptimalRenderer3D> OptimalRendererFactory::createOptimalRenderer() {
    CreationParams defaultParams;
    return createOptimalRenderer(defaultParams);
}

OptimalRenderer3D::HardwareConfig OptimalRendererFactory::detectHardware() {
    OptimalRenderer3D::HardwareConfig config;

    SAFE_PRINT_LINE("Определение конфигурации аппаратного обеспечения...");

    // TODO: Реальное определение характеристик GPU
    // Сейчас используем примерные значения

    config.performanceRating = 1.0f;  // Средняя производительность
    config.gpuMemoryMB = 4096;        // 4GB видеопамяти
    config.computeUnits = 32;         // 32 compute units

    config.autoDetect();

    return config;
}

OptimalRendererFactory::CreationParams OptimalRendererFactory::recommendSettings(
    const OptimalRenderer3D::HardwareConfig& hw) {
    CreationParams params;

    if (hw.performanceRating < 0.5f) {
        // Слабое железо
        params.qualityLevel = 1;
        params.targetFPS = 30.0f;
        params.width = 1280;
        params.height = 720;
    } else if (hw.performanceRating < 1.0f) {
        // Среднее железо
        params.qualityLevel = 2;
        params.targetFPS = 60.0f;
        params.width = 1920;
        params.height = 1080;
    } else if (hw.performanceRating < 2.0f) {
        // Хорошее железо
        params.qualityLevel = 3;
        params.targetFPS = 60.0f;
        params.width = 1920;
        params.height = 1080;
    } else {
        // Мощное железо
        params.qualityLevel = 4;
        params.targetFPS = 120.0f;
        params.width = 2560;
        params.height = 1440;
    }

    params.enableAllEffects = (hw.performanceRating > 0.7f);
    params.autoOptimize = true;

    std::cout << "Рекомендованные настройки: качество=" << params.qualityLevel
              << ", разрешение=" << params.width << "x" << params.height
              << ", FPS=" << params.targetFPS << std::endl;

    return params;
}

void OptimalRendererFactory::benchmarkSystem(OptimalRenderer3D::HardwareConfig& hw,
                                             CreationParams& params) {
    SAFE_PRINT_LINE("Бенчмаркинг системы для определения оптимальных настроек...");

    // TODO: Реализация быстрого бенчмарка для определения производительности
    // Создание простой тестовой сцены и измерение производительности

    // Пока используем значения по умолчанию
    params = recommendSettings(hw);

    SAFE_PRINT_LINE("Бенчмарк завершен");
}

// === AdaptiveLOD Implementation ===

AdaptiveLOD::AdaptiveLOD() : currentLODAdjustment(1.0f) {
    settings = LODSettings{};
    stats.reset();
    frameTimeHistory.reserve(60);
}

void AdaptiveLOD::setSettings(const LODSettings& newSettings) {
    settings = newSettings;
    SAFE_PRINT_LINE("Обновлены настройки адаптивного LOD");
}

void AdaptiveLOD::computeLOD(GaussianField3D& field,
                             const Vector3& cameraPos,
                             const Vector3& cameraVelocity,
                             float targetFrameTime) {
    std::cout << "Вычисление адаптивного LOD для " << field.getGaussianCount() << " гауссианов..."
              << std::endl;

    stats.reset();
    stats.totalGaussians = static_cast<int>(field.getGaussianCount());

    // Применяем различные типы LOD
    applySpatialLOD(field, cameraPos);

    if (settings.enableTemporalStability) {
        applyTemporalStabilization(field);
    }

    if (settings.enableMotionBasedLOD) {
        applyMotionBasedLOD(field, cameraVelocity);
    }

    // Адаптируем к производительности
    adaptToPerformance(0.0f, targetFrameTime);  // TODO: передать реальный frametime

    std::cout << "LOD распределение: L0=" << stats.gaussiansPerLevel[0] << ", L1=" << stats.gaussiansPerLevel[1]
              << ", L2=" << stats.gaussiansPerLevel[2] << ", L3=" << stats.gaussiansPerLevel[3] << std::endl;
}

void AdaptiveLOD::adaptToPerformance(float currentFrameTime, float targetFrameTime) {
    frameTimeHistory.push_back(currentFrameTime);
    if (frameTimeHistory.size() > 10) {
        frameTimeHistory.erase(frameTimeHistory.begin());
    }

    if (frameTimeHistory.size() < 3)
        return;

    float avgFrameTime = 0.0f;
    for (float time : frameTimeHistory) {
        avgFrameTime += time;
    }
    avgFrameTime /= frameTimeHistory.size();

    if (avgFrameTime > targetFrameTime * 1.1f) {
        // Слишком медленно - снижаем LOD
        currentLODAdjustment = std::max(0.1f, currentLODAdjustment * 0.9f);
    } else if (avgFrameTime < targetFrameTime * 0.9f) {
        // Достаточно быстро - можем повысить LOD
        currentLODAdjustment = std::min(1.0f, currentLODAdjustment * 1.05f);
    }

    stats.lodAdjustmentFactor = currentLODAdjustment;
}

int AdaptiveLOD::computeLODLevel(float distance, float screenSize, float velocity) const {
    // Базовый LOD на основе расстояния
    int baseLOD = 0;
    if (distance > settings.lowDetailDistance)
        baseLOD = 3;
    else if (distance > settings.mediumDetailDistance)
        baseLOD = 2;
    else if (distance > settings.highDetailDistance)
        baseLOD = 1;

    // Корректировка на основе размера на экране
    if (screenSize < 0.01f)
        baseLOD = std::min(3, baseLOD + 1);
    else if (screenSize > 0.1f)
        baseLOD = std::max(0, baseLOD - 1);

    // Корректировка на основе скорости движения
    if (velocity > 5.0f)
        baseLOD = std::min(3, baseLOD + 1);

    return baseLOD;
}

void AdaptiveLOD::applySpatialLOD(GaussianField3D& field, const Vector3& cameraPos) {
    auto& gaussians = const_cast<std::vector<Gaussian3D>&>(field.getGaussians());

    float totalDistance = 0.0f;

    for (auto& gaussian : gaussians) {
        float distance = (gaussian.position - cameraPos).magnitude();
        totalDistance += distance;

        // Примерный размер на экране (упрощенная оценка)
        float screenSize = gaussian.scale.magnitude() / (distance + 1.0f);

        int lodLevel = computeLODLevel(distance, screenSize, 0.0f);

        // Применяем LOD
        if (lodLevel > 0) {
            float qualityMultiplier = settings.qualityMultipliers[lodLevel] * currentLODAdjustment;
            gaussian.scale = gaussian.scale * qualityMultiplier;
            gaussian.opacity *= qualityMultiplier;
        }

        stats.gaussiansPerLevel[lodLevel]++;
    }

    stats.averageDistance = totalDistance / gaussians.size();
}

void AdaptiveLOD::applyTemporalStabilization(GaussianField3D& field) {
    // TODO: Реализация стабилизации LOD во времени
    // Предотвращение резких изменений LOD между кадрами
    SAFE_PRINT_LINE("Применение временной стабилизации LOD...");
}

void AdaptiveLOD::applyMotionBasedLOD(GaussianField3D& field, const Vector3& velocity) {
    if (velocity.magnitude() < 1.0f)
        return;  // Статическая камера

    std::cout << "Применение LOD на основе движения (скорость: " << velocity.magnitude() << ")..."
              << std::endl;

    // При быстром движении снижаем LOD для поддержания производительности
    float motionFactor = std::min(1.0f, velocity.magnitude() / 10.0f);
    currentLODAdjustment *= (1.0f - motionFactor * 0.3f);
}

}  // namespace Rendering
}  // namespace HyperEngine
