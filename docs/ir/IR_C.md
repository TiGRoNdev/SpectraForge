IR_C: Consolidated Optimal API IR (IR_1 ⊕ IR_2)
version: 0.1.0
date: 2025-10-03

principles:
  - Preserve existing modules (EngineCore, Console, VulkanRenderer, SceneManager, HardwareDetector)
  - Add minimal App facade to match demo ergonomics
  - Enforce SOLID, DIP, RAII; enable feature-flagged pipeline stages

public_api:
  namespace: SpectraForge::App
  headers:
    - App/Engine.h
    - App/IApp.h
    - App/Config.h
  Engine:
    - Engine(const AppConfig&)
    - bool initialize()
    - void shutdown()
    - void update(float dt)
    - void render()
    - bool load_scene(const std::string& path)
    - IWindow* get_window() const
    - void set_window_title(const std::string&)
  IApp:
    - bool on_init(Engine&)
    - void on_update(Engine&, float dt)
    - void on_render(Engine&)
    - void on_shutdown(Engine&)

subsystems_mapping:
  - Core::EngineCore orchestrates DI of:
      - Rendering::IRenderer → Vulkan::VulkanRenderer
      - Rendering::IResourceManager → Vulkan::ResourceManager
      - Scene → Vulkan::SceneManager
      - HW → Vulkan::HardwareDetector (+ UpscalerFactory)
  - Console/SafeConsole for logging and SAFE_* macros

render_pipeline:
  - Stages (feature toggles via HardwareDetector):
      1) GS raster (Vulkan + CUDA interop when available)
      2) Ray effects (OptiX)
      3) AI denoise (OptiX)
      4) Upscale (Streamline DLSS or FSR2)
      5) Present (swapchain)
  - Sync:
      - Acquire → Submit2 with timeline semaphores → Present
      - CUDA/OptiX interop via external memory + external semaphores

config_logging:
  - Use existing EngineCore config (engine.config) + AppConfig overlay
  - Structured logging; SAFE_* output at app boundary

testability:
  - Interfaces for renderer/scene/window; mocks in tests
  - AAA tests for facade behaviors; integration tests for scene load + stub frame

migration:
  - Keep demos; provide thin PSEUDO mapping; no breaking changes to core modules

roadmap_slices:
  - Slice A (facade & wiring): add App::Engine/IApp/Config headers + adapters
  - Slice B (interop sync hardening): introduce timeline semaphores path
  - Slice C (upscaling providers): integrate Streamline and FSR2 behind UpscalerFactory
  - Slice D (tests/docs): unit/integration tests, API docs


