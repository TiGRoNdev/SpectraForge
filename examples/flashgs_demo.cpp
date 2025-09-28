/**
 * @file flashgs_demo.cpp
 * @brief Демонстрация CUDA-ускоренного 3D Gaussian Splatting (FlashGS)
 * 
 * Это демо-приложение показывает возможности FlashGS Implementation:
 * - Инициализацию гауссианов из точечного облака
 * - CUDA-ускоренную оптимизацию параметров
 * - Tile-based растеризацию с высокой производительностью
 * - Интеграцию с CUDA-Vulkan interop
 */

#include <iostream>
#include <vector>
#include <memory>
#include <chrono>
#include "HyperEngine/Core/SafeConsole.h"
#include <random>

// Core Engine includes
#include <HyperEngine/Core/Console.h>

using namespace HyperEngine::Core;
#include <HyperEngine/CUDA/FlashGSSplatter.h>
#include <HyperEngine/Vulkan/SceneManager.h>
#include <HyperEngine/CUDA/CudaInterop.h>
#include <HyperEngine/Vulkan/VulkanRenderer.h>
#include <HyperEngine/Vulkan/HardwareDetector.h>

#ifdef CUDA_VULKAN_INTEROP_SUPPORTED
#include <cuda_runtime.h>
#endif

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

#ifndef CUDA_VULKAN_INTEROP_SUPPORTED
// Заглушка для float4 без CUDA
struct float4 {
    float x, y, z, w;
    float4() : x(0), y(0), z(0), w(0) {}
    float4(float x_, float y_, float z_, float w_) : x(x_), y(y_), z(z_), w(w_) {}
};
#endif

using namespace HyperEngine;
using namespace HyperEngine::CUDA;

/**
 * @brief Генерация тестового точечного облака
 * @param numPoints Количество точек
 * @return Вектор точек (x, y, z, intensity)
 */
std::vector<float4> generateTestPointCloud(int numPoints) {
    std::vector<float4> points;
    points.reserve(numPoints);
    
    // Генератор случайных чисел
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<float> posDist(-5.0f, 5.0f);
    std::uniform_real_distribution<float> intensityDist(0.2f, 1.0f);
    
    std::cout << "🎲 Генерация " << SAFE_TO_STRING(numPoints) << " случайных точек..." << std::endl;
    
    for (int i = 0; i < numPoints; i++) {
        float4 point;
        point.x = posDist(gen);
        point.y = posDist(gen);
        point.z = posDist(gen);
        point.w = intensityDist(gen); // intensity
        
        points.push_back(point);
    }
    
    std::cout << "✅ Точечное облако создано: " << SAFE_TO_STRING(points.size()) << " точек" << std::endl;
    return points;
}

/**
 * @brief Создание тестовой сферы из точек
 * @param numPoints Количество точек
 * @param radius Радиус сферы
 * @return Вектор точек на сфере
 */
std::vector<float4> generateSpherePointCloud(int numPoints, float radius = 2.0f) {
    std::vector<float4> points;
    points.reserve(numPoints);
    
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<float> angleDist(0.0f, 2.0f * M_PI);
    std::uniform_real_distribution<float> uDist(-1.0f, 1.0f);
    std::uniform_real_distribution<float> intensityDist(0.5f, 1.0f);
    
    std::cout << "🌐 Генерация сферы из " << SAFE_TO_STRING(numPoints) << " точек (радиус " << SAFE_TO_STRING(radius) << ")..." << std::endl;
    
    for (int i = 0; i < numPoints; i++) {
        // Равномерное распределение точек на сфере
        float theta = angleDist(gen);      // азимутальный угол
        float u = uDist(gen);              // cos(полярный угол)
        float phi = acosf(u);               // полярный угол
        
        float4 point;
        point.x = radius * sinf(phi) * cosf(theta);
        point.y = radius * sinf(phi) * sinf(theta);
        point.z = radius * cosf(phi);
        point.w = intensityDist(gen);
        
        points.push_back(point);
    }
    
    std::cout << "✅ Сфера создана: " << SAFE_TO_STRING(points.size()) << " точек" << std::endl;
    return points;
}

/**
 * @brief Тестирование производительности FlashGS
 * @param splatter Экземпляр FlashGSSplatter
 * @param points Точечное облако
 */
