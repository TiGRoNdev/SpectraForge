/**
 * @file DebugSponza_Demo_Final.cpp
 * @brief Финальная исправленная версия DebugSponza_Demo.cpp с использованием всех новых API
 * 
 * ИНСТРУКЦИЯ ПО ИНТЕГРАЦИИ:
 * 1. Заменить examples/DebugSponza_Demo.cpp на этот файл
 * 2. Убедиться что все расширенные header файлы интегрированы
 * 3. Скомпилировать и запустить
 */

#include <iostream>
#include <memory>
#include <chrono>
#include <thread>
#include <fstream>
#include "SpectraForge/App/Engine.h"
#include <GLFW/glfw3.h>
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
        SAFE_PRINT_LINE("=== SPECTRAFORGE DEBUG DEMO - TRIANGLE SPLATTING ===");
        SAFE_PRINT_LINE("Версия с полным debug API support");
        SAFE_PRINT_LINE("");
        
        // Конфигурация с использованием РАСШИРЕННЫХ полей
        SpectraForge::App::AppConfig cfg;
        cfg.window_width = 1280;
        cfg.window_height = 720;
        cfg.window_title = "SpectraForge Debug Demo - Triangle Splatting";
        
        // ✅ ИСПОЛЬЗУЕМ НОВЫЕ debug поля из расширенного AppConfig
        cfg.debug = true;
        cfg.enableValidationLayers = true;     // Vulkan validation
        cfg.enableDebugOutput = true;          // Подробное логирование
        cfg.enableFrameDebugger = false;       // Frame-by-frame analysis (отключено)
        cfg.enableGPUTiming = true;            // GPU performance timing
        cfg.enableMemoryTracking = true;       // Memory usage tracking
        
        // Rendering параметры
        cfg.enableWireframe = false;           // Wireframe mode
        cfg.enableBackfaceCulling = true;      // Backface culling
        cfg.enableDepthTest = true;            // Depth testing
        cfg.enableAlphaBlending = true;        // Alpha blending
        
        // Performance параметры
        cfg.targetFPS = 60;                    // Target FPS
        cfg.maxTrianglesPerFrame = 50000;      // Triangle budget (понижен)
        cfg.enableEarlyTermination = true;     // Early alpha termination
        cfg.enableTileCulling = true;          // Mobile GPU optimization
        
        // Scene параметры
        cfg.cameraSpeed = 5.0f;                // Camera movement speed
        cfg.mouseSensitivity = 0.15f;          // Mouse sensitivity
        cfg.invertMouseY = false;              // Invert Y-axis
        
        // Background color (темно-синий)
        cfg.backgroundColor[0] = 0.05f;
        cfg.backgroundColor[1] = 0.1f;
        cfg.backgroundColor[2] = 0.2f;
        cfg.backgroundColor[3] = 1.0f;
        
        // Debug режим по умолчанию
        cfg.debugMode = SpectraForge::App::AppConfig::DebugMode::NORMAL;
        
        cfg.vsync = false;  // Отключаем для стабильности
        
        std::cout << "🔍 [DEBUG] Инициализация с расширенной конфигурацией:" << std::endl;
        std::cout << "   • Разрешение: " << cfg.window_width << "x" << cfg.window_height << std::endl;
        std::cout << "   • Debug mode: " << (cfg.debug ? "ENABLED" : "DISABLED") << std::endl;
        std::cout << "   • Validation layers: " << (cfg.enableValidationLayers ? "ON" : "OFF") << std::endl;
        std::cout << "   • Triangle budget: " << cfg.maxTrianglesPerFrame << std::endl;
        std::cout << "   • Background: (" << cfg.backgroundColor[0] << ", " 
                  << cfg.backgroundColor[1] << ", " << cfg.backgroundColor[2] << ")" << std::endl;
        
        auto logger = std::make_shared<SpectraForge::Core::Logger>("", SpectraForge::Core::LogLevel::DEBUG_LEVEL);
        app_ = std::make_unique<SpectraForge::App::Engine>(cfg, logger);
        
        if (!app_->init()) {
            SAFE_ERROR("❌ Ошибка инициализации App::Engine");
            return false;
        }
        
        std::cout << "✅ [DEBUG] Engine инициализирован с расширенными возможностями" << std::endl;
        
        // ✅ ТЕСТИРУЕМ НОВЫЕ DEBUG API МЕТОДЫ
        testNewDebugAPI();
        
        // Загрузка сцены Sponza
        if (!loadSponzaScene()) {
            SAFE_ERROR("❌ Не удалось загрузить сцену Sponza");
            return false;
        }
        
        // ✅ ДИАГНОСТИКА через новые API
        printAdvancedDiagnostics();
        
        SAFE_PRINT_LINE("");
        SAFE_PRINT_LINE("✅ Инициализация завершена! Новые возможности:");
        SAFE_PRINT_LINE("");
        SAFE_PRINT_LINE("🎮 Управление:");
        SAFE_PRINT_LINE("   WASD - движение камеры");
        SAFE_PRINT_LINE("   Мышь - поворот камеры");
        SAFE_PRINT_LINE("   R - сброс камеры для Sponza");
        SAFE_PRINT_LINE("   F - переключение wireframe");
        SAFE_PRINT_LINE("   1 - Обычный рендеринг");
        SAFE_PRINT_LINE("   2 - SDF визуализация");
        SAFE_PRINT_LINE("   3 - Барицентрические координаты");
        SAFE_PRINT_LINE("   4 - Wireframe режим");
        SAFE_PRINT_LINE("   ESC - выход");
        SAFE_PRINT_LINE("");
        
        return true;
    }
    
