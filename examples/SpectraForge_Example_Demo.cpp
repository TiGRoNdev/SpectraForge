// PSEUDOCODE !!!!!!

#include <chrono>
#include <iostream>
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
        cfg.window_width = 1920;
        cfg.window_height = 1080;
        cfg.window_title = "Pseudo Demo";

        auto logger = std::make_shared<SpectraForge::Core::Logger>("", SpectraForge::Core::LogLevel::INFO_LEVEL);

        app_ = std::make_unique<SpectraForge::App::Engine>(cfg, logger);
        if (!app_->init()) {
            SAFE_ERROR("Ошибка инициализации App::Engine");
            return false;
        }

        // Загрузка сцены через фасад
        SpectraForge::Vulkan::SceneData scene{};
        scene.scenePath = "examples/scenes/sponza/sponza.obj";
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
        while (!app_->should_close()) {
            auto currentTime = std::chrono::high_resolution_clock::now();
            deltaTime_ =
                std::chrono::duration<float, std::chrono::seconds::period>(currentTime - lastTime)
                    .count();
            lastTime = currentTime;

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
            }
        }
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