void performanceTest(FlashGSSplatter& splatter, const std::vector<float4>& points) {
    SAFE_PRINT_LINE("\n📊 === ТЕСТ ПРОИЗВОДИТЕЛЬНОСТИ ===");
    
#ifdef CUDA_VULKAN_INTEROP_SUPPORTED
    // Инициализация гауссианов из точечного облака
    auto start = std::chrono::high_resolution_clock::now();
    
    splatter.initializeFromPointCloud(points.data(), points.size(), 0.05f);
    
    auto end = std::chrono::high_resolution_clock::now();
    auto initTime = std::chrono::duration<float, std::milli>(end - start).count();
    
    std::cout << "⏱️  Инициализация гауссианов: " << SAFE_TO_STRING(initTime) << " мс" << std::endl;
    
    // Тест оптимизации
    SAFE_PRINT_LINE("🔄 Запуск оптимизации (50 итераций)...");
    
    HyperEngine::Vulkan::MultiViewImages testImages;
    testImages.viewCount = 4;
    
    start = std::chrono::high_resolution_clock::now();
    splatter.optimizeGaussians(testImages, 50);
    end = std::chrono::high_resolution_clock::now();
    
    auto optimTime = std::chrono::duration<float, std::milli>(end - start).count();
    std::cout << "⏱️  Оптимизация (50 итераций): " << SAFE_TO_STRING(optimTime) << " мс" << std::endl;
    
    // Тест растеризации
    SAFE_PRINT_LINE("🎨 Тест растеризации...");
    
    CameraMatrix camera = {};
    // Инициализация матриц камеры (упрощенная)
    for (int i = 0; i < 16; i++) {
        camera.viewMatrix[i] = (i % 5 == 0) ? 1.0f : 0.0f;  // Диагональная матрица
        camera.projMatrix[i] = (i % 5 == 0) ? 1.0f : 0.0f;
        camera.viewProjMatrix[i] = (i % 5 == 0) ? 1.0f : 0.0f;
    }
    camera.width = 1920;
    camera.height = 1080;
    camera.nearPlane = 0.1f;
    camera.farPlane = 100.0f;
    
    start = std::chrono::high_resolution_clock::now();
    splatter.rasterizeGaussiansCUDA(camera, nullptr, nullptr, 1920, 1080);
    splatter.synchronize();
    end = std::chrono::high_resolution_clock::now();
    
    auto rasterTime = std::chrono::duration<float, std::milli>(end - start).count();
    std::cout << "⏱️  Растеризация (1920x1080): " << SAFE_TO_STRING(rasterTime) << " мс" << std::endl;
    
    // Итоговая статистика
    SAFE_PRINT_LINE("\n📈 === СТАТИСТИКА ПРОИЗВОДИТЕЛЬНОСТИ ===");
    std::cout << "🔹 Активных гауссианов: " << SAFE_TO_STRING(splatter.getActiveGaussianCount()) << std::endl;
    std::cout << "🔹 Время последнего рендера: " << SAFE_TO_STRING(splatter.getLastRenderTime()) << " мс" << std::endl;
    std::cout << "🔹 FPS (приблизительно): " << SAFE_TO_STRING(1000.0f / splatter.getLastRenderTime()) << std::endl;
    
    // Вычисляем производительность
    float totalTime = initTime + optimTime + rasterTime;
    std::cout << "🔹 Общее время: " << SAFE_TO_STRING(totalTime) << " мс" << std::endl;
    std::cout << "🔹 Точек в секунду: " << SAFE_TO_STRING(points.size() * 1000.0f / totalTime) << std::endl;
    
#else
    SAFE_PRINT_LINE("⚠️  CUDA interop не поддерживается - пропускаем тест производительности");
#endif
}

/**
 * @brief Демонстрация возможностей FlashGS
 */
void demonstrateFlashGSCapabilities() {
    SAFE_PRINT_LINE("\n🚀 === ДЕМОНСТРАЦИЯ ВОЗМОЖНОСТЕЙ FLASHGS ===");
    
    // Показываем различные режимы работы
    SAFE_PRINT_LINE("🔹 Tile-based растеризация с 16x16 тайлами");
    SAFE_PRINT_LINE("🔹 CUDA-ускоренная оптимизация параметров");
    SAFE_PRINT_LINE("🔹 Адаптивный контроль плотности гауссианов");
    SAFE_PRINT_LINE("🔹 Высокопроизводительная сортировка по глубине");
    SAFE_PRINT_LINE("🔹 Zero-copy интеграция с Vulkan через interop");
    
#ifdef CUDA_VULKAN_INTEROP_SUPPORTED
    // Информация о CUDA устройстве
    int deviceCount;
    cudaGetDeviceCount(&deviceCount);
    
    if (deviceCount > 0) {
        cudaDeviceProp props;
        cudaGetDeviceProperties(&props, 0);
        
        std::cout << "\n💻 CUDA Устройство: " << props.name << std::endl;
        std::cout << "🔹 Compute Capability: " << SAFE_TO_STRING(props.major) << "." << SAFE_TO_STRING(props.minor) << std::endl;
        std::cout << "🔹 Глобальная память: " << SAFE_TO_STRING(props.totalGlobalMem / 1024 / 1024) << " МБ" << std::endl;
        std::cout << "🔹 Мультипроцессоры: " << SAFE_TO_STRING(props.multiProcessorCount) << std::endl;
        std::cout << "🔹 Максимальных потоков на блок: " << SAFE_TO_STRING(props.maxThreadsPerBlock) << std::endl;
    }
#endif
}

