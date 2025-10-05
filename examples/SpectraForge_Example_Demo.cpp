#include <chrono>
#include <iostream>
#include <thread>  // для std::this_thread::sleep_for
#include "SpectraForge/App/Engine.h"
#include "SpectraForge/App/Config.h"
#include "SpectraForge/Core/Console.h"
#include "SpectraForge/Core/Logger.h"
#include "SpectraForge/Core/SafeConsole.h"
#include "SpectraForge/Rendering/Common/IRenderer.h"
#include "SpectraForge/Rendering/Common/IResourceManager.h"
#include "SpectraForge/Vulkan/SceneManager.h"

using namespace SpectraForge;

class PseudoDemo {
  public:
    PseudoDemo() = default;
    ~PseudoDemo() = default;

    bool initialize() {
        SAFE_PRINT_LINE("=== Pseudo Demo ===");
        SAFE_PRINT_LINE("");

        // Конфигурация и создание фасада
        SpectraForge::App::AppConfig cfg;
        // ВРЕМЕННО: Пониженное разрешение для стабилизации производительности
        cfg.window_width = 960;
        cfg.window_height = 540;
        cfg.window_title = "Pseudo Demo (960x540)";

        std::cout << "[Demo] 📺 Разрешение: " << cfg.window_width << "x" << cfg.window_height
                  << " (временно понижено для стабилизации)" << std::endl;

        auto logger = std::make_shared<SpectraForge::Core::Logger>("", SpectraForge::Core::LogLevel::INFO_LEVEL);

        app_ = std::make_unique<SpectraForge::App::Engine>(cfg, logger);
        if (!app_->init()) {
            SAFE_ERROR("Ошибка инициализации App::Engine");
            return false;
        }

        // Загрузка сцены через фасад
        SpectraForge::Vulkan::SceneData scene{};
        scene.scenePath = "examples/scenes/sponza/sponza.obj";

        // ОПТИМИЗАЦИЯ: Уменьшаем плотность треугольников для достижения целевого FPS
        // Sponza: 40,211 оригинальных треугольников
        // 
        // Benchmark results (см. TRIANGLE_STEP_BENCHMARK.md):
        // step=1   → 40,211 треугольников → 0.22 FPS  (baseline, неприемлемо)
        // step=100 →    403 треугольников → 18 FPS    (✅ ОПТИМАЛЬНЫЙ БАЛАНС)
        // step=200 →    202 треугольников → 25 FPS    (хорошая производительность)
        // step=500 →     81 треугольников → 32 FPS    (максимальная производительность)
        //
        // Выбрано: step=100 для лучшего баланса качества и производительности
        scene.triangleStep = 100;

        std::cout << "[Demo] 🔺 Загрузка сцены с уменьшенной плотностью треугольников (step=" << scene.triangleStep << ")" << std::endl;

        app_->load_scene(scene);

        SAFE_PRINT_LINE("");
        SAFE_PRINT_LINE("✅ Инициализация завершена успешно!");
        SAFE_PRINT_LINE("");
        SAFE_PRINT_LINE("Управление:");
        SAFE_PRINT_LINE("  WASD - движение камеры");
        SAFE_PRINT_LINE("  Мышь - поворот камеры");
        SAFE_PRINT_LINE("  Scroll - изменение скорости");
        SAFE_PRINT_LINE("  ESC - выход");
        SAFE_PRINT_LINE("");

        return true;
    }

    void run() {
        auto lastTime = std::chrono::high_resolution_clock::now();
        uint32_t frameCount = 0;
        float totalTime = 0.0f;
        
        // КРИТИЧНО: Ограничение FPS для предотвращения 100% CPU и зависания системы
        constexpr float TARGET_FPS = 60.0f;
        constexpr float FRAME_TIME_MS = 1000.0f / TARGET_FPS;  // 16.67ms для 60 FPS
        
        std::cout << "[Demo] 🎮 Запуск с ограничением FPS: " << TARGET_FPS << std::endl;
        
        while (!app_->should_close()) {
            auto frameStart = std::chrono::high_resolution_clock::now();
            
            auto currentTime = std::chrono::high_resolution_clock::now();
            deltaTime_ =
                std::chrono::duration<float, std::chrono::seconds::period>(currentTime - lastTime)
                    .count();
            lastTime = currentTime;

            // Обновление и рендеринг
            app_->update(deltaTime_);
            app_->render();

            // Выход по запросу закрытия окна
            if (app_->should_close()) break;

            // Статистика FPS
            frameCount++;
            totalTime += deltaTime_;

            if (totalTime >= 1.0f) {
                fps_ = frameCount / totalTime;
                frameCount = 0;
                totalTime = 0.0f;
                std::cout << "[Demo] 📊 FPS: " << static_cast<int>(fps_) << std::endl;
            }
            
            // КРИТИЧНО: Ограничение FPS и yield CPU
            auto frameEnd = std::chrono::high_resolution_clock::now();
            auto frameDuration = std::chrono::duration<float, std::milli>(frameEnd - frameStart).count();
            float remainingTime = FRAME_TIME_MS - frameDuration;
            
            if (remainingTime > 0.0f) {
                // Sleep до следующего кадра, освобождая CPU для системы
                std::this_thread::sleep_for(
                    std::chrono::microseconds(static_cast<int>(remainingTime * 1000.0f))
                );
            } else if (frameDuration > FRAME_TIME_MS * 2.0f) {
                // Предупреждение если кадр слишком долгий
                static int warnCount = 0;
                if (warnCount++ < 5) {
                    std::cerr << "[Demo] ⚠️  Долгий кадр: " << frameDuration << "ms (цель: " 
                              << FRAME_TIME_MS << "ms)" << std::endl;
                }
            }
        }
        
        std::cout << "[Demo] 🏁 Render loop завершён" << std::endl;
    }

    void shutdown() {
        SAFE_PRINT_LINE("Завершение работы...");

        if (app_) {
            app_->shutdown();
            app_.reset();
        }

        SAFE_PRINT_LINE("✅ Завершено успешно!");
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

    PseudoDemo demo;

    try {
        if (!demo.initialize()) {
            SAFE_ERROR("Ошибка инициализации демо");
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

