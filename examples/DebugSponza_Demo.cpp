#include <iostream>
#include <memory>
#include <chrono>
#include <thread>
#include <fstream>
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
        SAFE_PRINT_LINE("=== DEBUG DEMO FOR SPONZA RENDERING (ИСПРАВЛЕННАЯ ВЕРСИЯ) ===");
        SAFE_PRINT_LINE("");
        
        // Конфигурация с использованием ТОЛЬКО существующих полей
        SpectraForge::App::AppConfig cfg;
        cfg.window_width = 960;
        cfg.window_height = 540;
        cfg.window_title = "Debug Demo - Sponza Rendering";
        
        // ИСПРАВЛЕНО: Используем ТОЛЬКО существующие поля
        cfg.debug = true;    // Существующее поле debug
        cfg.vsync = false;   // Отключаем для стабильности
        
        std::cout << "🔍 [DEBUG] Разрешение: " << cfg.window_width << "x" << cfg.window_height << std::endl;
        std::cout << "🔍 [DEBUG] Debug mode: " << (cfg.debug ? "ENABLED" : "DISABLED") << std::endl;
        
        auto logger = std::make_shared<SpectraForge::Core::Logger>("", SpectraForge::Core::LogLevel::DEBUG_LEVEL);
        app_ = std::make_unique<SpectraForge::App::Engine>(cfg, logger);
        
        if (!app_->init()) {
            SAFE_ERROR("Ошибка инициализации App::Engine");
            return false;
        }
        
        std::cout << "✅ [DEBUG] Engine инициализирован" << std::endl;
        
        // Загрузка сцены с использованием ТОЛЬКО существующих полей
        SpectraForge::Vulkan::SceneData scene{};
        scene.scenePath = "examples/scenes/sponza/sponza.obj";
        
        // ДИАГНОСТИКА: Проверка существования файла
        std::ifstream file(scene.scenePath);
        if (!file.good()) {
            std::cerr << "❌ [ERROR] Файл сцены не найден: " << scene.scenePath << std::endl;
            std::cerr << "📝 [INFO] Попробуйте проверить пути:" << std::endl;
            std::cerr << "   • examples/scenes/sponza/sponza.obj" << std::endl;
            std::cerr << "   • assets/sponza/sponza.obj" << std::endl;
            std::cerr << "   • models/sponza/sponza.obj" << std::endl;
            
            // Попробуем альтернативные пути
            std::vector<std::string> alternativePaths = {
                "assets/sponza/sponza.obj",
                "models/sponza/sponza.obj",
                "data/sponza/sponza.obj",
                "../examples/scenes/sponza/sponza.obj",
                "./sponza.obj"
            };
            
            bool found = false;
            for (const auto& path : alternativePaths) {
                std::ifstream altFile(path);
                if (altFile.good()) {
                    scene.scenePath = path;
                    std::cout << "✅ [DEBUG] Найден альтернативный путь: " << path << std::endl;
                    found = true;
                    break;
                }
            }
            
            if (!found) {
                std::cerr << "❌ [ERROR] Файл Sponza не найден ни по одному из путей" << std::endl;
                return false;
            }
        } else {
            file.close();
            std::cout << "✅ [DEBUG] Файл сцены найден: " << scene.scenePath << std::endl;
        }
        
        // Для отладки: уменьшаем количество треугольников
        scene.triangleStep = 100;
        
        std::cout << "🔺 [DEBUG] Загрузка сцены с triangle step = " << scene.triangleStep << std::endl;
        
        // Загрузка с обработкой ошибок
        bool sceneLoaded = false;
        try {
            sceneLoaded = app_->load_scene(scene);
        } catch (const std::exception& e) {
            std::cerr << "❌ [ERROR] Исключение при загрузке сцены: " << e.what() << std::endl;
            return false;
        }
        
        if (!sceneLoaded) {
            std::cerr << "❌ [ERROR] Не удалось загрузить сцену (load_scene вернул false)" << std::endl;
            std::cerr << "📝 [INFO] Возможные причины:" << std::endl;
            std::cerr << "   • Файл поврежден или неподдерживаемый формат" << std::endl;
            std::cerr << "   • Недостаточно памяти" << std::endl;
            std::cerr << "   • Ошибка в OBJ parser" << std::endl;
            return false;
        }
        
        std::cout << "✅ [DEBUG] Сцена загружена успешно" << std::endl;
        
        // ПРОСТАЯ ДИАГНОСТИКА через логи (без несуществующих методов)
        std::cout << "📊 [DEBUG] Попытка получения базовой информации..." << std::endl;
        
        // ИСПРАВЛЕНО: НЕ используем несуществующие методы
        // Базовая информация через существующие каналы
        std::cout << "   • Файл: " << scene.scenePath << std::endl;
        std::cout << "   • Triangle step: " << scene.triangleStep << std::endl;
        std::cout << "   • Debug mode: " << (cfg.debug ? "включен" : "отключен") << std::endl;
        
        SAFE_PRINT_LINE("");
        SAFE_PRINT_LINE("✅ Инициализация завершена!");
        SAFE_PRINT_LINE("");
        SAFE_PRINT_LINE("🔍 Простая диагностика:");
        SAFE_PRINT_LINE("   • Файл сцены загружен");
        SAFE_PRINT_LINE("   • Engine инициализирован");
        SAFE_PRINT_LINE("   • Готов к рендерингу");
        SAFE_PRINT_LINE("");
        SAFE_PRINT_LINE("📋 Что проверить:");
        SAFE_PRINT_LINE("   1. Если видите монотонный цвет - возможно камера неправильно позиционирована");
        SAFE_PRINT_LINE("   2. Если экран черный - возможно проблема с lighting");
        SAFE_PRINT_LINE("   3. Если синий/серый - возможно background color перезаписывает геометрию");
        SAFE_PRINT_LINE("");
        SAFE_PRINT_LINE("Управление:");
        SAFE_PRINT_LINE("   WASD - движение камеры (если реализовано в Engine)");
        SAFE_PRINT_LINE("   Мышь - поворот камеры (если реализовано)");
        SAFE_PRINT_LINE("   ESC - выход");
        SAFE_PRINT_LINE("");
        
        return true;
    }
    
    void run() {
        auto lastTime = std::chrono::high_resolution_clock::now();
        uint32_t frameCount = 0;
        float totalTime = 0.0f;
        
        constexpr float TARGET_FPS = 30.0f;  // Понижаем для стабильности
        constexpr float FRAME_TIME_MS = 1000.0f / TARGET_FPS;
        
        std::cout << "🎮 [DEBUG] Запуск render loop (FPS target: " << TARGET_FPS << ")" << std::endl;
        
        while (!app_->should_close()) {
            auto frameStart = std::chrono::high_resolution_clock::now();
            auto currentTime = std::chrono::high_resolution_clock::now();
            deltaTime_ = std::chrono::duration<float>(currentTime - lastTime).count();
            lastTime = currentTime;
            
            // ПРОСТАЯ input обработка без несуществующих методов
            handleSimpleInput();
            
            // Обновление и рендеринг
            try {
                app_->update(deltaTime_);
                app_->render();
            } catch (const std::exception& e) {
                std::cerr << "❌ [ERROR] Ошибка в render loop: " << e.what() << std::endl;
                break;
            }
            
            if (app_->should_close()) break;
            
            // Базовая статистика FPS без несуществующих методов
            frameCount++;
            totalTime += deltaTime_;
            
            if (totalTime >= 2.0f) {  // Каждые 2 секунды
                fps_ = frameCount / totalTime;
                frameCount = 0;
                totalTime = 0.0f;
                
                std::cout << "📊 [DEBUG] FPS: " << static_cast<int>(fps_) 
                         << " | Frame time: " << (1000.0f / fps_) << "ms" << std::endl;
                
                // ПРОСТАЯ диагностика без несуществующих методов
                std::cout << "   • Delta time: " << deltaTime_ << "s" << std::endl;
                std::cout << "   • Should close: " << (app_->should_close() ? "true" : "false") << std::endl;
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
    
    void handleSimpleInput() {
        // ПРОСТАЯ обработка input без несуществующих API
        // Все input processing происходит внутри Engine через update()
        
        // Можно добавить простые проверки, если Engine предоставляет такую возможность
        // Но пока используем только существующие методы
        
        // Примечание: Real input handling должен быть в Engine::update()
        // где есть доступ к GLFW window callbacks
    }
    
    void shutdown() {
        SAFE_PRINT_LINE("🔍 [DEBUG] Завершение debug demo...");
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
        std::cout << "🚀 [MAIN] Запуск debug demo..." << std::endl;
        
        if (!demo.initialize()) {
            SAFE_ERROR("❌ [ERROR] Ошибка инициализации debug demo");
            return -1;
        }
        
        std::cout << "🎮 [MAIN] Запуск главного цикла..." << std::endl;
        demo.run();
        
        std::cout << "🛑 [MAIN] Завершение..." << std::endl;
        demo.shutdown();
        
    } catch (const std::exception& e) {
        std::cerr << "💥 [CRITICAL] Критическая ошибка: " << e.what() << std::endl;
        std::cerr << "📋 [INFO] Возможные причины:" << std::endl;
        std::cerr << "   • Vulkan/OpenGL драйверы не установлены" << std::endl;
        std::cerr << "   • Недостаточно памяти GPU" << std::endl;
        std::cerr << "   • Несовместимость GPU с Triangle Splatting" << std::endl;
        std::cerr << "   • Ошибки в shaders compilation" << std::endl;
        return -1;
    } catch (...) {
        std::cerr << "💥 [CRITICAL] Неизвестная критическая ошибка" << std::endl;
        return -1;
    }
    
    std::cout << "✅ [MAIN] Программа завершена успешно" << std::endl;
    return 0;
}