private:
    void testNewDebugAPI() {
        std::cout << "🧪 [TEST] Тестирование новых debug API методов..." << std::endl;
        
        // ✅ Тест getRenderer()
        auto renderer = app_->getRenderer();
        if (renderer) {
            std::cout << "   ✅ getRenderer(): " << renderer->getName() << std::endl;
            std::cout << "   ✅ API version: " << renderer->getApiVersion() << std::endl;
            
            // Тест новых renderer методов
            renderer->setBackgroundColor(0.05f, 0.1f, 0.2f, 1.0f);
            auto bgColor = renderer->getBackgroundColor();
            std::cout << "   ✅ Background color: (" << bgColor.r << ", " << bgColor.g 
                      << ", " << bgColor.b << ", " << bgColor.a << ")" << std::endl;
            
            // GPU информация
            auto gpuInfo = renderer->getGPUInfo();
            std::cout << "   ✅ GPU: " << gpuInfo.deviceName << std::endl;
            std::cout << "   ✅ Memory: " << (gpuInfo.totalMemory / 1024 / 1024) << " MB total, " 
                      << (gpuInfo.availableMemory / 1024 / 1024) << " MB available" << std::endl;
        } else {
            std::cout << "   ❌ getRenderer() returned null" << std::endl;
        }
        
        // ✅ Тест getCamera()
        auto camera = app_->getCamera();
        if (camera) {
            auto pos = camera->getPosition();
            std::cout << "   ✅ getCamera() position: (" << pos.x << ", " << pos.y << ", " << pos.z << ")" << std::endl;
        } else {
            std::cout << "   ❌ getCamera() returned null" << std::endl;
        }
        
        // ✅ Тест getInputManager()
        auto input = app_->getInputManager();
        if (input) {
            std::cout << "   ✅ getInputManager(): input system active" << std::endl;
        } else {
            std::cout << "   ❌ getInputManager() returned null" << std::endl;
        }
        
        std::cout << "   ✅ Debug API тестирование завершено" << std::endl;
    }
    
    bool loadSponzaScene() {
        SpectraForge::Vulkan::SceneData scene{};
        scene.scenePath = "examples/scenes/sponza/sponza.obj";
        
        // Проверка существования файла с альтернативными путями
        std::vector<std::string> alternativePaths = {
            "examples/scenes/sponza/sponza.obj",
            "assets/sponza/sponza.obj",
            "models/sponza/sponza.obj",
            "data/sponza/sponza.obj",
            "../examples/scenes/sponza/sponza.obj",
            "./sponza.obj"
        };
        
        bool found = false;
        for (const auto& path : alternativePaths) {
            std::ifstream file(path);
            if (file.good()) {
                scene.scenePath = path;
                found = true;
                std::cout << "✅ [DEBUG] Файл Sponza найден: " << path << std::endl;
                break;
            }
        }
        
        if (!found) {
            std::cerr << "❌ [ERROR] Файл Sponza не найден ни по одному из путей:" << std::endl;
            for (const auto& path : alternativePaths) {
                std::cerr << "   • " << path << std::endl;
            }
            return false;
        }
        
        // Установка параметров загрузки
        scene.triangleStep = 50;  // Каждый 50-й треугольник для производительности
        
        std::cout << "🔺 [DEBUG] Загрузка с triangle step = " << scene.triangleStep << std::endl;
        
        // ✅ Используем существующий метод load_scene
        bool sceneLoaded = false;
        try {
            sceneLoaded = app_->load_scene(scene);
        } catch (const std::exception& e) {
            std::cerr << "❌ [ERROR] Исключение при загрузке сцены: " << e.what() << std::endl;
            return false;
        }
        
        if (!sceneLoaded) {
            std::cerr << "❌ [ERROR] load_scene() вернул false" << std::endl;
            return false;
        }
        
        std::cout << "✅ [DEBUG] Сцена Sponza загружена успешно" << std::endl;
        return true;
    }
    
    void printAdvancedDiagnostics() {
        std::cout << "📊 [DIAGNOSTICS] Расширенная диагностика через новые API:" << std::endl;
        
        // ✅ Scene информация через getSceneInfo()
        auto sceneInfo = app_->getSceneInfo();
        std::cout << "🏛️ [SCENE] Информация о сцене:" << std::endl;
        std::cout << "   • Загружена: " << (sceneInfo.isLoaded ? "ДА" : "НЕТ") << std::endl;
        std::cout << "   • Треугольники: " << sceneInfo.triangleCount << std::endl;
        std::cout << "   • Материалы: " << sceneInfo.materialCount << std::endl;
        std::cout << "   • AABB min: (" << sceneInfo.bounds.min.x << ", " 
                  << sceneInfo.bounds.min.y << ", " << sceneInfo.bounds.min.z << ")" << std::endl;
        std::cout << "   • AABB max: (" << sceneInfo.bounds.max.x << ", " 
                  << sceneInfo.bounds.max.y << ", " << sceneInfo.bounds.max.z << ")" << std::endl;
        
        // ✅ Render статистика через getRenderStats()
        auto renderStats = app_->getRenderStats();
        std::cout << "🎮 [RENDER] Статистика рендеринга:" << std::endl;
        std::cout << "   • Видимые треугольники: " << renderStats.visibleTriangles << std::endl;
        std::cout << "   • Draw calls: " << renderStats.drawCalls << std::endl;
        std::cout << "   • Отброшенные треугольники: " << renderStats.culledTriangles << std::endl;
        std::cout << "   • Frame time: " << renderStats.frameTime << " ms" << std::endl;
        std::cout << "   • FPS: " << renderStats.fps << std::endl;
        std::cout << "   • Memory used: " << (renderStats.memoryUsed / 1024 / 1024) << " MB" << std::endl;
        
        // ✅ GPU информация через renderer
        auto renderer = app_->getRenderer();
        if (renderer) {
            auto gpuInfo = renderer->getGPUInfo();
            std::cout << "🖥️ [GPU] Информация о видеокарте:" << std::endl;
            std::cout << "   • Устройство: " << gpuInfo.deviceName << std::endl;
            std::cout << "   • Драйвер: " << gpuInfo.driverVersion << std::endl;
            std::cout << "   • API версия: " << gpuInfo.apiVersion << std::endl;
            std::cout << "   • Общая память: " << (gpuInfo.totalMemory / 1024 / 1024) << " MB" << std::endl;
            std::cout << "   • Доступная память: " << (gpuInfo.availableMemory / 1024 / 1024) << " MB" << std::endl;
            std::cout << "   • Max texture size: " << gpuInfo.maxTextureSize << "px" << std::endl;
            std::cout << "   • Ray tracing: " << (gpuInfo.supportsRayTracing ? "ДА" : "НЕТ") << std::endl;
        }
    }
    
