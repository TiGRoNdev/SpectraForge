#include <chrono>
#include <iostream>
#include <thread>
#include "SpectraForge/App/Engine.h"
#include "SpectraForge/App/Config.h"
#include "SpectraForge/Core/Console.h"
#include "SpectraForge/Core/Logger.h"
#include "SpectraForge/Core/SafeConsole.h"
#include "SpectraForge/Rendering/Common/IRenderer.h"
#include "SpectraForge/Rendering/Common/IResourceManager.h"
#include "SpectraForge/Rendering/HybridFreGSRenderer.h"
#include "SpectraForge/Rendering/RenderPass/TriangleSplattingPass.h"
#include "SpectraForge/Vulkan/SceneManager.h"

using namespace SpectraForge;

class SpectraForgeDemoFinalFixed {
  public:
    SpectraForgeDemoFinalFixed() = default;
    ~SpectraForgeDemoFinalFixed() = default;

    bool initialize() {
        SAFE_PRINT_LINE("=== SpectraForge Demo - FINAL FIXED VERSION ===");
        SAFE_PRINT_LINE("");

        SpectraForge::App::AppConfig cfg;
        cfg.window_width = 960;
        cfg.window_height = 540;
        cfg.window_title = "SpectraForge Demo - FINAL FIXED";

        std::cout << "[DemoFINAL] 📺 Resolution: " << cfg.window_width << "x" << cfg.window_height << std::endl;

        auto logger = std::make_shared<SpectraForge::Core::Logger>("", SpectraForge::Core::LogLevel::INFO_LEVEL);

        app_ = std::make_unique<SpectraForge::App::Engine>(cfg, logger);
        if (!app_->init()) {
            SAFE_ERROR("Ошибка инициализации App::Engine");
            return false;
        }

        // ✅ КРИТИЧЕСКИЙ ФИКС: Включаем внешнее управление камерой
        app_->setExternalCameraControl(true);
        std::cout << "[DemoFINAL] ✅ External camera control enabled" << std::endl;

        // ✅ КРИТИЧЕСКИЙ ФИКС: ПРАВИЛЬНАЯ позиция камеры для Sponza
        // Анализ логов показал: Sponza AABB Z ∈ [-7.8, 7.8], camera была в Z=-8 (снаружи!)
        auto camera = app_->getCamera();
        if (camera) {
            // ИСПРАВЛЕНО: Камера ВНУТРИ Sponza для правильного обзора
            camera->setPosition(SpectraForge::Math::Vector3(0.0f, 5.0f, 2.0f));  // Внутри, выше пола
            camera->lookAt(
                SpectraForge::Math::Vector3(0.0f, 5.0f, 2.0f),     // Позиция  
                SpectraForge::Math::Vector3(0.0f, 2.0f, -3.0f),    // Смотрим внутрь Sponza
                SpectraForge::Math::Vector3(0.0f, 1.0f, 0.0f)      // Up vector
            );
            
            float aspect = static_cast<float>(cfg.window_width) / static_cast<float>(cfg.window_height);
            camera->setPerspective(75.0f, aspect, 0.1f, 100.0f);  // Wider FOV для лучшего обзора
            
            std::cout << "[DemoFINAL] 📷 Camera ВНУТРИ Sponza: (0, 5, 2) → (0, 2, -3)" << std::endl;
        }

        // Загрузка сцены с улучшенными параметрами
        SpectraForge::Vulkan::SceneData scene{};
        scene.scenePath = "examples/scenes/sponza/sponza.obj";
        scene.triangleStep = 25;  // Еще больше треугольников для плотности
        
        std::cout << "[DemoFINAL] 🔺 Loading scene with triangle step=" << scene.triangleStep << std::endl;
        
        if (!app_->load_scene(scene)) {
            std::cout << "[DemoFINAL] ⚠️  Sponza not found, creating VISIBLE test scene..." << std::endl;
            createVisibleTestScene();
        }

        // ✅ КРИТИЧЕСКИЙ ФИКС: Debug mode 2 для визуализации проекций
        auto renderer = std::dynamic_pointer_cast<SpectraForge::Rendering::HybridFreGSRenderer>(app_->getRenderer());
        if (renderer) {
            auto triangleSplattingPass = renderer->getTriangleSplattingPass();
            if (triangleSplattingPass) {
                triangleSplattingPass->setDebugMode(2);  // Проекция треугольников + wireframe
                std::cout << "[DemoFINAL] ✅ Debug mode 2: triangle projection visualization" << std::endl;
            }
        }

        SAFE_PRINT_LINE("✅ FINAL FIXED Initialization completed!");
        SAFE_PRINT_LINE("Camera positioned INSIDE Sponza for proper view");
        SAFE_PRINT_LINE("Debug mode 2: should show triangle projections");

        return true;
    }

