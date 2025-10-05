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
#include "SpectraForge/Rendering/Camera3D.h"
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
    
    // Camera для рендеринга (Camera3D объект)
    std::unique_ptr<Rendering::Camera3D> renderCamera_;
    
    // Camera state - ВНУТРИ Sponza для корректного рендеринга
    // Sponza AABB: X=[-17..17], Y=[-0.9..15.6], Z=[-7.8..7.8]
    // Треугольники находятся в X=10.9, поэтому камера смотрит ВПРАВО (+X)
    glm::vec3 cameraPos{0.0f, 3.0f, 0.0f};     // ✅ ВНУТРИ атриума на уровне глаз
    glm::vec3 cameraFront{0.99f, -0.10f, 0.0f};  // Смотрим ВПРАВО (+X) и НЕМНОГО ВНИЗ
    float yaw = 0.0f;                          // ✅ ВПРАВО (+X direction)
    float pitch = -5.0f;                       // НЕМНОГО ВНИЗ
    double lastMouseX = 0.0;
    double lastMouseY = 0.0;
    bool firstMouse = true;
};

}  // namespace App
}  // namespace SpectraForge


