/**
 * @file Engine.cpp
 * @brief Simplified implementation of the high level App::Engine facade.
 *
 * This version wires together the refactored subsystems (window/input/scene
 * managers) and the Vulkan renderer so the demos can run inside the new
 * SOLID compliant architecture.
 */

#include "SpectraForge/App/Engine.h"

#include "SpectraForge/App/DISetup.h"
#include "SpectraForge/Core/DependencyInjection/Container.h"
#include "SpectraForge/Core/SafeConsole.h"
#include "SpectraForge/Rendering/HybridFreGSRenderer.h"
#include "SpectraForge/Rendering/RenderPass/TriangleSplattingPass.h"

#include <GLFW/glfw3.h>
#define GLFW_EXPOSE_NATIVE_X11
#include <GLFW/glfw3native.h>

#include <glm/gtc/type_ptr.hpp>

#include <stdexcept>

namespace SpectraForge {
namespace App {

Engine::Engine(const AppConfig& config, std::shared_ptr<SpectraForge::Core::ILogger> logger)
    : Engine(config,
             std::move(logger),
             []() {
                 auto& container = SpectraForge::Core::DI::ServiceLocator::getInstance();
                 if (!container.isRegistered<Rendering::IRenderer>()) {
                     App::DISetup::configure(container);
                 }
                 return container.resolve<Rendering::IRenderer>();
             }(),
             SpectraForge::Core::DI::ServiceLocator::get<Rendering::IWindowBinder>(),
             SpectraForge::Core::DI::ServiceLocator::get<Rendering::IResourceManager>(),
             SpectraForge::Core::DI::ServiceLocator::get<Core::IGameLoopManager>(),
             SpectraForge::Core::DI::ServiceLocator::get<Core::IWindowManager>(),
             SpectraForge::Core::DI::ServiceLocator::get<Core::IInputManager>(),
             SpectraForge::Core::DI::ServiceLocator::get<Core::ISceneCoordinator>(),
             SpectraForge::Core::DI::ServiceLocator::get<Vulkan::ISceneManager>(),
             SpectraForge::Core::DI::ServiceLocator::get<SpectraForge::Core::IEngineCore>()) {}

Engine::Engine(const AppConfig& config,
               std::shared_ptr<SpectraForge::Core::ILogger> logger,
               std::shared_ptr<Rendering::IRenderer> renderer,
               std::shared_ptr<Rendering::IWindowBinder> windowBinder,
               std::shared_ptr<Rendering::IResourceManager> resourceManager,
               std::shared_ptr<Core::IGameLoopManager> gameLoop,
               std::shared_ptr<Core::IWindowManager> windowManager,
               std::shared_ptr<Core::IInputManager> inputManager,
               std::shared_ptr<Core::ISceneCoordinator> sceneCoordinator,
               std::shared_ptr<Vulkan::ISceneManager> sceneManager,
               std::shared_ptr<SpectraForge::Core::IEngineCore> engineCore)
    : gameLoop_(std::move(gameLoop)),
      windowManager_(std::move(windowManager)),
      inputManager_(std::move(inputManager)),
      sceneCoordinator_(std::move(sceneCoordinator)),
      config_(config),
      logger_(std::move(logger)),
      renderer_(std::move(renderer)),
      windowBinder_(std::move(windowBinder)),
      resource_manager_(std::move(resourceManager)),
      scene_manager_(std::move(sceneManager)),
      core_(std::move(engineCore)) {
    if (!logger_ || !renderer_ || !resource_manager_ || !windowBinder_ || !gameLoop_ || !windowManager_
        || !inputManager_ || !sceneCoordinator_ || !scene_manager_ || !core_) {
        throw std::invalid_argument("App::Engine: dependencies must not be null");
    }
}

Engine::~Engine() { shutdown(); }

bool Engine::init() {
    if (!windowManager_->initializeSystem()) {
        SAFE_ERROR("[App::Engine] Failed to initialise window system");
        return false;
    }

    if (!windowManager_->createWindow(config_.window_title, config_.window_width, config_.window_height)) {
        SAFE_ERROR("[App::Engine] Failed to create window");
        return false;
    }

    auto* window = windowManager_->getWindow();
    if (!window) {
        SAFE_ERROR("[App::Engine] Window handle is null");
        return false;
    }

    GLFWwindow* nativeWindow = window->getNativeWindow();
    if (!nativeWindow) {
        SAFE_ERROR("[App::Engine] Native GLFW window is null");
        return false;
    }

    inputManager_->setupCallbacks(nativeWindow);
    if (!externalCameraControl_) {
        glfwSetInputMode(nativeWindow, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    }

    if (!core_->initialize()) {
        SAFE_ERROR("[App::Engine] EngineCore initialisation failed");
        return false;
    }

    // Attach the OS window to the Vulkan renderer when possible.
#ifdef GLFW_EXPOSE_NATIVE_X11
    if (windowBinder_) {
        Display* display = glfwGetX11Display();
        Window x11Window = glfwGetX11Window(nativeWindow);
        if (display && x11Window) {
            if (!windowBinder_->attachWindow(display,
                                             reinterpret_cast<void*>(x11Window),
                                             static_cast<uint32_t>(config_.window_width),
                                             static_cast<uint32_t>(config_.window_height))) {
                SAFE_ERROR("[App::Engine] Failed to attach window to HybridFreGSRenderer");
                return false;
            }
        }
    }
#endif

    // Ensure the scene coordinator owns a camera.
    if (!sceneCoordinator_->getCamera()) {
        auto camera = std::make_shared<Rendering::Camera3D>();
        float aspect = static_cast<float>(config_.window_width) / static_cast<float>(config_.window_height);
        camera->setPerspective(60.0f, aspect, 0.1f, 1000.0f);
        sceneCoordinator_->setCamera(camera);
    }

    if (!scene_manager_->init()) {
        SAFE_ERROR("[App::Engine] SceneManager initialisation failed");
        return false;
    }

    updateRenderStats();
    sceneInfo_ = sceneCoordinator_->getSceneInfo();
    return true;
}

bool Engine::load_scene(const Vulkan::SceneData& data) {
    if (!scene_manager_->loadScene(data)) {
        SAFE_ERROR("[App::Engine] SceneManager failed to load scene");
        return false;
    }

    sceneCoordinator_->loadScene(data);
    sceneInfo_ = sceneCoordinator_->getSceneInfo();
    return true;
}

void Engine::update(float deltaTime) {
    lastFrameDelta_ = deltaTime;

    windowManager_->pollEvents();
    inputManager_->update();
    sceneCoordinator_->updateCamera(deltaTime, inputManager_->getState(), externalCameraControl_);
    scene_manager_->updateDynamics(deltaTime);
    sceneInfo_ = sceneCoordinator_->getSceneInfo();
}

void Engine::render() {
    if (!renderer_) {
        return;
    }

    Rendering::FrameData frame{};
    frame.renderTargetSize.width = config_.window_width;
    frame.renderTargetSize.height = config_.window_height;
    frame.timing.deltaTime = lastFrameDelta_;
    frame.timing.frameNumber = frameCounter_++;

    if (auto camera = sceneCoordinator_->getCamera()) {
        const auto& pos = camera->getPosition();
        const auto& target = camera->getTarget();
        const auto up = camera->getUp();

        frame.camera.position = pos;
        frame.camera.target = target;
        frame.camera.up = up;
        frame.camera.fov = camera->getFieldOfView();
        frame.camera.nearPlane = camera->getNearPlane();
        frame.camera.farPlane = camera->getFarPlane();

        auto view = camera->getViewMatrix();
        auto proj = camera->getProjectionMatrix();
        frame.camera.viewMatrix = glm::make_mat4(view.data());
        frame.camera.projectionMatrix = glm::make_mat4(proj.data());
    }

    renderer_->beginFrame();
    renderer_->renderFrame(frame);
    renderer_->endFrame();

    windowManager_->swapBuffers();
    updateRenderStats();
}

void Engine::shutdown() {
    if (scene_manager_) {
        scene_manager_->shutdown();
    }

    if (renderer_) {
        renderer_->shutdown();
    }

    if (core_) {
        core_->shutdown();
        core_.reset();
    }

    if (windowManager_) {
        windowManager_->shutdown();
    }
}

SceneInfo Engine::getSceneInfo() const {
    return sceneCoordinator_->getSceneInfo();
}

std::shared_ptr<Rendering::IRenderer> Engine::getRenderer() const {
    return renderer_;
}

RenderStats Engine::getRenderStats() const {
    return renderStats_;
}

void Engine::setExternalCameraControl(bool enabled) {
    externalCameraControl_ = enabled;
}

void Engine::setDebugMode(AppConfig::DebugMode mode) {
    currentDebugMode_ = mode;
    if (renderer_) {
        renderer_->setDebugMode(static_cast<int>(mode));
    }
}

AppConfig::DebugMode Engine::getDebugMode() const {
    return currentDebugMode_;
}

void Engine::enableWireframe(bool enable) {
    wireframeEnabled_ = enable;
    if (renderer_) {
        renderer_->enableWireframe(enable);
    }
}

void Engine::setBackgroundColor(float r, float g, float b, float a) {
    if (renderer_) {
        renderer_->setBackgroundColor(r, g, b, a);
    }
}

void Engine::resetCameraForSponza() {
    sceneCoordinator_->resetCameraForSponza();
}

void Engine::logDebugInfo(const std::string& message) {
    if (logger_) {
        logger_->logInfo(message);
    }
}

bool Engine::isKeyPressed(int key) const {
    return inputManager_ ? inputManager_->isKeyPressed(key) : false;
}

void Engine::updateRenderStats() {
    if (!renderer_) {
        renderStats_ = {};
        return;
    }

    const auto stats = renderer_->getStats();
    renderStats_.frameTime = stats.frameTime;
    renderStats_.fps = stats.fps;
    renderStats_.drawCalls = stats.drawCalls;
    renderStats_.culledTriangles = stats.primitives;
    renderStats_.visibleTriangles = stats.primitives;
    renderStats_.memoryUsed = stats.memoryUsed;
}

} // namespace App
} // namespace SpectraForge