    void run() {
        auto lastTime = std::chrono::high_resolution_clock::now();
        uint32_t frameCount = 0;
        float totalTime = 0.0f;
        
        constexpr float TARGET_FPS = 60.0f;
        constexpr float FRAME_TIME_MS = 1000.0f / TARGET_FPS;
        
        std::cout << "[DemoFINAL] 🎮 Starting with FIXED camera position" << std::endl;
        
        while (!app_->should_close()) {
            auto frameStart = std::chrono::high_resolution_clock::now();
            
            auto currentTime = std::chrono::high_resolution_clock::now();
            deltaTime_ = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - lastTime).count();
            lastTime = currentTime;

            app_->update(deltaTime_);
            app_->render();

            if (app_->should_close()) break;

            frameCount++;
            totalTime += deltaTime_;

            if (totalTime >= 3.0f) {  // Каждые 3 секунды
                fps_ = frameCount / totalTime;
                frameCount = 0;
                totalTime = 0.0f;
                std::cout << "[DemoFINAL] 📊 FPS: " << static_cast<int>(fps_) 
                         << " (Camera INSIDE Sponza)" << std::endl;
            }
            
            auto frameEnd = std::chrono::high_resolution_clock::now();
            auto frameDuration = std::chrono::duration<float, std::milli>(frameEnd - frameStart).count();
            float remainingTime = FRAME_TIME_MS - frameDuration;
            
            if (remainingTime > 0.0f) {
                std::this_thread::sleep_for(
                    std::chrono::microseconds(static_cast<int>(remainingTime * 1000.0f))
                );
            }
        }
        
        std::cout << "[DemoFINAL] 🏁 Render loop completed" << std::endl;
    }

    void shutdown() {
        SAFE_PRINT_LINE("Shutting down FINAL FIXED demo...");

        if (app_) {
            app_->shutdown();
            app_.reset();
        }

        SAFE_PRINT_LINE("✅ FINAL FIXED shutdown completed!");
    }

private:
    std::unique_ptr<SpectraForge::App::Engine> app_;
    float deltaTime_ = 0.0f;
    float fps_ = 0.0f;

    // ✅ НОВЫЙ МЕТОД: Создание гарантированно видимой тестовой сцены
    void createVisibleTestScene() {
        auto renderer = std::dynamic_pointer_cast<SpectraForge::Rendering::HybridFreGSRenderer>(app_->getRenderer());
        if (!renderer) return;

        std::vector<spectraforge::rendering::TriangleSplattingPass::Triangle> triangles;

        // Создаем треугольники которые ГАРАНТИРОВАННО видны из камеры (0, 5, 2) → (0, 2, -3)
        // Размещаем их в области Z ∈ [-6, 0] прямо перед камерой

        for (int i = 0; i < 7; ++i) {
            spectraforge::rendering::TriangleSplattingPass::Triangle tri;
            
            float z = -1.0f - i * 0.8f;  // z = -1, -1.8, -2.6, -3.4, -4.2, -5.0, -5.8
            float x = (i - 3) * 3.0f;    // Разносим по X: -9, -6, -3, 0, 3, 6, 9
            
            // Большие треугольники для гарантированной видимости
            tri.v0 = glm::vec3(x - 1.5f, 1.0f, z);
            tri.v1 = glm::vec3(x + 1.5f, 1.0f, z);  
            tri.v2 = glm::vec3(x, 4.0f, z);
            
            // Яркие цвета для хорошего contrast
            switch (i) {
                case 0: tri.color = glm::vec3(1.0f, 0.0f, 0.0f); break; // Красный
                case 1: tri.color = glm::vec3(0.0f, 1.0f, 0.0f); break; // Зеленый  
                case 2: tri.color = glm::vec3(0.0f, 0.0f, 1.0f); break; // Синий
                case 3: tri.color = glm::vec3(1.0f, 1.0f, 0.0f); break; // Желтый
                case 4: tri.color = glm::vec3(1.0f, 0.0f, 1.0f); break; // Магента
                case 5: tri.color = glm::vec3(0.0f, 1.0f, 1.0f); break; // Циан
                case 6: tri.color = glm::vec3(1.0f, 0.5f, 0.0f); break; // Оранжевый
            }
            
            tri.opacity = 1.0f;
            tri.sigma = 1.2f;
            tri.normal = glm::vec3(0.0f, 0.0f, 1.0f);  // Смотрят на камеру
            tri.materialId = i;
            
            // Добавляем текстурные координаты
            tri.texCoord0 = glm::vec2(0.0f, 0.0f);
            tri.texCoord1 = glm::vec2(1.0f, 0.0f);
            tri.texCoord2 = glm::vec2(0.5f, 1.0f);
            
            triangles.push_back(tri);
        }

        renderer->uploadTriangles(triangles);
        std::cout << "[DemoFINAL] ✅ Created " << triangles.size() 
                  << " GUARANTEED VISIBLE triangles" << std::endl;
        
        // Выводим координаты первого треугольника для проверки
        if (!triangles.empty()) {
            const auto& t0 = triangles[0];
            std::cout << "[DemoFINAL] 🔺 First triangle: v0=(" << t0.v0.x << "," << t0.v0.y 
                      << "," << t0.v0.z << ")" << std::endl;
            std::cout << "[DemoFINAL] 📷 Camera at (0,5,2) should clearly see Z=" << t0.v0.z << std::endl;
        }
    }
};

int main() {
    SpectraForge::Core::Console::initialize();

    SpectraForgeDemoFinalFixed demo;

    try {
        if (!demo.initialize()) {
            SAFE_ERROR("Ошибка инициализации FINAL FIXED демо");
            return -1;
        }

        demo.run();
        demo.shutdown();

    } catch (const std::exception& e) {
        std::cerr << "💥 Критическая ошибка: " << e.what() << std::endl;
        return -1;
    }

    return 0;
}