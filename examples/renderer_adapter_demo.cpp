/**
 * @file renderer_adapter_demo.cpp
 * @brief Демонстрация работы с адаптерами рендеринга
 * 
 * Показывает как использовать RendererAdapter для переключения между
 * OpenGL и Vulkan backend'ами без изменения клиентского кода.
 */

#include "Engine3D/Rendering/RendererAdapter.h"
#include "Engine3D/Rendering/Mesh3D.h"
#include "Engine3D/Rendering/Shader3D.h"
#include "Engine3D/Rendering/Camera3D.h"
#include "Engine3D/Math/Matrix4.h"
#include "Engine3D/Math/Vector3.h"
#include "Engine3D/Core/Console.h"

#include <iostream>
#include <memory>
#include <chrono>
#include <thread>

using namespace Engine3D::Rendering;
using namespace Engine3D::Math;
using namespace Engine3D::Core;

/**
 * @brief Демонстрация базового рендеринга с адаптером
 */
void demonstrateBasicRendering(RendererAdapter& adapter) {
    std::cout << "\n=== Демонстрация базового рендеринга ===" << std::endl;
    
    // Инициализация рендерера
    if (!adapter.initialize(800, 600)) {
        std::cerr << "Ошибка инициализации рендерера!" << std::endl;
        return;
    }
    
    std::cout << "Рендерер инициализирован: " << adapter.getBackendName() << std::endl;
    
    // Настройка камеры
    auto camera = std::make_shared<Camera3D>();
    camera->setPerspective(60.0f, 800.0f / 600.0f, 0.1f, 1000.0f);
    camera->setPosition(Vector3(0, 0, 5));
    camera->lookAt(Vector3(0, 0, 5), Vector3(0, 0, 0), Vector3(0, 1, 0));
    adapter.setMainCamera(camera);
    
    // Создание меша (куб)
    auto mesh = Mesh3D::createCube(2.0f);
    if (!mesh) {
        std::cerr << "Ошибка создания меша!" << std::endl;
        return;
    }
    
    // Создание шейдера
    auto shader = Shader3D::createBasicShader();
    if (!shader) {
        std::cerr << "Ошибка создания шейдера!" << std::endl;
        return;
    }
    
    // Настройка рендеринга
    adapter.setClearColor(0.2f, 0.3f, 0.4f, 1.0f);
    adapter.enableDepthTest(true);
    adapter.enableBackfaceCulling(true);
    
    // Симуляция рендеринга нескольких кадров
    std::cout << "Рендеринг 5 кадров..." << std::endl;
    
    for (int frame = 0; frame < 5; ++frame) {
        adapter.beginFrame();
        adapter.clear();
        
        // Поворот куба
        float angle = frame * 0.5f;
        Matrix4 transform = Matrix4::rotationY(angle);
        
        // Рендеринг меша
        adapter.renderMesh(*mesh, transform, *shader);
        
        adapter.endFrame();
        
        std::cout << "Кадр " << std::to_string(frame + 1) << " отрендерен" << std::endl;
        
        // Небольшая пауза для имитации реального времени
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
    
    std::cout << "Рендеринг завершен" << std::endl;
}

/**
 * @brief Демонстрация переключения между backend'ами
 */
void demonstrateBackendSwitching() {
    std::cout << "\n=== Демонстрация переключения backend'ов ===" << std::endl;
    
    RendererAdapter& adapter = RendererAdapter::getInstance();
    
    // Показываем доступные backend'ы
    adapter.printBackendInfo();
    
    auto availableBackends = adapter.getAvailableBackends();
    
    // Тестируем каждый доступный backend
    for (auto backend : availableBackends) {
        if (backend == RenderBackend::AUTO) continue;  // Пропускаем AUTO
        
        const char* backendName = "";
        switch (backend) {
            case RenderBackend::OPENGL: backendName = "OpenGL"; break;
            case RenderBackend::VULKAN: backendName = "Vulkan"; break;
            default: continue;
        }
        
        std::cout << "\n--- Тестирование " << backendName << " ---" << std::endl;
        
        if (adapter.setBackend(backend)) {
            std::cout << "Успешно переключен на " << backendName << std::endl;
            
            // Быстрый тест рендеринга
            if (adapter.initialize(640, 480)) {
                std::cout << "Инициализация успешна" << std::endl;
                
                // Тест базовых функций
                adapter.setClearColor(0.1f, 0.2f, 0.3f, 1.0f);
                adapter.enableDepthTest(true);
                
                adapter.beginFrame();
                adapter.clear();
                adapter.endFrame();
                
                std::cout << "Базовый рендеринг работает" << std::endl;
                
                // Тест поддерживаемых функций
                std::cout << "Поддерживаемые функции:" << std::endl;
                std::vector<std::string> features = {
                    "basic_rendering", "wireframe", "depth_test", "blending",
                    "ray_tracing", "gaussian_splatting", "compute_shaders"
                };
                
                for (const auto& feature : features) {
                    bool supported = adapter.supportsFeature(feature);
                    std::cout << "  - " << feature << ": " 
                              << (supported ? "✓" : "✗") << std::endl;
                }
                
                adapter.cleanup();
            } else {
                std::cout << "Ошибка инициализации " << backendName << std::endl;
            }
        } else {
            std::cout << "Не удалось переключиться на " << backendName << std::endl;
        }
    }
}

/**
 * @brief Демонстрация автоматического выбора backend'а
 */
void demonstrateAutoBackendSelection() {
    std::cout << "\n=== Демонстрация автоматического выбора backend'а ===" << std::endl;
    
    RendererAdapter& adapter = RendererAdapter::getInstance();
    
    // Очищаем текущий backend
    adapter.cleanup();
    
    // Устанавливаем AUTO backend
    if (adapter.setBackend(RenderBackend::AUTO)) {
        std::cout << "Автоматически выбран backend: " << adapter.getBackendName() << std::endl;
        
        // Тестируем выбранный backend
        demonstrateBasicRendering(adapter);
        
        adapter.cleanup();
    } else {
        std::cout << "Ошибка автоматического выбора backend'а" << std::endl;
    }
}

/**
 * @brief Демонстрация обработки ошибок
 */
void demonstrateErrorHandling() {
    std::cout << "\n=== Демонстрация обработки ошибок ===" << std::endl;
    
    RendererAdapter& adapter = RendererAdapter::getInstance();
    
    // Попытка рендеринга без инициализации
    std::cout << "Тест рендеринга без инициализации..." << std::endl;
    
    auto mesh = Mesh3D::createCube(1.0f);
    auto shader = Shader3D::createBasicShader();
    
    if (mesh && shader) {
        adapter.beginFrame();  // Должно быть безопасно
        adapter.renderMesh(*mesh, Matrix4::identity(), *shader);  // Должно быть безопасно
        adapter.endFrame();    // Должно быть безопасно
        std::cout << "Обработка ошибок работает корректно" << std::endl;
    }
    
    // Тест недоступного backend'а (если есть)
    std::cout << "Тест недоступных backend'ов..." << std::endl;
    
    // Проверяем каждый backend на доступность
    std::vector<RenderBackend> allBackends = {
        RenderBackend::OPENGL, 
        RenderBackend::VULKAN
    };
    
    for (auto backend : allBackends) {
        const char* name = (backend == RenderBackend::OPENGL) ? "OpenGL" : "Vulkan";
        
        if (!adapter.isBackendAvailable(backend)) {
            std::cout << "Backend " << name << " недоступен (ожидаемо)" << std::endl;
        } else {
            std::cout << "Backend " << name << " доступен" << std::endl;
        }
    }
}

/**
 * @brief Главная функция демонстрации
 */
int main() {
    Console::initialize();
    std::cout << "=== Демонстрация RendererAdapter ===" << std::endl;
    std::cout << "Версия движка: HyperEngine v1.0.0" << std::endl;
    std::cout << "Этап разработки: 2.2 Интеграция с существующим кодом" << std::endl;
    
    try {
        // 1. Демонстрация автоматического выбора
        demonstrateAutoBackendSelection();
        
        // 2. Демонстрация переключения backend'ов
        demonstrateBackendSwitching();
        
        // 3. Демонстрация обработки ошибок
        demonstrateErrorHandling();
        
        std::cout << "\n=== Демонстрация завершена успешно ===" << std::endl;
        
    } catch (const std::exception& e) {
        std::cerr << "Ошибка во время демонстрации: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}

/**
 * @brief Дополнительная функция для тестирования производительности
 */
void performanceComparison() {
    std::cout << "\n=== Сравнение производительности backend'ов ===" << std::endl;
    
    RendererAdapter& adapter = RendererAdapter::getInstance();
    auto availableBackends = adapter.getAvailableBackends();
    
    // Создаем тестовые данные
    auto mesh = Mesh3D::createSphere(1.0f, 64);  // Детализированная сфера
    auto shader = Shader3D::createPhongShader();
    
    if (!mesh || !shader) {
        std::cout << "Не удалось создать тестовые данные" << std::endl;
        return;
    }
    
    const int TEST_FRAMES = 100;
    
    for (auto backend : availableBackends) {
        if (backend == RenderBackend::AUTO) continue;
        
        const char* backendName = "";
        switch (backend) {
            case RenderBackend::OPENGL: backendName = "OpenGL"; break;
            case RenderBackend::VULKAN: backendName = "Vulkan"; break;
            default: continue;
        }
        
        if (!adapter.setBackend(backend) || !adapter.initialize(1024, 768)) {
            std::cout << "Не удалось инициализировать " << backendName << std::endl;
            continue;
        }
        
        std::cout << "Тестирование производительности " << backendName << "..." << std::endl;
        
        auto startTime = std::chrono::high_resolution_clock::now();
        
        for (int frame = 0; frame < TEST_FRAMES; ++frame) {
            adapter.beginFrame();
            adapter.clear();
            
            // Рендерим несколько объектов
            for (int i = 0; i < 10; ++i) {
                Matrix4 transform = Matrix4::translation(Vector3(i * 2.0f - 10.0f, 0, 0)) *
                                   Matrix4::rotationY(frame * 0.01f + i);
                adapter.renderMesh(*mesh, transform, *shader);
            }
            
            adapter.endFrame();
        }
        
        auto endTime = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime);
        
        float fps = (float)TEST_FRAMES / (duration.count() / 1000.0f);
        
        std::cout << backendName << " результаты:" << std::endl;
        std::cout << "  - Время: " << duration.count() << " мс" << std::endl;
        std::cout << "  - FPS: " << fps << std::endl;
        std::cout << "  - Время на кадр: " << (duration.count() / (float)TEST_FRAMES) << " мс" << std::endl;
        
        adapter.cleanup();
    }
}
