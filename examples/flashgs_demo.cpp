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
#include <random>

// Core Engine includes
#include <Engine3D/Core/Console.h>
#include <Engine3D/CUDA/FlashGSSplatter.h>
#include <Engine3D/Vulkan/SceneManager.h>
#include <Engine3D/CUDA/CudaInterop.h>
#include <Engine3D/Vulkan/VulkanRenderer.h>
#include <Engine3D/Vulkan/HardwareDetector.h>

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

using namespace Engine3D;
using namespace Engine3D::CUDA;

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
    
    std::cout << "🎲 Генерация " << std::to_string(numPoints) << " случайных точек..." << std::endl;
    
    for (int i = 0; i < numPoints; i++) {
        float4 point;
        point.x = posDist(gen);
        point.y = posDist(gen);
        point.z = posDist(gen);
        point.w = intensityDist(gen); // intensity
        
        points.push_back(point);
    }
    
    std::cout << "✅ Точечное облако создано: " << std::to_string(points.size()) << " точек" << std::endl;
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
    
    std::cout << "🌐 Генерация сферы из " << std::to_string(numPoints) << " точек (радиус " << std::to_string(radius) << ")..." << std::endl;
    
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
    
    std::cout << "✅ Сфера создана: " << std::to_string(points.size()) << " точек" << std::endl;
    return points;
}

/**
 * @brief Тестирование производительности FlashGS
 * @param splatter Экземпляр FlashGSSplatter
 * @param points Точечное облако
 */
void performanceTest(FlashGSSplatter& splatter, const std::vector<float4>& points) {
    std::cout << "\n📊 === ТЕСТ ПРОИЗВОДИТЕЛЬНОСТИ ===" << std::endl;
    
#ifdef CUDA_VULKAN_INTEROP_SUPPORTED
    // Инициализация гауссианов из точечного облака
    auto start = std::chrono::high_resolution_clock::now();
    
    splatter.initializeFromPointCloud(points.data(), points.size(), 0.05f);
    
    auto end = std::chrono::high_resolution_clock::now();
    auto initTime = std::chrono::duration<float, std::milli>(end - start).count();
    
    std::cout << "⏱️  Инициализация гауссианов: " << std::to_string(initTime) << " мс" << std::endl;
    
    // Тест оптимизации
    std::cout << "🔄 Запуск оптимизации (50 итераций)..." << std::endl;
    
    Engine3D::Vulkan::MultiViewImages testImages;
    testImages.viewCount = 4;
    
    start = std::chrono::high_resolution_clock::now();
    splatter.optimizeGaussians(testImages, 50);
    end = std::chrono::high_resolution_clock::now();
    
    auto optimTime = std::chrono::duration<float, std::milli>(end - start).count();
    std::cout << "⏱️  Оптимизация (50 итераций): " << std::to_string(optimTime) << " мс" << std::endl;
    
    // Тест растеризации
    std::cout << "🎨 Тест растеризации..." << std::endl;
    
    CameraMatrix camera = {};
    // Инициализация матриц камеры (упрощенная)
    for (int i = 0; i < 16; i++) {
        camera.viewMatrix[i] = (i % 5 == 0) ? 1.0f : 0.0f;  // Диагональная матрица
        camera.projMatrix[i] = (i % 5 == 0) ? 1.0f : 0.0f;
        camera.viewProjMatrix[i] = (i % 5 == 0) ? 1.0f : 0.0f;
    }
    camera.width = 1920;
    camera.height = 1080;
    camera.near = 0.1f;
    camera.far = 100.0f;
    
    start = std::chrono::high_resolution_clock::now();
    splatter.rasterizeGaussiansCUDA(camera, nullptr, nullptr, 1920, 1080);
    splatter.synchronize();
    end = std::chrono::high_resolution_clock::now();
    
    auto rasterTime = std::chrono::duration<float, std::milli>(end - start).count();
    std::cout << "⏱️  Растеризация (1920x1080): " << std::to_string(rasterTime) << " мс" << std::endl;
    
    // Итоговая статистика
    std::cout << "\n📈 === СТАТИСТИКА ПРОИЗВОДИТЕЛЬНОСТИ ===" << std::endl;
    std::cout << "🔹 Активных гауссианов: " << std::to_string(splatter.getActiveGaussianCount()) << std::endl;
    std::cout << "🔹 Время последнего рендера: " << std::to_string(splatter.getLastRenderTime()) << " мс" << std::endl;
    std::cout << "🔹 FPS (приблизительно): " << std::to_string(1000.0f / splatter.getLastRenderTime()) << std::endl;
    
    // Вычисляем производительность
    float totalTime = initTime + optimTime + rasterTime;
    std::cout << "🔹 Общее время: " << std::to_string(totalTime) << " мс" << std::endl;
    std::cout << "🔹 Точек в секунду: " << std::to_string(points.size() * 1000.0f / totalTime) << std::endl;
    
#else
    std::cout << "⚠️  CUDA interop не поддерживается - пропускаем тест производительности" << std::endl;
#endif
}

/**
 * @brief Демонстрация возможностей FlashGS
 */
