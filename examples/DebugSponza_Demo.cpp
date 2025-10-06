#include <iostream>
#include <memory>
#include <chrono> // для std::this_thread::sleep_for
#include "SpectraForge/App/Engine.h"
#include "SpectraForge/App/Config.h"
#include "SpectraForge/Core/Console.h"
#include "SpectraForge/Core/Logger.h"
#include "SpectraForge/Core/SafeConsole.h"
#include "SpectraForge/Rendering/Common/IRenderer.h"
#include "SpectraForge/Rendering/Common/IResourceManager.h"
#include "SpectraForge/Vulkan/SceneManager.h"

using namespace SpectraForge;

class DebugDemo {
public:
    DebugDemo() = default;
    ~DebugDemo() = default;

    bool initialize() {
        SAFE_PRINT_LINE("=== DEBUG DEMO FOR SPONZA RENDERING ===");
        SAFE_PRINT_LINE("");
        
        // Конфигурация с debug параметрами
        SpectraForge::App::AppConfig cfg;
        cfg.window_width = 960;
        cfg.window_height = 540;
        cfg.window_title = "Debug Demo - Sponza Rendering";
        
        // КРИТИЧНО: Добавить debug флаги
        cfg.enableValidationLayers = true;  // Vulkan validation
        cfg.enableDebugOutput = true;       // Detailed logging
        cfg.enableFrameDebugger = true;     // Frame-by-frame analysis
        
        std::cout << "🔍 [DEBUG] Разрешение: " << cfg.window_width << "x" << cfg.window_height << std::endl;
        std::cout << "🔍 [DEBUG] Validation layers: ENABLED" << std::endl;
        
        auto logger = std::make_shared<SpectraForge::Core::Logger>("", SpectraForge::Core::LogLevel::DEBUG_LEVEL);
        app_ = std::make_unique<SpectraForge::App::Engine>(cfg, logger);
        
        if (!app_->init()) {
            SAFE_ERROR("Ошибка инициализации App::Engine");
            return false;
        }
        
        std::cout << "✅ [DEBUG] Engine инициализирован" << std::endl;
        
        // Загрузка сцены с подробной диагностикой
        SpectraForge::Vulkan::SceneData scene{};
        scene.scenePath = "examples/scenes/sponza/sponza.obj";
        
        // ДИАГНОСТИКА: Проверка существования файла
        std::ifstream file(scene.scenePath);
        if (!file.good()) {
            std::cerr << "❌ [ERROR] Файл сцены не найден: " << scene.scenePath << std::endl;
            std::cerr << "📝 [INFO] Проверьте путь к файлу и убедитесь что он существует" << std::endl;
            return false;
        }
        file.close();
        std::cout << "✅ [DEBUG] Файл сцены найден: " << scene.scenePath << std::endl;
        
        // Для отладки: уменьшаем количество треугольников
        scene.triangleStep = 100;
        
        // КРИТИЧНО: Включить debug режимы
        scene.enableDebugOutput = true;     // Подробный вывод загрузки
        scene.enableBoundsCheck = true;     // Проверка AABB сцены
        scene.enableMaterialDebug = true;   // Информация о материалах
        
        std::cout << "🔺 [DEBUG] Загрузка сцены с triangle step = " << scene.triangleStep << std::endl;
        
        // Загрузка с подробной диагностикой
        bool sceneLoaded = app_->load_scene(scene);
        if (!sceneLoaded) {
            std::cerr << "❌ [ERROR] Не удалось загрузить сцену" << std::endl;
            return false;
        }
        
        std::cout << "✅ [DEBUG] Сцена загружена успешно" << std::endl;
        
        // ДИАГНОСТИКА: Получить информацию о загруженной сцене
        auto sceneInfo = app_->getSceneInfo();
        std::cout << "📊 [DEBUG] Статистика сцены:" << std::endl;
        std::cout << "   • Треугольников загружено: " << sceneInfo.triangleCount << std::endl;
        std::cout << "   • Материалов: " << sceneInfo.materialCount << std::endl;
        std::cout << "   • AABB сцены: min(" << sceneInfo.bounds.min.x << ", " << sceneInfo.bounds.min.y << ", " << sceneInfo.bounds.min.z << ")" << std::endl;
        std::cout << "   • AABB сцены: max(" << sceneInfo.bounds.max.x << ", " << sceneInfo.bounds.max.y << ", " << sceneInfo.bounds.max.z << ")" << std::endl;
        
        // ДИАГНОСТИКА: Проверить и настроить камеру
        auto camera = app_->getCamera();
        if (camera) {
            // Позиционировать камеру в центре Sponza
            // Sponza обычно имеет размеры примерно (-12, -1, -5) до (12, 20, 5)
            auto cameraPos = SpectraForge::Math::Vector3(0.0f, 5.0f, 10.0f);  // Смотрим в центр
            auto lookAt = SpectraForge::Math::Vector3(0.0f, 5.0f, 0.0f);      // Центр сцены
            auto up = SpectraForge::Math::Vector3(0.0f, 1.0f, 0.0f);
            
            camera->setPosition(cameraPos);
            camera->lookAt(lookAt, up);
            
            std::cout << "📹 [DEBUG] Камера установлена:" << std::endl;
            std::cout << "   • Позиция: (" << cameraPos.x << ", " << cameraPos.y << ", " << cameraPos.z << ")" << std::endl;
            std::cout << "   • Направление: (" << lookAt.x << ", " << lookAt.y << ", " << lookAt.z << ")" << std::endl;
            std::cout << "   • FOV: " << camera->getFOV() << "°" << std::endl;
            std::cout << "   • Near: " << camera->getNear() << ", Far: " << camera->getFar() << std::endl;
        }
        
        // КРИТИЧНО: Включить debug режимы в рендерере
        auto renderer = app_->getRenderer();
        if (renderer) {
            // Включить различные debug режимы
            renderer->setDebugMode(1);              // SDF visualization
            renderer->enableWireframe(false);       // Пока отключим wireframe
            renderer->enableBackfaceCulling(true);  // Стандартный backface culling
            renderer->enableDepthTest(true);        // Depth testing
            renderer->setBackgroundColor(0.1f, 0.2f, 0.3f, 1.0f); // Синий фон для контраста
            
            std::cout << "🎨 [DEBUG] Renderer настроен:" << std::endl;
            std::cout << "   • Debug mode: SDF visualization" << std::endl;
            std::cout << "   • Background: синий (0.1, 0.2, 0.3)" << std::endl;
            std::cout << "   • Backface culling: включен" << std::endl;
        }
        
        SAFE_PRINT_LINE("");
        SAFE_PRINT_LINE("✅ Debug инициализация завершена!");
        SAFE_PRINT_LINE("");
        SAFE_PRINT_LINE("🔍 Debug режимы:");
        SAFE_PRINT_LINE("   1 - SDF visualization (красный = внутри треугольника)");
        SAFE_PRINT_LINE("   2 - Barycentric coordinates");
        SAFE_PRINT_LINE("   0 - Нормальный рендеринг");
        SAFE_PRINT_LINE("");
        SAFE_PRINT_LINE("Управление:");
        SAFE_PRINT_LINE("   WASD - движение камеры");
        SAFE_PRINT_LINE("   Мышь - поворот камеры");
        SAFE_PRINT_LINE("   1,2,0 - переключение debug режимов");
        SAFE_PRINT_LINE("   R - сброс камеры");
        SAFE_PRINT_LINE("   ESC - выход");
        SAFE_PRINT_LINE("");
        
        return true;
    }
    