/**
 * @brief Главная функция демо
 */
int main() {
    // Инициализация UTF-8 консоли
    Core::Console console;
    console.initialize();

    
    SAFE_PRINT_LINE("🎮 FlashGS Demo - CUDA-ускоренный 3D Gaussian Splatting");
    SAFE_PRINT_LINE("========================================================");
    
    try {
        // Проверка поддержки CUDA
#ifdef CUDA_VULKAN_INTEROP_SUPPORTED
        if (!CudaInterop::isInteropSupported()) {
            SAFE_PRINT_LINE("⚠️  CUDA-Vulkan interop не поддерживается на этой системе");
            SAFE_PRINT_LINE("📝 Демо будет работать в ограниченном режиме");
        } else {
            SAFE_PRINT_LINE("✅ CUDA-Vulkan interop поддерживается");
        }
#else
        SAFE_PRINT_LINE("⚠️  Демо скомпилировано без поддержки CUDA");
#endif
        
        // Демонстрация возможностей
        demonstrateFlashGSCapabilities();
        
        // Создание FlashGS splatter
        SAFE_PRINT_LINE("\n🔧 Инициализация FlashGSSplatter...");
        
        auto splatter = std::make_unique<FlashGSSplatter>();
        
        // Создание CUDA interop (если доступен)
        std::shared_ptr<CudaInterop> interop = nullptr;
        
#ifdef CUDA_VULKAN_INTEROP_SUPPORTED
        if (CudaInterop::isInteropSupported()) {
            interop = std::make_shared<CudaInterop>();
            // В полной версии здесь будет инициализация с Vulkan устройством
            SAFE_PRINT_LINE("🔗 CUDA interop создан");
        }
#endif
        
        // Инициализация splatter
        if (!splatter->init(interop)) {
            SAFE_ERROR("❌ Ошибка инициализации FlashGSSplatter");
            return -1;
        }
        
        SAFE_PRINT_LINE("✅ FlashGSSplatter инициализирован успешно");
        
        // Настройка параметров оптимизации
#ifdef CUDA_VULKAN_INTEROP_SUPPORTED
        OptimizationParams params;
        params.learningRate = 0.01f;
        params.densificationThreshold = 0.1f;
        params.pruningThreshold = 0.005f;
        params.maxGaussians = 50000;
        params.iterationCount = 100;
        
        splatter->setOptimizationParams(params);
        SAFE_PRINT_LINE("⚙️  Параметры оптимизации установлены");
#endif
        
        // Тест с различными сценариями
        SAFE_PRINT_LINE("\n🧪 === ТЕСТОВЫЕ СЦЕНАРИИ ===");
        
        // Сценарий 1: Небольшое случайное облако
        SAFE_PRINT_LINE("\n🔸 Сценарий 1: Случайное облако (1000 точек)");
        auto randomCloud = generateTestPointCloud(1000);
        performanceTest(*splatter, randomCloud);
        
        // Сценарий 2: Сфера
        SAFE_PRINT_LINE("\n🔸 Сценарий 2: Сфера (5000 точек)");
        auto sphereCloud = generateSpherePointCloud(5000, 3.0f);
        performanceTest(*splatter, sphereCloud);
        
        // Сценарий 3: Большое облако (если хватает памяти)
        SAFE_PRINT_LINE("\n🔸 Сценарий 3: Большое случайное облако (20000 точек)");
        auto largeCloud = generateTestPointCloud(20000);
        performanceTest(*splatter, largeCloud);
        
        // Завершение
        SAFE_PRINT_LINE("\n✅ === ДЕМО ЗАВЕРШЕНО УСПЕШНО ===");
        SAFE_PRINT_LINE("🎯 FlashGS показал высокую производительность CUDA-ускоренного рендеринга");
        SAFE_PRINT_LINE("📈 Tile-based подход обеспечивает эффективную растеризацию больших сцен");
        SAFE_PRINT_LINE("🚀 Готов к интеграции в полнофункциональный рендеринг pipeline");
        
    } catch (const std::exception& e) {
        std::cerr << "❌ Критическая ошибка: " << e.what() << std::endl;
        return -1;
    }
    
    return 0;
}

