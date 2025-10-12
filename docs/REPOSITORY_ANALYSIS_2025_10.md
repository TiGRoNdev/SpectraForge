# SpectraForge Repository Analysis (October 2025)

## Executive Summary
- **Architecture**: High-level orchestration classes (`App::Engine`, `HybridFreGSRenderer`) still combine multiple responsibilities, leaving TODO blocks and platform bindings in production code. This creates fragility for future render paths and platform targets.
- **Infrastructure**: The DI setup wires a placeholder `NullResourceManager`, preventing persistent GPU resource ownership and masking lifetime issues discovered during refactors.
- **Quality Signals**: Documentation files (e.g., `MISSING_TESTS_MATRIX.md`) no longer reflect the current automated test suite, reducing trust in the written audit history and slowing onboarding.

## Architectural Findings
### 1. `App::Engine` still behaves like a coordinator
The facade pulls concrete types from the service locator and still manages window creation, Vulkan surface binding, input routing, camera setup, and scene state inside a single class.【F:src/app/Engine.cpp†L29-L200】 This makes it difficult to host alternative front ends (e.g., headless benchmarking) or replace GLFW/X11 bindings.

**Suggested Remediation**
- Extract platform-specific window/bootstrap code into dedicated strategies (`IWindowBootstrap`, `IGraphicsBootstrap`) resolved via DI. That keeps `App::Engine` focused on game-loop orchestration.
- Promote camera/state refresh to specialized systems (e.g., `ICameraController`, `ISceneLifecycle`) so `Engine::update` becomes a thin mediator.
- Introduce thin adapters around the DI service locator to allow deterministic construction in tests (e.g., pass in a scoped container rather than using `ServiceLocator::getInstance`).

### 2. `HybridFreGSRenderer` is an unfinished orchestrator
Although refactored, the renderer still combines swapchain ownership, pass scheduling, statistics, and debug UI toggles. Large swaths of behaviour remain as TODO placeholders (triangle pass setup, command buffer recording, mesh conversion, screenshot capture).【F:src/rendering/HybridFreGSRenderer.cpp†L21-L346】 Because these pieces are stubbed, production builds cannot yet exercise triangle splatting or FreGS uploads end-to-end.

**Suggested Remediation**
- Split renderer responsibilities into focused collaborators (e.g., `SwapchainOrchestrator`, `TriangleSplattingExecutor`, `FreGSUploadService`). Wire them via DI factories already exposed in `DISetup`.
- Move TODO logic into new implementation files and cover them with component-level tests; keep the orchestrator limited to sequencing calls.
- Gate unimplemented features behind capability flags so consuming code can degrade gracefully instead of calling stubs.

## Infrastructure Findings
### 3. DI registers a `NullResourceManager`
`App::DISetup` currently binds `IResourceManager` to a stub that returns invalid handles for every API and performs no lifetime tracking.【F:include/SpectraForge/App/DISetup.h†L140-L172】 This blocks integration of resource streaming and hides leaks that the production manager would surface.

**Suggested Remediation**
- Implement a thin Vulkan-aware resource manager that wraps existing buffer/texture helpers; register that implementation as the singleton instead of `NullResourceManager`.
- Provide a mock or in-memory resource manager for tests via scoped DI overrides to keep unit tests fast.

## Process & Quality Findings
### 4. Documentation drift on test coverage
`MISSING_TESTS_MATRIX.md` still flags 44 missing tests for `HybridFreGSRenderer`, yet the repository already ships a 70-test suite for that class and related passes.【F:MISSING_TESTS_MATRIX.md†L1-L103】【F:tests/hybrid_fregs_renderer_test.cpp†L1-L120】 Similar drift exists across other refactoring reports, undermining trust in written guidance.

**Suggested Remediation**
- Automate the generation of coverage and gap reports (e.g., `scripts/generate_test_matrix.py`) and integrate them into CI so `docs/` stays in sync.
- Add a short "living docs" checklist to PR templates requiring authors to update or explicitly acknowledge stale documents.

## Next Steps Checklist
1. Design bootstrap interfaces and refactor `App::Engine` to delegate platform-specific work.
2. Break `HybridFreGSRenderer` into discrete services and replace TODO placeholders with implemented collaborators.
3. Replace `NullResourceManager` with a production-ready implementation, providing mocks for tests.
4. Wire automated documentation updates into CI and clean up stale audit files.

Executing these steps will align runtime behaviour with the documented SOLID goals and restore confidence in the repository's reference material.