    void run() {
        auto lastTime = std::chrono::high_resolution_clock::now();
        uint32_t frameCount = 0;
        float totalTime = 0.0f;
        
        constexpr float TARGET_FPS = 30.0f;  // Понижаем для debug
        constexpr float FRAME_TIME_MS = 1000.0f / TARGET_FPS;
        
        std::cout << "🎮 [DEBUG] Запуск debug render loop (FPS target: " << TARGET_FPS << ")" << std::endl;
        
        while (!app_->should_close()) {
            auto frameStart = std::chrono::high_resolution_clock::now();
            auto currentTime = std::chrono::high_resolution_clock::now();
            deltaTime_ = std::chrono::duration<float>(currentTime - lastTime).count();
            lastTime = currentTime;
            
            // Debug input handling
            handleDebugInput();
            
            // Обновление и рендеринг
            app_->update(deltaTime_);
            app_->render();
            
            if (app_->should_close()) break;
            
            // Подробная статистика FPS
            frameCount++;
            totalTime += deltaTime_;
            
            if (totalTime >= 2.0f) {  // Каждые 2 секунды
                fps_ = frameCount / totalTime;
                frameCount = 0;
                totalTime = 0.0f;
                
                std::cout << "📊 [DEBUG] FPS: " << static_cast<int>(fps_) 
                         << " | Frame time: " << (1000.0f / fps_) << "ms" << std::endl;
                
                // Дополнительная статистика рендеринга
                auto stats = app_->getRenderStats();
                std::cout << "   • Треугольников после culling: " << stats.visibleTriangles << std::endl;
                std::cout << "   • Draw calls: " << stats.drawCalls << std::endl;
            }
            
            // Ограничение FPS
            auto frameEnd = std::chrono::high_resolution_clock::now();
            auto frameDuration = std::chrono::duration<float, std::milli>(frameEnd - frameStart).count();
            float remainingTime = FRAME_TIME_MS - frameDuration;
            
            if (remainingTime > 0.0f) {
                std::this_thread::sleep_for(
                    std::chrono::microseconds(static_cast<int>(remainingTime * 1000.0f))
                );
            }
        }
        
        std::cout << "🏁 [DEBUG] Render loop завершён" << std::endl;
    }
    