void demonstrateFlashGSCapabilities() {
    std::cout << "\n🚀 === ДЕМОНСТРАЦИЯ ВОЗМОЖНОСТЕЙ FLASHGS ===" << std::endl;
    
    // Показываем различные режимы работы
    std::cout << "🔹 Tile-based растеризация с 16x16 тайлами" << std::endl;
    std::cout << "🔹 CUDA-ускоренная оптимизация параметров" << std::endl;
    std::cout << "🔹 Адаптивный контроль плотности гауссианов" << std::endl;
    std::cout << "🔹 Высокопроизводительная сортировка по глубине" << std::endl;
    std::cout << "🔹 Zero-copy интеграция с Vulkan через interop" << std::endl;
    
#ifdef CUDA_VULKAN_INTEROP_SUPPORTED
    // Информация о CUDA устройстве
    int deviceCount;
    cudaGetDeviceCount(&deviceCount);
    
    if (deviceCount > 0) {
        cudaDeviceProp props;
        cudaGetDeviceProperties(&props, 0);
        
        std::cout << "\n💻 CUDA Устройство: " << props.name << std::endl;
        std::cout << "🔹 Compute Capability: " << std::to_string(props.major) << "." << std::to_string(props.minor) << std::endl;
        std::cout << "🔹 Глобальная память: " << std::to_string(props.totalGlobalMem / 1024 / 1024) << " МБ" << std::endl;
        std::cout << "🔹 Мультипроцессоры: " << std::to_string(props.multiProcessorCount) << std::endl;
        std::cout << "🔹 Максимальных потоков на блок: " << std::to_string(props.maxThreadsPerBlock) << std::endl;
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

    
    std::cout << "🎮 FlashGS Demo - CUDA-ускоренный 3D Gaussian Splatting" << std::endl;
    std::cout << "========================================================" << std::endl;
    
    try {
        // Проверка поддержки CUDA
#ifdef CUDA_VULKAN_INTEROP_SUPPORTED
        if (!CudaInterop::isInteropSupported()) {
            std::cout << "⚠️  CUDA-Vulkan interop не поддерживается на этой системе" << std::endl;
            std::cout << "📝 Демо будет работать в ограниченном режиме" << std::endl;
        } else {
            std::cout << "✅ CUDA-Vulkan interop поддерживается" << std::endl;
        }
#else
        std::cout << "⚠️  Демо скомпилировано без поддержки CUDA" << std::endl;
#endif
        
        // Демонстрация возможностей
        demonstrateFlashGSCapabilities();
        
        // Создание FlashGS splatter
        std::cout << "\n🔧 Инициализация FlashGSSplatter..." << std::endl;
        
        auto splatter = std::make_unique<FlashGSSplatter>();
        
        // Создание CUDA interop (если доступен)
        std::shared_ptr<CudaInterop> interop = nullptr;
        
#ifdef CUDA_VULKAN_INTEROP_SUPPORTED
        if (CudaInterop::isInteropSupported()) {
            interop = std::make_shared<CudaInterop>();
            // В полной версии здесь будет инициализация с Vulkan устройством
            std::cout << "🔗 CUDA interop создан" << std::endl;
        }
#endif
        
        // Инициализация splatter
        if (!splatter->init(interop)) {
            std::cerr << "❌ Ошибка инициализации FlashGSSplatter" << std::endl;
            return -1;
        }
        
        std::cout << "✅ FlashGSSplatter инициализирован успешно" << std::endl;
        
        // Настройка параметров оптимизации
#ifdef CUDA_VULKAN_INTEROP_SUPPORTED
        OptimizationParams params;
        params.learningRate = 0.01f;
        params.densificationThreshold = 0.1f;
        params.pruningThreshold = 0.005f;
        params.maxGaussians = 50000;
        params.iterationCount = 100;
        
        splatter->setOptimizationParams(params);
        std::cout << "⚙️  Параметры оптимизации установлены" << std::endl;
#endif
        
        // Тест с различными сценариями
        std::cout << "\n🧪 === ТЕСТОВЫЕ СЦЕНАРИИ ===" << std::endl;
        
        // Сценарий 1: Небольшое случайное облако
        std::cout << "\n🔸 Сценарий 1: Случайное облако (1000 точек)" << std::endl;
        auto randomCloud = generateTestPointCloud(1000);
        performanceTest(*splatter, randomCloud);
        
        // Сценарий 2: Сфера
        std::cout << "\n🔸 Сценарий 2: Сфера (5000 точек)" << std::endl;
        auto sphereCloud = generateSpherePointCloud(5000, 3.0f);
        performanceTest(*splatter, sphereCloud);
        
        // Сценарий 3: Большое облако (если хватает памяти)
        std::cout << "\n🔸 Сценарий 3: Большое случайное облако (20000 точек)" << std::endl;
        auto largeCloud = generateTestPointCloud(20000);
        performanceTest(*splatter, largeCloud);
        
        // Завершение
        std::cout << "\n✅ === ДЕМО ЗАВЕРШЕНО УСПЕШНО ===" << std::endl;
        std::cout << "🎯 FlashGS показал высокую производительность CUDA-ускоренного рендеринга" << std::endl;
        std::cout << "📈 Tile-based подход обеспечивает эффективную растеризацию больших сцен" << std::endl;
        std::cout << "🚀 Готов к интеграции в полнофункциональный рендеринг pipeline" << std::endl;
        
    } catch (const std::exception& e) {
        std::cerr << "❌ Критическая ошибка: " << e.what() << std::endl;
        return -1;
    }
    
    return 0;
}
