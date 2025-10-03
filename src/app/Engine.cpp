/**
 * @file Engine.cpp
 * @brief Реализация фасада App::Engine
 */

#include "SpectraForge/App/Engine.h"
#include <stdexcept>
#include "SpectraForge/Core/SafeConsole.h"
#include "SpectraForge/Rendering/RendererFactory.h"
#include "SpectraForge/Rendering/Common/IResourceManager.h"
#include "SpectraForge/Rendering/HybridFreGSRenderer.h"
#include "SpectraForge/Core/Logger.h"

namespace {
// Простой дефолтный ResourceManager-адаптер до полноценной интеграции VMA-слоя в IResourceManager
class NullResourceManager final : public SpectraForge::Rendering::IResourceManager {
  public:
    bool initialize() override { return true; }
    void shutdown() override {}
    SpectraForge::Rendering::BufferHandle createBuffer(const SpectraForge::Rendering::BufferDesc&) override { return SpectraForge::Rendering::INVALID_HANDLE; }
    void updateBuffer(SpectraForge::Rendering::BufferHandle, const void*, size_t, size_t) override {}
    void readBuffer(SpectraForge::Rendering::BufferHandle, void*, size_t, size_t) override {}
    SpectraForge::Rendering::TextureHandle createTexture(const SpectraForge::Rendering::TextureDesc&) override { return SpectraForge::Rendering::INVALID_HANDLE; }
    void updateTexture(SpectraForge::Rendering::TextureHandle, const void*, uint32_t, uint32_t) override {}
    SpectraForge::Rendering::ShaderHandle createShader(const std::string&, SpectraForge::Rendering::ShaderType) override { return SpectraForge::Rendering::INVALID_HANDLE; }
    SpectraForge::Rendering::ShaderHandle createShaderFromFile(const std::string&, SpectraForge::Rendering::ShaderType) override { return SpectraForge::Rendering::INVALID_HANDLE; }
    void releaseResource(SpectraForge::Rendering::ResourceHandle) override {}
    void releaseAllResources() override {}
    bool isValid(SpectraForge::Rendering::ResourceHandle) const override { return false; }
    size_t getResourceSize(SpectraForge::Rendering::ResourceHandle) const override { return 0; }
    SpectraForge::Rendering::MemoryStats getMemoryStats() const override { return {}; }
    void waitForCompletion() override {}
    void flush() override {}
};
}

using namespace SpectraForge;
using namespace SpectraForge::App;

Engine::Engine(const AppConfig &config, std::shared_ptr<Core::ILogger> logger)
    : config_(config), logger_(std::move(logger))
{
    // Дефолт: Жёстко выбираем HybridFreGSRenderer для соответствия требованиям
    std::shared_ptr<Rendering::IRenderer> rendererShared =
        std::make_shared<Rendering::HybridFreGSRenderer>();
    std::shared_ptr<Rendering::IResourceManager> rm = std::make_shared<NullResourceManager>();

    renderer_ = std::move(rendererShared);
    resource_manager_ = std::move(rm);

    if (!logger_ || !renderer_ || !resource_manager_) {
        throw std::invalid_argument("App::Engine: зависимости не могут быть nullptr");
    }
}

Engine::Engine(const AppConfig &config,
               std::shared_ptr<Core::ILogger> logger,
               std::shared_ptr<Rendering::IRenderer> renderer,
               std::shared_ptr<Rendering::IResourceManager> resource_manager)
    : config_(config),
      logger_(std::move(logger)),
      renderer_(std::move(renderer)),
      resource_manager_(std::move(resource_manager)) {
    if (!logger_ || !renderer_ || !resource_manager_) {
        throw std::invalid_argument("App::Engine: зависимости не могут быть nullptr");
    }
}

Engine::~Engine() { shutdown(); }

bool Engine::init() {
    // Инициализируем GLFW ДО всего остального
    logger_->logInfo("[App::Engine] Инициализация GLFW системы...");
    if (!Core::Window::initializeSystem()) {
        SAFE_ERROR("[App::Engine] Ошибка инициализации GLFW");
        return false;
    }
    logger_->logInfo("[App::Engine] GLFW инициализирована успешно");

    // Окно - создаем ДО инициализации EngineCore (Vulkan нужно окно для получения расширений WSI)
    logger_->logInfo("[App::Engine] Создание окна...");
    Core::Window::Config wc;
    wc.title = config_.window_title;
    wc.width = config_.window_width;
    wc.height = config_.window_height;
    wc.vSync = config_.vsync;
    // Подсказка для окна: использовать Vulkan presentation, без GL контекста
    setenv("SPECTRAFORGE_PRESENT_VULKAN", "1", 1);
    window_ = std::make_unique<Core::Window>(wc);
    if (!window_->initialize()) {
        SAFE_ERROR("[App::Engine] Ошибка инициализации окна (возможно, Vulkan не поддерживается GLFW)");
        return false;
    }
    logger_->logInfo("[App::Engine] Окно создано успешно");

    // !!!ВАЖНО!!! glfwGetRequiredInstanceExtensions работает ТОЛЬКО после создания окна
    
    // Ядро - инициализируем ПОСЛЕ создания окна
    logger_->logInfo("[App::Engine] Инициализация EngineCore...");
    core_ = std::make_unique<Core::EngineCore>(renderer_, resource_manager_, logger_);
    if (!core_->initialize()) {
        SAFE_ERROR("[App::Engine] Ошибка инициализации EngineCore");
        return false;
    }
    logger_->logInfo("[App::Engine] EngineCore инициализирован успешно");

    // Сцена
    scene_manager_ = std::make_unique<Vulkan::SceneManager>();
    if (!scene_manager_->init()) {
        SAFE_ERROR("[App::Engine] Ошибка инициализации SceneManager");
        return false;
    }

    logger_->logInfo("App::Engine инициализирован");
    return true;
}

bool Engine::load_scene(const Vulkan::SceneData &data) {
    if (!scene_manager_) return false;
    return scene_manager_->loadScene(data);
}

void Engine::update(float delta_time) {
    if (core_) {
        // Пока только обновляем подсистемы через ядро
        // В текущем ядре updateSubsystems вызывается внутри run(),
        // поэтому здесь достаточно подготовить данные. Заглушка.
        (void)delta_time;
    }
    if (window_) {
        window_->pollEvents();
    }
}

void Engine::render() {
    if (!renderer_ || !renderer_->isInitialized()) return;
    renderer_->beginFrame();
    // Формируем минимальные данные кадра
    Rendering::FrameData frame{};
    if (window_) {
        auto fb = window_->getFramebufferSize();
        frame.renderTargetSize.width = static_cast<int>(fb.x);
        frame.renderTargetSize.height = static_cast<int>(fb.y);
    }
    renderer_->renderFrame(frame);
    renderer_->endFrame();
    // Vulkan presentation выполняется внутри рендерера; swapBuffers не нужен
}

void Engine::shutdown() {
    if (scene_manager_) {
        scene_manager_->shutdown();
        scene_manager_.reset();
    }
    if (core_) {
        core_->shutdown();
        core_.reset();
    }
    if (window_) {
        window_->close();
        window_.reset();
    }
}