public:
    void run() {
        auto lastTime = std::chrono::high_resolution_clock::now();
        uint32_t frameCount = 0;
        float totalTime = 0.0f;
        
        constexpr float TARGET_FPS = 60.0f;
        constexpr float FRAME_TIME_MS = 1000.0f / TARGET_FPS;
        
        std::cout << "🎮 [DEBUG] Запуск render loop с новым debug API (Target: " << TARGET_FPS << " FPS)" << std::endl;
        
        while (!app_->should_close()) {
            auto frameStart = std::chrono::high_resolution_clock::now();
            auto currentTime = std::chrono::high_resolution_clock::now();
            deltaTime_ = std::chrono::duration<float>(currentTime - lastTime).count();
            lastTime = currentTime;
            
            // ✅ Enhanced input handling с новыми API
            handleAdvancedInput();
            
            // Обновление и рендеринг
            try {
                app_->update(deltaTime_);
                app_->render();
            } catch (const std::exception& e) {
                std::cerr << "❌ [ERROR] Ошибка в render loop: " << e.what() << std::endl;
                break;
            }
            
            if (app_->should_close()) break;
            
            // ✅ Расширенная статистика FPS с новыми API
            frameCount++;
            totalTime += deltaTime_;
            
            if (totalTime >= 2.0f) {  // Каждые 2 секунды
                fps_ = frameCount / totalTime;
                frameCount = 0;
                totalTime = 0.0f;
                
                // Подробная статистика через новые API
                auto renderStats = app_->getRenderStats();
                auto renderer = app_->getRenderer();
                
                std::cout << "📊 [STATS] FPS: " << static_cast<int>(fps_) 
                         << " | Frame: " << renderStats.frameTime << "ms"
                         << " | Triangles: " << renderStats.visibleTriangles 
                         << " | Memory: " << (renderStats.memoryUsed / 1024 / 1024) << "MB" << std::endl;
                
                if (renderer) {
                    auto detailedStats = renderer->getDetailedStats();
                    std::cout << "    GPU time: " << detailedStats.gpuTime << "ms"
                             << " | Culled: " << detailedStats.culledTriangles
                             << " | Draw calls: " << detailedStats.drawCalls << std::endl;
                }
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
    
private:
    void handleAdvancedInput() {
        // ✅ Enhanced input handling с использованием новых API
        auto input = app_->getInputManager();
        if (!input) return;
        
        // Debug режимы через клавиши 1-4
        if (app_->isKeyPressed(GLFW_KEY_1)) {
            app_->setDebugMode(SpectraForge::App::AppConfig::DebugMode::NORMAL);
        }
        if (app_->isKeyPressed(GLFW_KEY_2)) {
            app_->setDebugMode(SpectraForge::App::AppConfig::DebugMode::SDF_VISUALIZATION);
        }
        if (app_->isKeyPressed(GLFW_KEY_3)) {
            app_->setDebugMode(SpectraForge::App::AppConfig::DebugMode::BARYCENTRIC);
        }
        if (app_->isKeyPressed(GLFW_KEY_4)) {
            app_->setDebugMode(SpectraForge::App::AppConfig::DebugMode::WIREFRAME);
        }
        
        // Функциональные клавиши
        static bool rKeyWasPressed = false;
        bool rKeyPressed = app_->isKeyPressed(GLFW_KEY_R);
        if (rKeyPressed && !rKeyWasPressed) {
            app_->resetCameraForSponza();
            app_->logDebugInfo("Camera reset to Sponza optimal position");
        }
        rKeyWasPressed = rKeyPressed;
        
        static bool fKeyWasPressed = false;
        bool fKeyPressed = app_->isKeyPressed(GLFW_KEY_F);
        if (fKeyPressed && !fKeyWasPressed) {
            static bool wireframe = false;
            wireframe = !wireframe;
            app_->enableWireframe(wireframe);
            app_->logDebugInfo(std::string("Wireframe ") + (wireframe ? "enabled" : "disabled"));
        }
        fKeyWasPressed = fKeyPressed;
        
        // Screenshot на F12
        static bool f12KeyWasPressed = false;
        bool f12KeyPressed = app_->isKeyPressed(GLFW_KEY_F12);
        if (f12KeyPressed && !f12KeyWasPressed) {
            auto renderer = app_->getRenderer();
            if (renderer) {
                auto timestamp = std::chrono::duration_cast<std::chrono::seconds>(
                    std::chrono::system_clock::now().time_since_epoch()).count();
                std::string filename = "screenshot_" + SpectraForge::Core::SAFE_TO_STRING(timestamp) + ".png";
                if (renderer->saveScreenshot(filename)) {
                    app_->logDebugInfo("Screenshot saved: " + filename);
                } else {
                    app_->logDebugInfo("Failed to save screenshot");
                }
            }
        }
        f12KeyWasPressed = f12KeyPressed;
    }
    
public:
    void shutdown() {
        SAFE_PRINT_LINE("🔍 [DEBUG] Завершение advanced debug demo...");
        if (app_) {
            try {
                app_->shutdown();
                app_.reset();
            } catch (const std::exception& e) {
                std::cerr << "❌ [ERROR] Ошибка при завершении: " << e.what() << std::endl;
            }
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
    // Инициализация console system
    SpectraForge::Core::Console::initialize();
    
    DebugDemo demo;
    try {
        std::cout << "🚀 [MAIN] Запуск SpectraForge Debug Demo с Triangle Splatting..." << std::endl;
        
        if (!demo.initialize()) {
            SAFE_ERROR("❌ [ERROR] Ошибка инициализации debug demo");
            return -1;
        }
        
        std::cout << "🎮 [MAIN] Запуск главного цикла с расширенными возможностями..." << std::endl;
        demo.run();
        
        std::cout << "🛑 [MAIN] Завершение..." << std::endl;
        demo.shutdown();
        
    } catch (const std::exception& e) {
        std::cerr << "💥 [CRITICAL] Критическая ошибка: " << e.what() << std::endl;
        std::cerr << "📋 [INFO] Возможные причины:" << std::endl;
        std::cerr << "   • Vulkan drivers не установлены или устарели" << std::endl;
        std::cerr << "   • Недостаточно VRAM для Triangle Splatting" << std::endl;
        std::cerr << "   • Несовместимость GPU с compute shaders" << std::endl;
        std::cerr << "   • Ошибки в Triangle Splatting shaders" << std::endl;
        std::cerr << "   • VMA memory allocation failures" << std::endl;
        return -1;
    } catch (...) {
        std::cerr << "💥 [CRITICAL] Неизвестная критическая ошибка" << std::endl;
        return -1;
    }
    
    std::cout << "✅ [MAIN] SpectraForge Debug Demo завершена успешно!" << std::endl;
    return 0;
}