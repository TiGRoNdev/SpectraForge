/**
 * @file Engine.h
 * @brief Высокоуровневый фасад приложения поверх Core::EngineCore (РАСШИРЕННАЯ ВЕРСИЯ)
 */
#pragma once
#include <chrono>
#include <functional>
#include <memory>
#include <string>
#include "SpectraForge/App/Config.h"
#include "SpectraForge/App/IApp.h"
#include "SpectraForge/App/Core/GameLoopManager.h"
#include "SpectraForge/App/Core/WindowManager.h"
#include "SpectraForge/App/Core/InputManager.h"
#include "SpectraForge/App/Core/SceneCoordinator.h"
#include "SpectraForge/Core/EngineCore.h"
#include "SpectraForge/Core/Logger.h"
#include "SpectraForge/Rendering/Common/IRenderer.h"
#include "SpectraForge/Rendering/Common/IResourceManager.h"
#include "SpectraForge/Rendering/Camera3D.h"
#include "SpectraForge/Vulkan/SceneManager.h"

namespace SpectraForge {
namespace App {

// ДОБАВЛЕНО: Структуры для debug информации
struct SceneInfo {
    uint32_t triangleCount = 0;
    uint32_t materialCount = 0;
    struct {
        SpectraForge::Math::Vector3 min{0.0f, 0.0f, 0.0f};
        SpectraForge::Math::Vector3 max{0.0f, 0.0f, 0.0f};
    } bounds;
    bool isLoaded = false;
    std::string scenePath;
};

struct RenderStats {
    uint32_t visibleTriangles = 0;
    uint32_t drawCalls = 0;
    uint32_t culledTriangles = 0;
    float frameTime = 0.0f;
    float fps = 0.0f;
    size_t memoryUsed = 0;
    float gpuTime = 0.0f;
};

// P0.3 REFACTORED: Используем InputState из Core::InputManager
using InputState = Core::InputState;

/**
 * @brief Высокоуровневый фасад, упрощающий запуск и цикл приложения (РАСШИРЕННАЯ ВЕРСИЯ)
 */
class Engine final : public IApp {
public:
    /**
     * @brief Упрощённый конструктор: движок сам выбирает рендерер и менеджер ресурсов
     * по умолчанию (HybridFreGS + оптимальный ResourceManager)
     */
    explicit Engine(const AppConfig &config, std::shared_ptr<SpectraForge::Core::ILogger> logger);
    
    explicit Engine(const AppConfig &config,
                   std::shared_ptr<SpectraForge::Core::ILogger> logger,
                   std::shared_ptr<Rendering::IRenderer> renderer,
                   std::shared_ptr<Rendering::IResourceManager> resource_manager);
    
    ~Engine() override;

    // Основные методы IApp
    bool init() override;
    bool load_scene(const Vulkan::SceneData &data) override;
    void update(float delta_time) override;
    void render() override;
    void shutdown() override;

    /**
     * @brief Признак, что окно запрошено к закрытию
     */
    bool should_close() const { return windowManager_->shouldClose(); }

    // ДОБАВЛЕНО: Debug и diagnostic методы
    
    /**
     * @brief Получить информацию о загруженной сцене
     */
    SceneInfo getSceneInfo() const;
    
    /**
     * @brief Получить камеру для управления
     */
    std::shared_ptr<Rendering::Camera3D> getCamera() const { return sceneCoordinator_->getCamera(); }
    
    /**
     * @brief Включить/выключить внешнее управление камерой
     * @param external Если true, Engine не будет автоматически обновлять позицию камеры
     */
    void setExternalCameraControl(bool external);
    
    /**
     * @brief Получить рендерер
     */
    std::shared_ptr<Rendering::IRenderer> getRenderer() const;
    
    /**
     * @brief Получить менеджер ввода (простая реализация)
     */
    const InputState* getInputManager() const { return inputManager_->getState(); }
    
    /**
     * @brief Получить статистику рендеринга
     */
    RenderStats getRenderStats() const;
    
    /**
     * @brief Установить режим отладки
     */
    void setDebugMode(AppConfig::DebugMode mode);
    
    /**
     * @brief Получить текущий режим отладки
     */
    AppConfig::DebugMode getDebugMode() const;
    
    /**
     * @brief Включить/выключить wireframe режим
     */
    void enableWireframe(bool enable);
    
    /**
     * @brief Установить цвет фона
     */
    void setBackgroundColor(float r, float g, float b, float a = 1.0f);
    
    /**
     * @brief Сбросить камеру в исходное положение для Sponza
     */
    void resetCameraForSponza();
    
    /**
     * @brief Логирование debug информации
     */
    void logDebugInfo(const std::string& message);
    
    /**
     * @brief Проверка нажатия клавиши
     */
    bool isKeyPressed(int key) const;
    // Debug и diagnostic методы (уже объявлены выше)
 
 private:
    // P0.3 REFACTORED: Композиция компонентов вместо God Class
    std::unique_ptr<Core::GameLoopManager> gameLoop_;
    std::unique_ptr<Core::WindowManager> windowManager_;
    std::unique_ptr<Core::InputManager> inputManager_;
    std::unique_ptr<Core::SceneCoordinator> sceneCoordinator_;
    
    // Scene и render state (агрегируется из компонентов)
    mutable SceneInfo sceneInfo_;
    mutable RenderStats renderStats_;
    
    // Debug state
    AppConfig::DebugMode currentDebugMode_ = AppConfig::DebugMode::NORMAL;
    bool wireframeEnabled_ = false;
    bool externalCameraControl_ = false; // Флаг внешнего управления камерой
    
    // Private helper методы
    void updateRenderStats();

    AppConfig config_;
    std::unique_ptr<SpectraForge::Core::EngineCore> core_;
    std::shared_ptr<SpectraForge::Core::ILogger> logger_;
    std::shared_ptr<Rendering::IRenderer> renderer_;
    std::shared_ptr<Rendering::IResourceManager> resource_manager_;
    std::unique_ptr<Vulkan::SceneManager> scene_manager_;
};

} // namespace App
} // namespace SpectraForge