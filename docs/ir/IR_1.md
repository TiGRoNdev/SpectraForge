IR_1: SpectraForge High-Level API IR (local analysis)
version: 0.1.0
date: 2025-10-03
source_basis:
  - codebase_scan: true
  - repo_architecture_docs:
      - docs/architecture/ARCHITECTURE.md
      - docs/modules/rendering.md
      - docs/modules/upscaling.md
  - key_headers:
      - include/SpectraForge/Core/EngineCore.h
      - include/SpectraForge/Core/Console.h
      - include/SpectraForge/Core/SafeConsole.h
      - include/SpectraForge/Vulkan/VulkanRenderer.h
      - include/SpectraForge/Vulkan/SceneManager.h
      - include/SpectraForge/Vulkan/HardwareDetector.h
  - key_sources:
      - src/core/EngineCore.cpp
      - src/core/Console.cpp
      - src/rendering/vulkan/VulkanRenderer.cpp
      - src/rendering/vulkan/SceneManager.cpp
      - src/rendering/vulkan/HardwareDetector.cpp

goals:
  - Define stable App-level API (facade) reusing existing subsystems
  - Keep SOLID, DIP; avoid breaking current modules; thin adapters only
  - Provide minimal, ergonomic surface for demos like PSEUDO example

app_api_surface:
  namespace: SpectraForge::App
  types:
    - name: Engine
      role: Public facade over Core::EngineCore and runtime subsystems
      lifecycle:
        - Engine(const AppConfig&)
        - bool initialize()
        - void shutdown()
        - void run(const IApp& app)  # optional framework loop
        - void stop()
      real_time:
        - void update(float dt)
        - void render()
      windowing:
        - IWindow* get_window() const  # non-owning
        - void set_window_title(const std::string&)
      scene:
        - bool load_scene(const std::string& scene_path)
        - bool is_scene_loaded() const
      info:
        - EngineInfo info() const
      mapping:
        - forwards to Core::EngineCore::{initialize,run,stop,shutdown}
        - delegates to Vulkan::SceneManager for scene ops
        - delegates to Renderer (VulkanRenderer) for render()
        - uses Console/SafeConsole for logging

    - name: IApp
      role: Thin interface for user app callbacks
      methods:
        - bool on_init(Engine&)
        - void on_update(Engine&, float dt)
        - void on_render(Engine&)
        - void on_shutdown(Engine&)

    - name: AppConfig
      role: Unified startup configuration
      fields:
        - window: WindowConfig
        - renderer: RendererConfig
        - upscaling: UpscalerConfig
        - resources: ResourceConfig

  demo_mapping:
    PSEUDO_API_to_real:
      - Engine::initialize(w,h,title) -> AppConfig{window} + Engine::initialize()
      - Engine::loadScene(path) -> SceneManager::loadScene(SceneData)
      - Engine::update(dt) -> EngineCore::updateSubsystems(dt)
      - Engine::render() -> EngineCore::renderFrame() -> VulkanRenderer pipeline
      - Engine::getWindow() -> Window facade over GLFW/Vulkan swapchain
      - Engine::updateWindowTitle(s) -> Console::setTitle(s)

core_layer:
  keep:
    - Core::EngineCore (orchestrator, DI of subsystems)
    - Core::Console, Core::SAFE_* macros
    - Interfaces (IInitializable, IConfigurable, IEventHandler)
  extend:
    - Add thin App::Engine facade -> holds shared_ptr<Core::EngineCore>
    - Provide Core::IWindow and implementation binding to platform layer

rendering_layer:
  interfaces:
    - Rendering::IRenderer  # already exists
    - Rendering::IResourceManager
  impl:
    - Vulkan::VulkanRenderer as primary implementation
    - Stages: FlashGSSplatter (CUDA), OptiXRayTracer, Denoiser, Upscaler
  frame_flow:
    - rasterizePrimary(Gaussians) -> rayTraceSecondary(PrimaryImage) -> denoiseAI(RawEffects)
      -> upscale(DenoisedImage, target) -> presentFinalImage()
  selection:
    - Vulkan::HardwareDetector selects features and Upscaler path

scene_layer:
  api:
    - ISceneManager { init, load_scene(SceneData), getGaussians(), updateDynamics(dt) }
  impl:
    - Vulkan::SceneManager (current)
  data:
    - SceneData{scenePath, meshPaths[], texturePaths[]}
    - Gaussians, MultiViewImages, DynamicElements

hardware_layer:
  api:
    - Vulkan::HardwareDetector { detectVendor, supportsRayTracing, selectUpscalerPath, supportsCUDA, supportsOptiX }
  upscaling:
    - Upscaling::IUpscaler, UpscalerFactory (auto-select DLSS/FSR2/NONE)

window_input_layer:
  api:
    - Core::IWindow { should_close(), set_title(), size(), poll_events() }
    - Input::IInput (future), mapping to GLFW
  note:
    - Reuse existing GLFW integration hidden behind IWindow

configuration:
  files:
    - engine.config (via Core::EngineCore::loadConfig/saveConfig)
  runtime_selection:
    - via HardwareDetector + UpscalerFactory

errors_logging:
  - Use Console::safe* + SAFE_* macros everywhere
  - Exceptions only for exceptional cases in renderer/scene

threading:
  - Single-threaded main loop initially; leave hooks for worker threads in renderer

public_headers_layout:
  include/SpectraForge/App/Engine.h           # facade
  include/SpectraForge/App/IApp.h             # callbacks
  include/SpectraForge/App/Config.h           # AppConfig, WindowConfig, RendererConfig, UpscalerConfig
  include/SpectraForge/Core/*                 # reuse existing
  include/SpectraForge/Rendering/*            # reuse existing
  include/SpectraForge/Vulkan/*               # reuse existing
  include/SpectraForge/Upscaling/*            # reuse existing

minimal_api_signatures_cpp:
  - Engine facade:
      - bool initialize();
      - void shutdown();
      - void update(float dt);
      - void render();
      - bool load_scene(const std::string& path);
      - IWindow* get_window() const;
      - void set_window_title(const std::string&);

demo_loop_guidelines:
  - AAA: Arrange(init), Act(update/render), Assert(logging/stats)
  - FPS title update via Console::setTitle()

non_goals_now:
  - Full ECS; physics; editor tooling; network

testing_requirements:
  - Unit tests for Engine facade behaviors (80%+)
  - Integration tests for scene load + render stub
  - Mock IRenderer/ISceneManager for AAA tests

migration_notes:
  - Keep existing demos; add thin adapter to new App::Engine where needed
  - PSEUDO demo maps 1:1 via facade methods