    void handleDebugInput() {
        auto input = app_->getInputManager();
        if (!input) return;
        
        // Переключение debug режимов
        if (input->isKeyPressed('1')) {
            app_->getRenderer()->setDebugMode(1);  // SDF
            std::cout << "🔍 [DEBUG] Режим: SDF visualization" << std::endl;
        }
        if (input->isKeyPressed('2')) {
            app_->getRenderer()->setDebugMode(2);  // Barycentric
            std::cout << "🔍 [DEBUG] Режим: Barycentric coordinates" << std::endl;
        }
        if (input->isKeyPressed('0')) {
            app_->getRenderer()->setDebugMode(0);  // Normal
            std::cout << "🔍 [DEBUG] Режим: Нормальный рендеринг" << std::endl;
        }
        
        // Сброс камеры
        if (input->isKeyPressed('R')) {
            auto camera = app_->getCamera();
            if (camera) {
                auto cameraPos = SpectraForge::Math::Vector3(0.0f, 5.0f, 10.0f);
                auto lookAt = SpectraForge::Math::Vector3(0.0f, 5.0f, 0.0f);
                auto up = SpectraForge::Math::Vector3(0.0f, 1.0f, 0.0f);
                
                camera->setPosition(cameraPos);
                camera->lookAt(lookAt, up);
                
                std::cout << "📹 [DEBUG] Камера сброшена" << std::endl;
            }
        }
    }
    
    void shutdown() {
        SAFE_PRINT_LINE("🔍 [DEBUG] Завершение debug demo...");
        if (app_) {
            app_->shutdown();
            app_.reset();
        }
        SAFE_PRINT_LINE("✅ [DEBUG] Завершено!");
    }
    
private:
    std::unique_ptr<SpectraForge::App::Engine> app_;
    float deltaTime_ = 0.0f;
    float fps_ = 0.0f;
};

// ============================================================================
// Main
// ============================================================================

int main() {
    SpectraForge::Core::Console::initialize();
    
    DebugDemo demo;
    try {
        if (!demo.initialize()) {
            SAFE_ERROR("❌ [ERROR] Ошибка инициализации debug demo");
            return -1;
        }
        
        demo.run();
        demo.shutdown();
        
    } catch (const std::exception& e) {
        std::cerr << "💥 [CRITICAL] Критическая ошибка: " << e.what() << std::endl;
        return -1;
    }
    
    return 0;
}