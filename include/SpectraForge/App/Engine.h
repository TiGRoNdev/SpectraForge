/**
 * @file Engine.h
 * @brief Высокоуровневый фасад приложения поверх Core::EngineCore
 */

#pragma once

#include <memory>
#include <glm/glm.hpp>
#include "SpectraForge/App/Config.h"
#include "SpectraForge/App/IApp.h"
#include "SpectraForge/Core/EngineCore.h"
#include "SpectraForge/Core/Logger.h"
#include "SpectraForge/Core/Window.h"
#include "SpectraForge/Rendering/Common/IRenderer.h"
#include "SpectraForge/Rendering/Common/IResourceManager.h"
#include "SpectraForge/Vulkan/SceneManager.h"

namespace SpectraForge {
namespace App {

/**
 * @brief Высокоуровневый фасад, упрощающий запуск и цикл приложения
 */
class Engine final : public IApp {
  public:
    /**
     * @brief Упрощённый конструктор: движок сам выбирает рендерер и менеджер ресурсов
     *        по умолчанию (HybridFreGS + оптимальный ResourceManager)
     */
    explicit Engine(const AppConfig &config, std::shared_ptr<Core::ILogger> logger);

    explicit Engine(const AppConfig &config,
                    std::shared_ptr<Core::ILogger> logger,
                    std::shared_ptr<Rendering::IRenderer> renderer,
                    std::shared_ptr<Rendering::IResourceManager> resource_manager);

    ~Engine() override;

    bool init() override;
    bool load_scene(const Vulkan::SceneData &data) override;
    void update(float delta_time) override;
    void render() override;
    void shutdown() override;

    /**
     * @brief Признак, что окно запрошено к закрытию (например, нажата кнопка закрытия)
     */
    bool should_close() const { return window_ ? window_->shouldClose() : true; }

  private:
    AppConfig config_;

    std::unique_ptr<Core::Window> window_;
    std::unique_ptr<Core::EngineCore> core_;

    std::shared_ptr<Core::ILogger> logger_;
    std::shared_ptr<Rendering::IRenderer> renderer_;
    std::shared_ptr<Rendering::IResourceManager> resource_manager_;
    std::unique_ptr<Vulkan::SceneManager> scene_manager_;
    
    // Camera state (отодвинуты назад и выше для лучшего обзора Sponza)
    // Оптимизированная позиция камеры для Sponza сцены
    glm::vec3 cameraPos{0.0f, 2.0f, 10.0f};    // Ближе к сцене, на уровне глаз
    glm::vec3 cameraFront{0.0f, 0.0f, -1.0f}; // Смотрим вперёд
    float yaw = -90.0f;                        // Угол поворота по горизонтали
    float pitch = 0.0f;                        // Угол наклона по вертикали
    double lastMouseX = 0.0;
    double lastMouseY = 0.0;
    bool firstMouse = true;
};

}  // namespace App
}  // namespace SpectraForge


