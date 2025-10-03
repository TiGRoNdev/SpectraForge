IR_2: SpectraForge High-Level API IR (external research)
version: 0.1.0
date: 2025-10-03
sources:
  - Vulkan spec (swapchain, sync, submit2, present)
  - CUDA Toolkit (external memory/semaphore interop, stream attach, async copies)
  - NVIDIA Streamline DLSS integration (DLSS4 checklists, vk_streamline sample)
  - AMD FidelityFX FSR2 docs and GitHub sample

sections:
  1. App-facing facade & lifecycle
     - Interface `IRendererEngine` with Initialize/RunFrame/Resize/ReloadResources/Shutdown (noexcept)
     - RAII lifecycle, FrameContext per frame, DI for window/input/scene
     - Hot-reload of shaders/config, frame-graph orchestration

  2. Scene & resources
     - `IScene`/`ISceneNode` components: camera/light/renderable
     - GPU handles for splats/AS/textures; bindless descriptors; background streaming
     - Support for 3DGS/NeRF assets; upload-on-demand

  3. Render pipeline & sync
     - Stages: GS raster (Vulkan) -> OptiX ray effects -> AI denoise -> Upscale (DLSS/FSR2) -> UI
     - Timeline semaphores and external memory for Vulkan/CUDA/OptiX interop
     - vkAcquireNextImage/vkQueueSubmit2/vkQueuePresent synchronization per spec
     - Per-stage GPU markers, metrics (Nsight/RenderDoc friendly)

  4. Hardware detection & feature selection
     - Vendor/feature scan (RT, mesh shaders, CUDA/OptiX, DLSS/FSR2)
     - Enum-based policy with fallbacks: DLSS→FSR2→NONE; OptiX→fallback
     - Injectable capability registry; runtime toggles; warnings on mismatch

  5. Config & logging
     - JSON/TOML + env + CLI overrides; live reload
     - Structured logging with sinks; levels per module; frame reports
     - Centralized error handler; GPU exceptions with context

  6. Testability & SOLID
     - Interfaces for all subsystems; DI; mocks for devices/scenes/pipelines
     - RAII ownership, smart pointers; noexcept at boundaries
     - C++20 concepts for constraints; deterministic test hooks

interop_checklists:
  - Vulkan↔CUDA/OptiX:
    - Use external memory handles, external semaphores
    - Map buffers/images with cudaExternalMemoryGetMapped*; wait/signal with cudaWait/SignalExternalSemaphoresAsync
    - Synchronize via timeline semaphores; ensure layout/barrier correctness
  - Streamline (DLSS):
    - Integrate SR first, then Ray Reconstruction; validate inputs (motion vectors, depth, exposure)
    - Select interposer or manual hooks; handle HDR/NGX status and fallbacks
  - FSR2:
    - Provide color, depth, motion vectors, reactive/transparency masks
    - Quality modes; memory budget; typed UAV load requirement; Vulkan sample as reference


