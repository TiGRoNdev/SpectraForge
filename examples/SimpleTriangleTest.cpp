/**
 * @file SimpleTriangleTest.cpp
 * @brief Минимальный тест для отладки Triangle Splatting
 * 
 * Создаёт один треугольник в центре экрана для проверки рендеринга
 */

#include <iostream>
#include <memory>
#include "SpectraForge/App/Engine.h"
#include "SpectraForge/App/Config.h"
#include "SpectraForge/Core/Console.h"
#include "SpectraForge/Core/Logger.h"
#include "SpectraForge/Core/SafeConsole.h"
#include "SpectraForge/Rendering/Common/IRenderer.h"
#include "SpectraForge/Rendering/RenderPass/TriangleSplattingPass.h"
#include "SpectraForge/Rendering/HybridFreGSRenderer.h"

using namespace SpectraForge;
using namespace spectraforge::rendering;

class SimpleTriangleTest {
public:
    bool initialize() {
        // Минимальная конфигурация
        SpectraForge::App::AppConfig cfg;
        cfg.window_width = 800;
        cfg.window_height = 600;
        cfg.window_title = "Simple Triangle Test";
        cfg.enableValidationLayers = true; // Включаем для отладки
        
        std::cout << "[Test] 🔺 Создаём один треугольник для отладки" << std::endl;
        
        auto logger = std::make_shared<SpectraForge::Core::Logger>("", 
                     SpectraForge::Core::LogLevel::DEBUG_LEVEL);
        
        app_ = std::make_unique<SpectraForge::App::Engine>(cfg, logger);
        if (!app_->init()) {
            SAFE_ERROR("Ошибка инициализации Engine");
            return false;
        }
        
        // Создаём один треугольник вручную
        createSingleTriangle();
        
        return true;
    }
    
    void run() {
        std::cout << "[Test] ▶️ Запуск рендер цикла..." << std::endl;
        
        while (!app_->should_close()) {
            // Обрабатываем события через движок
            
            // Обновляем движок
            float deltaTime = 0.016f; // 60 FPS
            app_->update(deltaTime);
            
            // Рендерим
            app_->render();
        }
        
        std::cout << "[Test] ✅ Тест завершён" << std::endl;
    }
    
private:
    void createSingleTriangle() {
        auto renderer = std::dynamic_pointer_cast<SpectraForge::Rendering::HybridFreGSRenderer>(
            app_->getRenderer()
        );
        
        if (!renderer) {
            SAFE_ERROR("Не удалось получить HybridFreGSRenderer");
            return;
        }
        
        // Создаём один треугольник в NDC координатах
        std::vector<spectraforge::rendering::Triangle> triangles;
        
        spectraforge::rendering::Triangle tri;
        
        // Треугольник ДАЛЕКО перед камерой для тестирования
        tri.v0 = glm::vec3(-5.0f, -5.0f, -10.0f);
        tri.v1 = glm::vec3( 5.0f, -5.0f, -10.0f);
        tri.v2 = glm::vec3( 0.0f,  5.0f, -10.0f);
        
        // Ярко-красный цвет
        tri.color = glm::vec3(1.0f, 0.0f, 0.0f);
        tri.opacity = 1.0f;
        
        // Параметры рендеринга
        tri.sigma = 1.0f; // Резкие края
        tri.normal = glm::vec3(0.0f, 0.0f, 1.0f); // Смотрит на камеру
        tri.materialId = 0;
        
        // Текстурные координаты
        tri.texCoord0 = glm::vec2(0.0f, 0.0f);
        tri.texCoord1 = glm::vec2(1.0f, 0.0f);
        tri.texCoord2 = glm::vec2(0.5f, 1.0f);
        
        triangles.push_back(tri);
        
        std::cout << "[Test] 🔺 Создан треугольник:" << std::endl;
        std::cout << "  v0: (" << tri.v0.x << ", " << tri.v0.y << ", " << tri.v0.z << ")" << std::endl;
        std::cout << "  v1: (" << tri.v1.x << ", " << tri.v1.y << ", " << tri.v1.z << ")" << std::endl;
        std::cout << "  v2: (" << tri.v2.x << ", " << tri.v2.y << ", " << tri.v2.z << ")" << std::endl;
        std::cout << "  color: RGB(" << tri.color.r << ", " << tri.color.g << ", " << tri.color.b << ")" << std::endl;
        std::cout << "  sigma: " << tri.sigma << ", opacity: " << tri.opacity << std::endl;
        
        // Включаем внешнее управление камерой ДО загрузки сцены
        app_->setExternalCameraControl(true);
        
        // Устанавливаем камеру в позицию (0, 0, 2) смотрящую на треугольник в z=-1
        auto camera = app_->getCamera();
        if (camera) {
            camera->setPosition(SpectraForge::Math::Vector3(0.0f, 0.0f, 2.0f));
            camera->lookAt(SpectraForge::Math::Vector3(0.0f, 0.0f, 2.0f),     // Позиция камеры
                          SpectraForge::Math::Vector3(0.0f, 0.0f, -1.0f),   // Смотрим на треугольник
                          SpectraForge::Math::Vector3(0.0f, 1.0f, 0.0f));   // Up = +Y
            std::cout << "[Test] 📷 Камера установлена в (0, 0, 2) смотрит на треугольник в z=-1" << std::endl;
            std::cout << "[Test] 🔒 Внешнее управление камерой активировано" << std::endl;
            
            // Выводим позицию камеры для отладки
            auto pos = camera->getPosition();
            std::cout << "[Test] 📍 Позиция камеры: (" << pos.x << ", " << pos.y << ", " << pos.z << ")" << std::endl;
        }
        
        // Загружаем треугольник в рендерер
        renderer->uploadTriangles(triangles);
        
        // Debug mode 3: простая визуализация проекции
        auto triangleSplattingPass = renderer->getTriangleSplattingPass();
        if (triangleSplattingPass) {
            triangleSplattingPass->setDebugMode(3);
            std::cout << "[Test] 🎯 Debug mode 3: простая визуализация проекции" << std::endl;
        }
    }
    
    std::unique_ptr<SpectraForge::App::Engine> app_;
};

int main(int argc, char* argv[]) {
    std::cout << "=== Simple Triangle Test ===" << std::endl;
    std::cout << "Минимальный тест для отладки Triangle Splatting" << std::endl;
    
    SimpleTriangleTest test;
    
    if (!test.initialize()) {
        std::cerr << "❌ Ошибка инициализации!" << std::endl;
        return -1;
    }
    
    test.run();
    
    return 0;
}
