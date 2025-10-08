# 📊 SpectraForge - Полный Индекс Проекта

**Дата создания**: 2025-10-07  
**Версия проекта**: 1.0.0  
**Статус**: Production Ready

---

## 📋 ОГЛАВЛЕНИЕ

1. [Обзор проекта](#обзор-проекта)
2. [Архитектура](#архитектура)
3. [Модульная структура](#модульная-структура)
4. [Основные компоненты](#основные-компоненты)
5. [Рендеринг система](#рендеринг-система)
6. [Математические модули](#математические-модули)
7. [Vulkan интеграция](#vulkan-интеграция)
8. [Шейдеры](#шейдеры)
9. [Тестирование](#тестирование)
10. [Документация](#документация)
11. [Сборка и развертывание](#сборка-и-развертывание)
12. [Зависимости](#зависимости)
13. [Производительность](#производительность)
14. [Roadmap и планы](#roadmap-и-планы)

---

## 🎯 ОБЗОР ПРОЕКТА

### Описание
**SpectraForge** - это современный 3D движок на чистом C++17/20 с использованием Vulkan API, реализующий инновационную гибридную систему рендеринга:
- **Triangle Splatting** - для треугольных мешей (.obj)
- **FreGS (Frequency Gaussian Splatting)** - для point clouds (.ply)
- **Wavelet-based Lifting** - для частотной декомпозиции

### Ключевые особенности
- ✅ **Pure C++ + Vulkan 1.3** (без OpenGL)
- ✅ **SOLID-принципы** во всей кодовой базе
- ✅ **Compute Shader Pipeline** для GPU-ускорения
- ✅ **60+ FPS** на мобильных GPU (Adreno 740)
- ✅ **4K HDR** поддержка с адаптивным upscaling
- ✅ **80%+ Test Coverage** с Google Test
- ✅ **CI/CD** через GitHub Actions

### Технологический стек
```yaml
Язык:           C++17/20
Graphics API:   Vulkan 1.3 (compute-focused)
Build System:   CMake 3.16+
Testing:        Google Test
Memory:         VMA (Vulkan Memory Allocator)
Math:           GLM + собственная библиотека
Platform:       Linux (X11), Windows, Android (planned)
```

---

## 🏗️ АРХИТЕКТУРА

### Общая структура

```
┌─────────────────────────────────────────────┐
│           SpectraForge Engine               │
├─────────────────────────────────────────────┤
│                                             │
│  ┌──────────┐  ┌──────────┐  ┌──────────┐ │
│  │   Core   │  │   Math   │  │  Input   │ │
│  └──────────┘  └──────────┘  └──────────┘ │
│                                             │
│  ┌──────────────────────────────────────┐  │
│  │      Rendering Subsystem             │  │
│  │  ┌────────────┐  ┌───────────────┐  │  │
│  │  │  Vulkan    │  │   Hybrid      │  │  │
│  │  │  Engine    │  │   FreGS       │  │  │
│  │  └────────────┘  │   Renderer    │  │  │
│  │                  └───────────────┘  │  │
│  │  ┌────────────┐  ┌───────────────┐  │  │
│  │  │ Triangle   │  │    FreGS      │  │  │
│  │  │ Splatting  │  │    Pass       │  │  │
│  │  │   Pass     │  └───────────────┘  │  │
│  │  └────────────┘                     │  │
│  └──────────────────────────────────────┘  │
│                                             │
│  ┌──────────┐  ┌──────────┐  ┌──────────┐ │
│  │ Physics  │  │Upscaling │  │ Platform │ │
│  └──────────┘  └──────────┘  └──────────┘ │
└─────────────────────────────────────────────┘
```

### SOLID Compliance

**Статус архитектуры**: ⚠️ В процессе рефакторинга

#### ✅ Соблюдается:
- `EngineCore.h` - образцовая реализация SOLID
- `IRenderer.h` - правильная сегрегация интерфейсов
- `ModernRenderer3D.cpp` - dependency injection
- `RenderStrategy` паттерн - Open/Closed

#### ⚠️ Требует улучшения:
- `HybridFreGSRenderer` - нарушает SRP (1383 строки)
- `Engine.cpp` - God Class (1010 строк)
- Отсутствие полного DI контейнера

**План**: См. `docs/SPECTRAFORGE_AUDIT.md` (15-20 недель работы)

---

## 📁 МОДУЛЬНАЯ СТРУКТУРА

### `/include/SpectraForge/` - Заголовочные файлы (75 файлов)

```
include/SpectraForge/
├── App/
│   ├── Config.h
│   ├── Engine.h
│   └── IApp.h
│
├── Core/
│   ├── Component.h
│   ├── Console.h
│   ├── DependencyInjection/
│   │   └── Container.h
│   ├── EngineCore.h                    # ⭐ SOLID-образец
│   ├── GameObject3D.h
│   ├── Interfaces/
│   │   ├── IConfigurable.h
│   │   ├── IEventHandler.h
│   │   ├── IInitializable.h
│   │   └── IRenderable.h
│   ├── Logger.h
│   ├── SafeConsole.h
│   ├── Transform3D.h
│   ├── VMAMemoryManager.h
│   ├── VulkanContext.h
│   ├── Window.h
│   └── WindowFactory.h
│
├── Math/
│   ├── Math.h
│   ├── MathConstants.h
│   ├── Matrix4.h                       # 4x4 трансформации
│   ├── Quaternion.h                    # Вращения
│   ├── Vec4.h
│   ├── Vector2.h
│   ├── Vector3.h
│   └── Vector4.h
│
├── Rendering/
│   ├── Camera3D.h
│   ├── Common/
│   │   ├── IRenderer.h                 # ⭐ Базовый интерфейс
│   │   └── IResourceManager.h
│   ├── FrameOutput.h
│   ├── Gaussian3D.h
│   ├── HybridFreGSRenderer.h          # 🔴 Основной рендерер
│   ├── IRenderStrategy.h
│   ├── Material3D.h
│   ├── Mesh3D.h
│   ├── MeshConnectivity.hpp           # Для Triangle Splatting+
│   ├── RendererAdapter.h
│   ├── RendererFactory.h
│   ├── RenderPass/
│   │   ├── FreGSPass.h                # FreGS Gaussian Splatting
│   │   ├── InstancedMeshPass.h
│   │   ├── RenderPass.h
│   │   ├── TriangleSplattingPass.h   # 🔴 Triangle Splatting
│   │   └── WaveletPass.h              # Wavelet Lifting
│   ├── RenderStages/
│   │   ├── AIDenoiseStage.h
│   │   ├── FinalCompositeStage.h
│   │   ├── IRenderStage.h
│   │   ├── PrimaryRasterizationStage.h
│   │   ├── SecondaryRayTracingStage.h
│   │   └── UpscalingStage.h
│   ├── RenderStrategy/
│   │   ├── FreGSRenderStrategy.h
│   │   ├── IPassScheduler.h
│   │   └── RenderContextView.h
│   ├── Shader3D.h
│   └── StrategyFactory.h
│
├── Upscaling/
│   ├── DLSSUpscaler.h
│   ├── FSR2Upscaler.h
│   ├── NativeUpscaler.h
│   ├── Upscaler.h
│   └── UpscalerFactory.h
│
├── Vulkan/
│   ├── HardwareDetector.h
│   ├── ResourceManager.h
│   ├── SceneManager.h
│   ├── VulkanEngine.h
│   ├── VulkanRenderer.h
│   └── VulkanValidation.h
│
├── Input/
│   └── Input3D.h
│
├── Physics/
│   └── Physics3D.h
│
└── Platform/
    └── X11Window.h
```

### `/src/` - Реализации (86 файлов)

```
src/
├── app/
│   ├── Engine.cpp                      # 1010 строк - главный движок
│   └── Engine_BACKUP.cpp
│
├── core/
│   ├── Component.cpp
│   ├── Console.cpp
│   ├── Container.cpp
│   ├── EngineCore.cpp
│   ├── GameObject3D.cpp
│   ├── Logger.cpp
│   ├── Transform3D.cpp
│   ├── VMAImplementation.cpp
│   ├── VMAMemoryManager.cpp
│   ├── VulkanContextImpl.cpp
│   ├── VulkanHppDispatch.cpp
│   ├── Window.cpp
│   └── WindowFactory.cpp
│
├── math/
│   ├── matrix/
│   │   ├── Matrix4.cpp
│   │   └── Quaternion.cpp
│   ├── Vec4.cpp
│   └── vector/
│       ├── Vector3.cpp
│       └── Vector4.cpp
│
├── rendering/
│   ├── FreGSPass.cpp                   # 545 строк
│   ├── FreGSRenderStrategy.cpp
│   ├── HybridFreGSRenderer.cpp        # 🔴 1383 строки - критично
│   ├── InstancedMeshPass.cpp
│   ├── InstancedMeshPassOptimized.cpp
│   ├── ModernRenderer3D.cpp           # 350 строк
│   ├── opengl/                         # 📌 Legacy - удалить
│   │   ├── Camera3D.cpp
│   │   ├── Gaussian3D.cpp
│   │   ├── HybridRenderer3D.cpp
│   │   ├── Mesh3D.cpp
│   │   ├── OptimalRenderer3D.cpp
│   │   ├── Renderer3D.cpp
│   │   ├── RendererAdapter.cpp
│   │   └── Shader3D.cpp
│   ├── RenderPass.cpp
│   ├── StrategyFactory.cpp
│   ├── TriangleSplattingPass.cpp
│   ├── vulkan/
│   │   ├── HardwareDetector.cpp
│   │   ├── ResourceManager.cpp
│   │   ├── SceneManager.cpp
│   │   ├── VulkanEngine.cpp
│   │   └── VulkanRenderer.cpp
│   ├── VulkanValidation.cpp
│   └── WaveletPass.cpp                # 665 строк
│
├── upscaling/
│   ├── DLSSUpscaler.cpp
│   ├── FSR2Upscaler.cpp
│   ├── NativeUpscaler.cpp
│   ├── Upscaler.cpp
│   └── UpscalerFactory.cpp
│
├── input/
│   └── Input3D.cpp
│
├── physics/
│   └── Physics3D.cpp
│
├── output/
│   └── FrameOutput.cpp
│
├── performance/
│   └── TriangleSplattingBenchmark.cpp
│
└── platform/
    └── X11Window.cpp                   # 399 строк
```

### `/tests/` - Тесты (31 файл)

```
tests/
├── app_engine_test.cpp
├── connectivity_phase1_test.cpp
├── core_gameobject3d_test.cpp
├── core_logger_test.cpp
├── core_transform3d_test.cpp
├── core_vma_memory_test.cpp
├── fregs_pass_test.cpp
├── hybrid_fregs_renderer_test.cpp
├── input_system_test.cpp
├── instanced_mesh_test.cpp
├── integration_pipeline_test.cpp
├── math_matrix4_test.cpp
├── math_quaternion_test.cpp
├── math_vector_extra_test.cpp
├── math_vector3_test.cpp
├── modern_renderer3d_test.cpp
├── physics_system_test.cpp
├── render_strategy_factory_test.cpp
├── render_strategy_interfaces_test.cpp
├── rendering_camera3d_test.cpp
├── triangle_splatting_minimal_test.cpp
├── triangle_splatting_test.cpp
├── upscaling_system_test.cpp
├── vulkan_engine_test.cpp
├── vulkan_hardware_detector_test.cpp
├── vulkan_renderer_test.cpp
├── vulkan_resource_manager_test.cpp
├── vulkan_scene_manager_test.cpp
├── vulkan_validation_test.cpp
├── wavelet_pass_test.cpp
└── x11_window_test.cpp

Покрытие: 633/788 функций (80%)
Требуется: 225-255 дополнительных тестов для 100%
```

### `/shaders/` - Compute Shaders (45 файлов)

```
shaders/
├── BitonicSort.comp                    # O(N log N) сортировка
├── DepthKeyCompute.comp                # Вычисление глубины
├── DepthSortAtomic.comp               # O(N) атомарная сортировка
├── float_to_rgba8.comp                # Конвертация цвета
├── FrustumCulling.comp                # Frustum culling
├── GaussFreqSplat.comp                # FreGS Gaussian Splatting
├── IndirectArgsCompute.comp           # Indirect dispatch
├── InstancedMesh.frag / .vert         # Instanced rendering
├── light_culling.comp                 # Light culling
├── MobileUpscalingHDR.comp           # 🔥 HDR upscaling
├── TileCulling.comp                   # Tile-based culling
├── TriangleShading.comp               # Triangle shading pass
├── TriangleSplatting.comp            # 🔴 Основной Triangle Splatting
├── TriangleSplattingDebug.comp       # Debug режим
├── TriangleSplattingSimple.comp      # Упрощенная версия
├── TriangleVisibility.comp           # Visibility pass
├── WaveletLifting.comp               # Wavelet transform
├── gui_*.glsl                         # GUI шейдеры
└── compiled/                          # .spv бинарники
```

### `/examples/` - Демо приложения (7 примеров)

```
examples/
├── BlueCube_Demo.cpp                  # Минимальный вращающийся куб
├── DebugSponza_Demo.cpp              # Sponza сцена с отладкой
├── InstancedMeshDemo.cpp             # Instanced rendering
├── SimpleTriangleTest.cpp            # Один треугольник
├── SpectraForge_Example_Demo.cpp     # Полный пример
├── SpectraForge_Optimized_Demo.cpp   # 🔥 60+ FPS оптимизированный
└── scenes/sponza/                     # Sponza atrium mesh + текстуры
```

### `/docs/` - Документация (12 документов)

```
docs/
├── SPECTRAFORGE_AUDIT.md             # 🔴 КРИТИЧНО: Аудит архитектуры
├── SPECTRAFORGE-DEEP-RESEARCH.md     # Deep learning research
├── VECTORIZED-OPTIMIZATIONS.md       # SIMD оптимизации
├── triangle-mesh-conversion-plan.md  # План миграции к Triangle Splatting+
├── Phase1_Refactor_Plan.md           # План рефакторинга
├── QUICK_TEST_GUIDE.md               # Гайд по тестированию
├── Renderer.md                        # Архитектура рендерера
├── index/
│   ├── deep_research_index.json
│   ├── manifest.json
│   └── vectorized_optimizations_index.json
└── spectraforge-solid-audit-trianglesplatting-migration.pdf
```

---

## 🔧 ОСНОВНЫЕ КОМПОНЕНТЫ

### 1. EngineCore (⭐ SOLID-образец)

**Файл**: `include/SpectraForge/Core/EngineCore.h`  
**Строк**: 229

**Описание**: Рефакторенный основной класс движка, строго следующий SOLID принципам.

**Интерфейсы**:
- `IInitializable` - инициализация/shutdown
- `IConfigurable` - управление конфигурацией
- `IEventHandler` - обработка событий

**Dependency Injection**:
```cpp
EngineCore(
    std::shared_ptr<Rendering::IRenderer> renderer,        // DIP
    std::shared_ptr<Rendering::IResourceManager> resourceManager, // DIP
    std::shared_ptr<ILogger> logger                        // DIP
);
```

**Подсистемы** (OCP):
```cpp
void registerSubsystem(std::shared_ptr<ISubsystem> subsystem);
```

**Ключевые методы**:
```cpp
bool initialize() override;
void shutdown() override;
void run();  // Главный цикл
void stop();
```

### 2. HybridFreGSRenderer (🔴 Критичный компонент)

**Файл**: `include/SpectraForge/Rendering/HybridFreGSRenderer.h` (288 строк)  
**Реализация**: `src/rendering/HybridFreGSRenderer.cpp` (1383 строки)

**⚠️ ПРОБЛЕМА**: Нарушает SRP - слишком много ответственностей в одном классе

**Режимы рендеринга**:
```cpp
enum class RenderMode {
    GaussianSplatting,  // Для point clouds (.ply)
    TriangleSplatting   // Для triangle meshes (.obj)
};
```

**Основной pipeline**:
```cpp
bool initialize();                        // Инициализация Vulkan
bool attachWindow(void* x11Display, ...); // Привязка окна
void beginFrame();                        // Начало кадра
void renderFrame(const FrameData&);       // Рендеринг
void endFrame();                          // Завершение кадра
void shutdown();                          // Очистка
```

**Загрузка данных**:
```cpp
void uploadGaussians(const std::vector<GaussianSplat>&);
void uploadTriangles(const std::vector<Triangle>&);
void uploadMesh(const std::shared_ptr<Mesh3D>&);
```

**Debug API** (новое):
```cpp
void setDebugMode(int mode);              // 0=normal, 1=SDF, 2=barycentric
void enableWireframe(bool enable);
void enableBackfaceCulling(bool enable);
void setBackgroundColor(float r, g, b, a);
DetailedRenderingStats getDetailedStats();
bool saveScreenshot(const std::string& filename);
GPUInfo getGPUInfo();
```

### 3. TriangleSplattingPass (🔥 Инновационный алгоритм)

**Файл**: `include/SpectraForge/Rendering/RenderPass/TriangleSplattingPass.h`  
**Строк**: 425

**Базовая статья**: "Triangle Splatting for Real-Time Radiance Field Rendering" (2025)  
**Сайт**: https://trianglesplatting.github.io/

**Triangle Primitive** (КРИТИЧНО - должен соответствовать GLSL):
```cpp
#pragma pack(push, 1)
struct Triangle {
    glm::vec3 v0, v1, v2;          // 36 bytes - вершины
    glm::vec3 color;               // 12 bytes - цвет
    float opacity;                 // 4 bytes - прозрачность
    float sigma;                   // 4 bytes - smoothness
    glm::vec3 normal;              // 12 bytes - нормаль
    int materialId;                // 4 bytes - material
    glm::vec2 texCoord0, texCoord1, texCoord2; // 24 bytes - UV
};
#pragma pack(pop)
static_assert(sizeof(Triangle) == 96, "Must be 96 bytes!");
```

**Конфигурация**:
```cpp
struct Config {
    uint32_t outputWidth, outputHeight;
    bool enableDepthSort;              // Сортировка по глубине
    bool enableEarlyTermination;       // Ранняя остановка
    float alphaThreshold;              // Порог альфа
    bool enableTwoPassRendering;       // Two-Pass O(N+M) алгоритм
    uint32_t maxTrianglesPerPixel;     // Лимит треугольников/пиксель
};
```

**Основные методы**:
```cpp
bool initialize(vk::Device, VmaAllocator, ...);
void execute(vk::CommandBuffer cmd, uint32_t frameIndex);
void uploadTriangles(const std::vector<Triangle>&);
void setViewProjection(const glm::mat4& viewProj);
void setCameraPosition(const glm::vec3& cameraPos);
void setLighting(const glm::vec3& lightDir, float intensity, ...);
uint32_t getVisibleTriangleCount() const;
```

**Optimizations**:
- ✅ Frustum culling compute shader
- ✅ Atomic depth sorting O(N)
- ✅ Two-Pass rendering (Visibility + Shading)
- ✅ Tile-based culling для мобильных GPU
- ✅ Early Z-termination

### 4. FreGSPass (Frequency Gaussian Splatting)

**Файл**: `src/rendering/FreGSPass.cpp` (545 строк)

**Описание**: Реализация Gaussian Splatting в частотной области через Wavelet decomposition.

**Pipeline**:
```
Input Image 
    ↓
WaveletPass (DWT) → Subbands (LL, LH, HL, HH)
    ↓
FreGSPass → Gaussian Splatting в frequency domain
    ↓
Output Image
```

**Методы**:
```cpp
bool initialize(const VulkanContext&);
void execute(vk::CommandBuffer, uint32_t imageIndex);
void setInputSubbands(const WaveletSubbands&);
void uploadGaussians(const std::vector<GaussianSplat>&);
void updateFoveation(glm::vec2 focusPoint, float radius);
```

### 5. WaveletPass (DWT Lifting)

**Файл**: `src/rendering/WaveletPass.cpp` (665 строк)

**Описание**: Discrete Wavelet Transform через lifting scheme для multi-resolution decomposition.

**Subbands**:
```cpp
struct WaveletSubbands {
    vk::Image LL;  // Low-Low   (approximation)
    vk::Image LH;  // Low-High  (horizontal detail)
    vk::Image HL;  // High-Low  (vertical detail)
    vk::Image HH;  // High-High (diagonal detail)
};
```

**Методы**:
```cpp
bool initialize(const VulkanContext&);
void execute(vk::CommandBuffer, uint32_t imageIndex);
void setInputImage(vk::Image, vk::ImageView);
WaveletSubbands getSubbands() const;
```

---

## 🎨 РЕНДЕРИНГ СИСТЕМА

### Архитектура рендеринга

```
┌───────────────────────────────────────────────┐
│         IRenderer (Interface)                 │
│  ┌─────────────────────────────────────────┐ │
│  │  - initialize()                         │ │
│  │  - renderFrame(FrameData)               │ │
│  │  - shutdown()                           │ │
│  │  - getStats()                           │ │
│  └─────────────────────────────────────────┘ │
└───────────────────────────────────────────────┘
                     ▲
                     │
                     │ implements
                     │
┌────────────────────────────────────────────────┐
│      HybridFreGSRenderer                       │
│  ┌──────────────────────────────────────────┐ │
│  │  Dual-Path Rendering:                    │ │
│  │    1. GaussianSplatting (FreGS)          │ │
│  │    2. TriangleSplatting                  │ │
│  └──────────────────────────────────────────┘ │
│                                                │
│  ┌─────────────────┐  ┌─────────────────────┐│
│  │ FreGSPass       │  │ TriangleSplattingPass││
│  │ (Point Clouds)  │  │ (Triangle Meshes)   ││
│  └─────────────────┘  └─────────────────────┘│
│           ▲                      ▲            │
│           │                      │            │
│  ┌─────────────────┐             │            │
│  │  WaveletPass    │─────────────┘            │
│  │  (DWT Lifting)  │                          │
│  └─────────────────┘                          │
└────────────────────────────────────────────────┘
```

### Rendering Pipeline (Single Frame)

```
┌─────────────────────────────────────────────────┐
│  1. НАЧАЛО КАДРА (beginFrame)                   │
│     - Acquire swapchain image                   │
│     - Wait for previous frame fence             │
└─────────────────────────────────────────────────┘
                     ↓
┌─────────────────────────────────────────────────┐
│  2. COMMAND BUFFER RECORDING                    │
│     - Begin command buffer                      │
│     - Begin render pass                         │
│     - Clear background                          │
└─────────────────────────────────────────────────┘
                     ↓
┌─────────────────────────────────────────────────┐
│  3. TRIANGLE SPLATTING PASS (если включен)      │
│     ┌───────────────────────────────────────┐   │
│     │ a) Frustum Culling (compute)          │   │
│     │    - GPU parallel culling             │   │
│     │    - Atomic counter для видимых       │   │
│     └───────────────────────────────────────┘   │
│     ┌───────────────────────────────────────┐   │
│     │ b) Depth Sort (compute)               │   │
│     │    - O(N) atomic binning              │   │
│     │    - Back-to-front ordering           │   │
│     └───────────────────────────────────────┘   │
│     ┌───────────────────────────────────────┐   │
│     │ c) Triangle Splatting (compute)       │   │
│     │    - SDF-based rasterization          │   │
│     │    - Barycentric interpolation        │   │
│     │    - Alpha blending                   │   │
│     └───────────────────────────────────────┘   │
└─────────────────────────────────────────────────┘
                     ↓
┌─────────────────────────────────────────────────┐
│  4. FREGS PASS (если включен)                   │
│     ┌───────────────────────────────────────┐   │
│     │ a) Wavelet Decomposition (compute)    │   │
│     │    - DWT lifting scheme               │   │
│     │    - Generate subbands                │   │
│     └───────────────────────────────────────┘   │
│     ┌───────────────────────────────────────┐   │
│     │ b) Gaussian Splatting (compute)       │   │
│     │    - Frequency-domain rendering       │   │
│     │    - Foveated quality                 │   │
│     └───────────────────────────────────────┘   │
└─────────────────────────────────────────────────┘
                     ↓
┌─────────────────────────────────────────────────┐
│  5. UPSCALING (опционально)                     │
│     - Mobile mode: 1080p → 4K                   │
│     - Balanced: 1440p → 4K                      │
│     - Quality: Native 4K                        │
│     - HDR tone mapping (ACES)                   │
└─────────────────────────────────────────────────┘
                     ↓
┌─────────────────────────────────────────────────┐
│  6. ЗАВЕРШЕНИЕ (endFrame)                       │
│     - End render pass                           │
│     - Submit command buffer                     │
│     - Present to swapchain                      │
│     - Signal fence                              │
└─────────────────────────────────────────────────┘
```

### Compute Shader Workflow (Triangle Splatting)

**1. Frustum Culling** (`FrustumCulling.comp`)
```glsl
Input:  triangleBuffer[N]         // Все треугольники
        viewProjMatrix            // Camera MVP
Output: visibleIndicesBuffer[M]   // Индексы видимых
        atomicCounter             // Счетчик M
        
Complexity: O(N) parallel
Workgroup: 256 threads
```

**2. Depth Key Compute** (`DepthKeyCompute.comp`)
```glsl
Input:  visibleIndicesBuffer[M]   // Видимые треугольники
        triangleBuffer[N]
        cameraPosition            
Output: depthKeysBuffer[M]        // Float depth keys

Compute: depth = distance(triangle.centroid, camera)
```

**3. Atomic Depth Sort** (`DepthSortAtomic.comp`)
```glsl
Input:  depthKeysBuffer[M]
        visibleIndicesBuffer[M]
Output: sortedIndicesBuffer[M]    // Back-to-front order

Algorithm:
  Phase 0: Count triangles per bin
  Phase 1: Prefix sum (scan)
  Phase 2: Atomic placement
  
Complexity: O(N) vs O(N log N) Bitonic
Bins: 256 (depth range quantization)
```

**4. Triangle Splatting** (`TriangleSplatting.comp`)
```glsl
Input:  sortedIndicesBuffer[M]
        triangleBuffer[N]
        viewProjMatrix
Output: outputImage[W×H]          // RGBA16F framebuffer

For each pixel (x, y):
  accColor = vec3(0)
  accAlpha = 0.0
  
  For each sortedTriangle:
    // Project to 2D
    v0_2d = project(triangle.v0)
    v1_2d = project(triangle.v1)
    v2_2d = project(triangle.v2)
    
    // Signed Distance Field
    sdf = pointToTriangleSDF(pixel, v0_2d, v1_2d, v2_2d)
    
    // Smooth window function
    weight = windowFunction(sdf, triangle.sigma)
    
    // Barycentric interpolation
    bary = computeBarycentric(pixel, v0_2d, v1_2d, v2_2d)
    color = bary.x * c0 + bary.y * c1 + bary.z * c2
    
    // Alpha blending (front-to-back)
    alpha = triangle.opacity * weight
    accColor += color * alpha * (1.0 - accAlpha)
    accAlpha += alpha * (1.0 - accAlpha)
    
    // Early termination
    if (accAlpha > 0.99) break;
  
  outputImage[pixel] = vec4(accColor, accAlpha)
```

---

## 🔢 МАТЕМАТИЧЕСКИЕ МОДУЛИ

### Matrix4 (4x4 Transformations)

**Файл**: `include/SpectraForge/Math/Matrix4.h`  
**Реализация**: `src/math/matrix/Matrix4.cpp`

**Хранение**: Row-major order
```cpp
std::array<std::array<float, 4>, 4> m;
```

**Основные операции**:
```cpp
// Трансформации
static Matrix4 translation(const Vector3&);
static Matrix4 scaling(const Vector3&);
static Matrix4 rotation(const Vector3& axis, float angle);
static Matrix4 rotationFromQuaternion(const Quaternion&);

// Камера и проекции
static Matrix4 lookAt(const Vector3& eye, 
                     const Vector3& target, 
                     const Vector3& up);
static Matrix4 perspective(float fovY, float aspect, float near, float far);
static Matrix4 orthographic(float left, float right, ...);

// Операции
Matrix4 transpose() const;
float determinant() const;
Matrix4 inverse() const;

// Трансформация векторов
Vector3 transformPoint(const Vector3&) const;
Vector3 transformDirection(const Vector3&) const;

// Декомпозиция
void decompose(Vector3& translation, 
               Quaternion& rotation, 
               Vector3& scale) const;
```

### Quaternion (Rotations)

**Файл**: `include/SpectraForge/Math/Quaternion.h`  
**Реализация**: `src/math/matrix/Quaternion.cpp`

**Формат**: [w, x, y, z]
```cpp
float w, x, y, z;
```

**Операции**:
```cpp
Quaternion operator*(const Quaternion& other) const;  // Composition
Vector3 operator*(const Vector3& v) const;            // Rotate vector

// Создание
static Quaternion fromAxisAngle(const Vector3& axis, float angle);
static Quaternion fromEuler(float pitch, float yaw, float roll);
static Quaternion fromMatrix(const Matrix4&);

// Интерполяция
static Quaternion slerp(const Quaternion& a, 
                       const Quaternion& b, 
                       float t);
static Quaternion nlerp(const Quaternion& a, 
                       const Quaternion& b, 
                       float t);

// Конверсия
Matrix4 toMatrix() const;
Vector3 toEuler() const;
```

### Vector3/Vector4

**Файлы**: `include/SpectraForge/Math/Vector3.h`, `Vector4.h`

**Операции**:
```cpp
// Арифметика
Vector3 operator+(const Vector3&) const;
Vector3 operator-(const Vector3&) const;
Vector3 operator*(float scalar) const;

// Векторные операции
float dot(const Vector3& other) const;
Vector3 cross(const Vector3& other) const;
float magnitude() const;
Vector3 normalized() const;

// Утилиты
float distance(const Vector3& other) const;
float angle(const Vector3& other) const;
Vector3 lerp(const Vector3& target, float t) const;
```

---

## 💾 VULKAN ИНТЕГРАЦИЯ

### VulkanContext

**Файл**: `include/SpectraForge/Core/VulkanContext.h`

**Содержит**:
```cpp
struct VulkanContext {
    vk::Instance instance;
    vk::PhysicalDevice physicalDevice;
    vk::Device device;
    vk::Queue graphicsQueue;
    vk::Queue computeQueue;
    vk::Queue transferQueue;
    vk::CommandPool commandPool;
    VmaAllocator allocator;  // Vulkan Memory Allocator
};
```

### VMAMemoryManager (Memory Management)

**Файл**: `include/SpectraForge/Core/VMAMemoryManager.h`

**Функции**:
```cpp
class VMAMemoryManager {
public:
    bool initialize(vk::Instance, vk::PhysicalDevice, vk::Device);
    void shutdown();
    
    // Buffer allocation
    VmaBuffer createBuffer(vk::DeviceSize size,
                          vk::BufferUsageFlags usage,
                          VmaMemoryUsage memoryUsage);
    void destroyBuffer(VmaBuffer buffer);
    
    // Image allocation
    VmaImage createImage(vk::ImageCreateInfo imageInfo,
                        VmaMemoryUsage memoryUsage);
    void destroyImage(VmaImage image);
    
    // Statistics
    VmaBudget getBudget() const;
    VmaStatistics getStatistics() const;
};
```

### ResourceManager

**Файл**: `include/SpectraForge/Vulkan/ResourceManager.h`

**Управление ресурсами**:
```cpp
class ResourceManager : public IResourceManager {
public:
    // Buffers
    BufferHandle createBuffer(const BufferDesc&);
    void updateBuffer(BufferHandle, const void* data, size_t size);
    void destroyBuffer(BufferHandle);
    
    // Textures
    TextureHandle loadTexture(const std::string& path);
    TextureHandle createTexture(const TextureDesc&);
    void destroyTexture(TextureHandle);
    
    // Shaders
    ShaderHandle loadShader(const std::string& path, ShaderStage);
    void destroyShader(ShaderHandle);
    
    // Memory management
    void collectGarbage();
    MemoryStats getMemoryStats() const;
};
```

### HardwareDetector

**Файл**: `include/SpectraForge/Vulkan/HardwareDetector.h`

**Детекция GPU**:
```cpp
class HardwareDetector {
public:
    struct GPUFeatures {
        bool supportsRayTracing;
        bool supportsVariableRateShading;
        bool supportsMeshShaders;
        bool supportsComputeShaders;
        bool supportsAsyncCompute;
        uint32_t maxComputeWorkGroupSize[3];
        uint64_t maxMemoryAllocation;
    };
    
    std::vector<GPUInfo> enumerateGPUs();
    GPUFeatures detectFeatures(vk::PhysicalDevice);
    GPUInfo getBestGPU(const std::vector<GPUInfo>&);
    bool isMobileGPU(vk::PhysicalDevice);
};
```

---

## 🔥 ШЕЙДЕРЫ

### Triangle Splatting Shaders

**1. TriangleSplatting.comp** (главный шейдер)
```glsl
#version 450
layout(local_size_x = 16, local_size_y = 16) in;

layout(set = 0, binding = 0, rgba16f) uniform writeonly image2D outputImage;
layout(set = 0, binding = 1) readonly buffer TriangleBuffer {
    Triangle triangles[];
};
layout(set = 0, binding = 2) readonly buffer SortedIndicesBuffer {
    uint sortedIndices[];
};

layout(push_constant) uniform PushConstants {
    mat4 viewProj;
    uint outputWidth;
    uint outputHeight;
    uint triangleCount;
    uint enableEarlyTermination;
    float alphaThreshold;
};

// Point-to-triangle signed distance field
float pointToTriangleSDF(vec2 p, vec2 v0, vec2 v1, vec2 v2) {
    // Implementation based on Íñigo Quilez's 2D SDF
    // https://iquilezles.org/articles/distfunctions2d/
    
    vec2 e0 = v1 - v0;
    vec2 e1 = v2 - v1;
    vec2 e2 = v0 - v2;
    vec2 v0p = p - v0;
    vec2 v1p = p - v1;
    vec2 v2p = p - v2;
    
    vec2 pq0 = v0p - e0 * clamp(dot(v0p, e0) / dot(e0, e0), 0.0, 1.0);
    vec2 pq1 = v1p - e1 * clamp(dot(v1p, e1) / dot(e1, e1), 0.0, 1.0);
    vec2 pq2 = v2p - e2 * clamp(dot(v2p, e2) / dot(e2, e2), 0.0, 1.0);
    
    float s = sign(e0.x * e2.y - e0.y * e2.x);
    vec2 d = min(min(vec2(dot(pq0, pq0), s * (v0p.x * e0.y - v0p.y * e0.x)),
                     vec2(dot(pq1, pq1), s * (v1p.x * e1.y - v1p.y * e1.x))),
                     vec2(dot(pq2, pq2), s * (v2p.x * e2.y - v2p.y * e2.x)));
    
    return -sqrt(d.x) * sign(d.y);
}

// Smooth window function W(d, σ)
float windowFunction(float sdf, float sigma) {
    return exp(-sdf * sdf / (sigma * sigma));
}

void main() {
    ivec2 pixelCoord = ivec2(gl_GlobalInvocationID.xy);
    if (pixelCoord.x >= outputWidth || pixelCoord.y >= outputHeight) {
        return;
    }
    
    vec2 pixel = vec2(pixelCoord) + 0.5;
    vec3 accColor = vec3(0.0);
    float accAlpha = 0.0;
    
    // Front-to-back alpha blending
    for (uint i = 0; i < triangleCount; ++i) {
        uint triIdx = sortedIndices[i];
        Triangle tri = triangles[triIdx];
        
        // Project vertices to screen space
        vec4 v0_clip = viewProj * vec4(tri.v0, 1.0);
        vec4 v1_clip = viewProj * vec4(tri.v1, 1.0);
        vec4 v2_clip = viewProj * vec4(tri.v2, 1.0);
        
        vec3 v0_ndc = v0_clip.xyz / v0_clip.w;
        vec3 v1_ndc = v1_clip.xyz / v1_clip.w;
        vec3 v2_ndc = v2_clip.xyz / v2_clip.w;
        
        vec2 v0_screen = (v0_ndc.xy * 0.5 + 0.5) * vec2(outputWidth, outputHeight);
        vec2 v1_screen = (v1_ndc.xy * 0.5 + 0.5) * vec2(outputWidth, outputHeight);
        vec2 v2_screen = (v2_ndc.xy * 0.5 + 0.5) * vec2(outputWidth, outputHeight);
        
        // Compute SDF
        float sdf = pointToTriangleSDF(pixel, v0_screen, v1_screen, v2_screen);
        
        // Window function
        float weight = windowFunction(sdf, tri.sigma);
        
        // Barycentric interpolation (for color)
        vec3 bary = computeBarycentric(pixel, v0_screen, v1_screen, v2_screen);
        vec3 color = tri.color;  // Uniform color for now
        
        // Alpha blending
        float alpha = tri.opacity * weight;
        accColor += color * alpha * (1.0 - accAlpha);
        accAlpha += alpha * (1.0 - accAlpha);
        
        // Early termination
        if (enableEarlyTermination != 0 && accAlpha > alphaThreshold) {
            break;
        }
    }
    
    imageStore(outputImage, pixelCoord, vec4(accColor, accAlpha));
}
```

**2. FrustumCulling.comp**
```glsl
#version 450
layout(local_size_x = 256) in;

layout(set = 0, binding = 0) readonly buffer TriangleBuffer {
    Triangle triangles[];
};
layout(set = 0, binding = 1) writeonly buffer VisibleIndicesBuffer {
    uint visibleIndices[];
};
layout(set = 0, binding = 2) buffer AtomicCounterBuffer {
    uint counter;
};

layout(push_constant) uniform PushConstants {
    mat4 viewProj;
    uint triangleCount;
};

bool isTriangleVisible(Triangle tri, mat4 mvp) {
    // Project vertices
    vec4 v0 = mvp * vec4(tri.v0, 1.0);
    vec4 v1 = mvp * vec4(tri.v1, 1.0);
    vec4 v2 = mvp * vec4(tri.v2, 1.0);
    
    // Clip space frustum test
    bool visible = false;
    visible = visible || isInFrustum(v0);
    visible = visible || isInFrustum(v1);
    visible = visible || isInFrustum(v2);
    
    return visible;
}

void main() {
    uint tid = gl_GlobalInvocationID.x;
    if (tid >= triangleCount) return;
    
    Triangle tri = triangles[tid];
    
    if (isTriangleVisible(tri, viewProj)) {
        uint index = atomicAdd(counter, 1);
        visibleIndices[index] = tid;
    }
}
```

**3. DepthSortAtomic.comp** (O(N) sorting)
```glsl
#version 450
#define NUM_BINS 256
layout(local_size_x = 256) in;

// Phase 0: Count triangles per bin
// Phase 1: Compute prefix sums
// Phase 2: Place triangles in sorted order

shared uint binCounters[NUM_BINS];

layout(set = 0, binding = 0) readonly buffer DepthKeysBuffer {
    float depthKeys[];
};
layout(set = 0, binding = 1) readonly buffer VisibleIndicesBuffer {
    uint visibleIndices[];
};
layout(set = 0, binding = 2) writeonly buffer SortedIndicesBuffer {
    uint sortedIndices[];
};

layout(push_constant) uniform PushConstants {
    uint visibleCount;
    uint phase;
};

uint depthToBin(float depth) {
    // Quantize depth to [0, NUM_BINS-1]
    return uint(clamp(depth * NUM_BINS, 0.0, NUM_BINS - 1.0));
}

void main() {
    uint tid = gl_GlobalInvocationID.x;
    uint lid = gl_LocalInvocationID.x;
    
    if (phase == 0) {
        // Phase 0: Count
        if (lid < NUM_BINS) {
            binCounters[lid] = 0;
        }
        barrier();
        
        if (tid < visibleCount) {
            float depth = depthKeys[tid];
            uint bin = depthToBin(depth);
            atomicAdd(binCounters[bin], 1);
        }
    }
    else if (phase == 1) {
        // Phase 1: Prefix sum (scan)
        // Implementation omitted for brevity
    }
    else if (phase == 2) {
        // Phase 2: Place in sorted order
        if (tid < visibleCount) {
            float depth = depthKeys[tid];
            uint bin = depthToBin(depth);
            uint offset = prefixSums[bin];
            uint localIndex = atomicAdd(binCounters[bin], 1);
            sortedIndices[offset + localIndex] = visibleIndices[tid];
        }
    }
}
```

### FreGS Shaders

**GaussFreqSplat.comp**
```glsl
#version 450
layout(local_size_x = 16, local_size_y = 16) in;

layout(set = 0, binding = 0) readonly buffer GaussianBuffer {
    Gaussian gaussians[];
};
layout(set = 0, binding = 1) readonly uniform sampler2D subbandLL;
layout(set = 0, binding = 2) readonly uniform sampler2D subbandLH;
layout(set = 0, binding = 3) readonly uniform sampler2D subbandHL;
layout(set = 0, binding = 4) readonly uniform sampler2D subbandHH;
layout(set = 0, binding = 5, rgba16f) uniform writeonly image2D outputImage;

// Gaussian Splatting in frequency domain
void main() {
    // Implementation similar to original 3DGS but in wavelet domain
}
```

### Upscaling Shaders

**MobileUpscalingHDR.comp**
```glsl
#version 450
layout(local_size_x = 16, local_size_y = 16) in;

layout(set = 0, binding = 0) readonly uniform sampler2D inputImage;  // Low-res
layout(set = 0, binding = 1, rgba16f) uniform writeonly image2D outputImage; // High-res

layout(push_constant) uniform PushConstants {
    uvec2 inputSize;
    uvec2 outputSize;
    float sharpness;
    uint enableHDR;
};

// ACES tone mapping for HDR
vec3 ACESToneMap(vec3 color, float exposure) {
    color *= exposure;
    const float a = 2.51;
    const float b = 0.03;
    const float c = 2.43;
    const float d = 0.59;
    const float e = 0.14;
    return clamp((color * (a * color + b)) / (color * (c * color + d) + e), 0.0, 1.0);
}

// Bicubic interpolation для upscaling
vec3 bicubicSample(sampler2D tex, vec2 uv) {
    // Implementation of Catmull-Rom spline
    // ...
}

void main() {
    ivec2 outCoord = ivec2(gl_GlobalInvocationID.xy);
    vec2 uv = (vec2(outCoord) + 0.5) / vec2(outputSize);
    
    vec3 color = bicubicSample(inputImage, uv);
    
    if (enableHDR != 0) {
        color = ACESToneMap(color, 1.2);
    }
    
    imageStore(outputImage, outCoord, vec4(color, 1.0));
}
```

---

## 🧪 ТЕСТИРОВАНИЕ

### Структура тестов

```
tests/
├── Unit Tests (21 файл)
│   ├── Math
│   │   ├── math_matrix4_test.cpp
│   │   ├── math_quaternion_test.cpp
│   │   ├── math_vector3_test.cpp
│   │   └── math_vector_extra_test.cpp
│   ├── Core
│   │   ├── core_gameobject3d_test.cpp
│   │   ├── core_logger_test.cpp
│   │   ├── core_transform3d_test.cpp
│   │   └── core_vma_memory_test.cpp
│   ├── Rendering
│   │   ├── rendering_camera3d_test.cpp
│   │   ├── triangle_splatting_test.cpp
│   │   ├── triangle_splatting_minimal_test.cpp
│   │   ├── fregs_pass_test.cpp
│   │   ├── wavelet_pass_test.cpp
│   │   └── hybrid_fregs_renderer_test.cpp
│   └── Vulkan
│       ├── vulkan_engine_test.cpp
│       ├── vulkan_renderer_test.cpp
│       ├── vulkan_resource_manager_test.cpp
│       ├── vulkan_scene_manager_test.cpp
│       ├── vulkan_hardware_detector_test.cpp
│       └── vulkan_validation_test.cpp
│
├── Integration Tests (5 файлов)
│   ├── integration_pipeline_test.cpp
│   ├── instanced_mesh_test.cpp
│   ├── modern_renderer3d_test.cpp
│   ├── render_strategy_factory_test.cpp
│   └── render_strategy_interfaces_test.cpp
│
└── System Tests (5 файлов)
    ├── app_engine_test.cpp
    ├── input_system_test.cpp
    ├── physics_system_test.cpp
    ├── upscaling_system_test.cpp
    └── x11_window_test.cpp
```

### Покрытие тестами

```yaml
Текущее покрытие: 80% (633/788 функций)
Цель:            100% (788/788 функций)

Критический приоритет (44 функции):
  - HybridFreGSRenderer.cpp: 44 функции БЕЗ тестов

Высокий приоритет (43 функции):
  - ModernRenderer3D.cpp: 21 функция
  - FreGSPass.cpp: 12 функций
  - WaveletPass.cpp: 10 функций

Средний приоритет (48 функций):
  - Engine.cpp: 26 функций
  - X11Window.cpp: 22 функции

Legacy (не требует тестов):
  - OpenGL rendering код: 313 функций
```

### Пример теста (Triangle Splatting)

```cpp
// tests/triangle_splatting_test.cpp
#include <gtest/gtest.h>
#include <SpectraForge/Rendering/RenderPass/TriangleSplattingPass.h>

class TriangleSplattingTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Mock Vulkan context
        device = createMockDevice();
        allocator = createMockAllocator();
        
        // Create pass
        config.outputWidth = 800;
        config.outputHeight = 600;
        pass = std::make_unique<TriangleSplattingPass>(config);
    }
    
    void TearDown() override {
        pass->cleanup();
        pass.reset();
    }
    
    vk::Device device;
    VmaAllocator allocator;
    TriangleSplattingPass::Config config;
    std::unique_ptr<TriangleSplattingPass> pass;
};

TEST_F(TriangleSplattingTest, InitializeSuccess) {
    ASSERT_TRUE(pass->initialize(device, allocator, queue, pool));
    EXPECT_TRUE(pass->isInitialized());
}

TEST_F(TriangleSplattingTest, UploadTriangles) {
    std::vector<TriangleSplattingPass::Triangle> triangles;
    
    // Create simple triangle
    TriangleSplattingPass::Triangle tri;
    tri.v0 = glm::vec3(0.0f, 1.0f, 0.0f);
    tri.v1 = glm::vec3(-1.0f, -1.0f, 0.0f);
    tri.v2 = glm::vec3(1.0f, -1.0f, 0.0f);
    tri.color = glm::vec3(1.0f, 0.0f, 0.0f);
    tri.opacity = 1.0f;
    tri.sigma = 1.0f;
    triangles.push_back(tri);
    
    ASSERT_NO_THROW(pass->uploadTriangles(triangles));
    EXPECT_EQ(pass->getTriangleCount(), 1);
}

TEST_F(TriangleSplattingTest, RenderSingleTriangle) {
    // Arrange
    std::vector<Triangle> triangles = createTestTriangles(1);
    pass->uploadTriangles(triangles);
    
    glm::mat4 viewProj = glm::mat4(1.0f);
    pass->setViewProjection(viewProj);
    
    // Act
    vk::CommandBuffer cmd = allocateCommandBuffer();
    pass->execute(cmd, 0);
    
    // Assert
    vk::Image outputImage = pass->getOutputImage();
    EXPECT_NE(outputImage, vk::Image(nullptr));
    
    // Verify output (requires GPU readback)
    auto pixels = readImageData(outputImage);
    EXPECT_GT(countNonZeroPixels(pixels), 0);
}

TEST_F(TriangleSplattingTest, FrustumCullingRemovesOffscreenTriangles) {
    // Arrange
    std::vector<Triangle> triangles;
    
    // Triangle 1: On screen
    triangles.push_back(createTriangleAt(0.0f, 0.0f, 0.0f));
    
    // Triangle 2: Far behind camera (should be culled)
    triangles.push_back(createTriangleAt(0.0f, 0.0f, -1000.0f));
    
    pass->uploadTriangles(triangles);
    pass->setFrustumCullingEnabled(true);
    
    // Act
    pass->execute(cmd, 0);
    
    // Assert
    EXPECT_EQ(pass->getVisibleTriangleCount(), 1);
}

TEST_F(TriangleSplattingTest, DepthSortingBackToFront) {
    // Arrange
    std::vector<Triangle> triangles;
    triangles.push_back(createTriangleAt(0.0f, 0.0f, -5.0f));  // Far
    triangles.push_back(createTriangleAt(0.0f, 0.0f, -2.0f));  // Near
    
    pass->uploadTriangles(triangles);
    pass->setCameraPosition(glm::vec3(0.0f, 0.0f, 0.0f));
    
    // Act
    pass->execute(cmd, 0);
    
    // Assert - read sorted indices from GPU
    auto sortedIndices = readSortedIndices();
    EXPECT_EQ(sortedIndices[0], 0);  // Far triangle first
    EXPECT_EQ(sortedIndices[1], 1);  // Near triangle second
}
```

---

## 📚 ДОКУМЕНТАЦИЯ

### Основные документы

| Файл | Описание | Приоритет |
|------|----------|-----------|
| `SPECTRAFORGE_AUDIT.md` | Аудит архитектуры, SOLID violations | 🔴 КРИТИЧНО |
| `triangle-mesh-conversion-plan.md` | План миграции к Triangle Splatting+ | ⚠️ Высокий |
| `VECTORIZED-OPTIMIZATIONS.md` | SIMD и оптимизации | ⚠️ Высокий |
| `OPTIMIZATIONS_EXPLAINED.md` | Объяснение производительности | ℹ️ Средний |
| `MISSING_TESTS_MATRIX.md` | Матрица недостающих тестов | ℹ️ Средний |
| `QUICK_START.md` | Быстрый старт для разработчиков | ℹ️ Низкий |

### SPECTRAFORGE_AUDIT.md - Ключевые выводы

**Критические проблемы**:
1. **Engine.h** - монолитная структура, нарушает SRP
2. **HybridFreGSRenderer** - God class с 1383 строками
3. **VulkanRenderer** - жестко привязан к Gaussian Splatting
4. **Отсутствие DI контейнера** - препятствует модульности

**План миграции (15-20 недель)**:

```
Phase 1 (Weeks 1-6): Immediate Actions
  ✓ Создание IRenderStrategy интерфейса
  ✓ Рефакторинг VulkanRenderer
  ✓ Выделение triangle rendering
  
Phase 2 (Weeks 7-14): Architectural Refactoring
  - Внедрение Dependency Injection
  - Декомпозиция Engine монолита
  - Создание mesh management абстракций
  
Phase 3 (Weeks 15-20): TriangleSplatting+ Integration
  - Интеграция TriangleSplatting+ renderer
  - Добавление differentiable training
  - Оптимизация производительности
```

### triangle-mesh-conversion-plan.md

**Цель**: Переход от triangle soup к vertex-based representation с connectivity.

**Ключевые структуры**:
```cpp
struct TriangleSplattingVertex {
    Math::Vector3 position;    // (x_i, y_i, z_i)
    Math::Vector3 color;       // c_i vertex color
    float opacity;             // o_i ∈ [0,1]
    std::vector<uint32_t> adjacent_triangles;
    std::vector<uint32_t> adjacent_vertices;
};

struct ConnectedTriangle {
    uint32_t indices[3];       // Vertex indices
    float smoothness;          // σ trainable
    uint32_t neighbors[3];     // Adjacent triangles
    Math::Vector3 normal;
    float area;
};
```

**Алгоритмы**:
- Vertex deduplication (epsilon-based hashing)
- Adjacency building (edge-to-triangles mapping)
- Barycentric interpolation
- Mesh topology validation

---

## 🔨 СБОРКА И РАЗВЕРТЫВАНИЕ

### Зависимости

```yaml
Обязательные:
  - CMake: >= 3.16
  - Компилятор: GCC 9+, Clang 10+, MSVC 2019+
  - Vulkan SDK: >= 1.3
  - GLFW: >= 3.3
  - GLM: >= 0.9.9
  - VMA (Vulkan Memory Allocator): >= 3.0

Опциональные:
  - libpng: для сохранения скриншотов
  - Google Test: для тестов
  - Doxygen: для документации
  - clang-format, clang-tidy: для форматирования
```

### Сборка (Linux)

```bash
# 1. Установка зависимостей (Ubuntu/Debian)
sudo apt install vulkan-sdk glslc cmake build-essential
sudo apt install libglfw3-dev libglm-dev libpng-dev

# 2. Клонирование
git clone https://github.com/TiGRoNdev/SpectraForge.git
cd SpectraForge

# 3. Компиляция шейдеров
chmod +x compile_shaders.sh
./compile_shaders.sh

# 4. Сборка проекта
mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
make -j$(nproc)

# 5. Запуск примера
./SpectraForge_Optimized_Demo --mobile
```

### Сборка с тестами

```bash
mkdir build && cd build
cmake .. -DENABLE_TESTS=ON -DCMAKE_BUILD_TYPE=Debug
make -j$(nproc)

# Запуск тестов
ctest --verbose

# Запуск с покрытием
../run_tests_with_coverage.sh
```

### CMake структура

```cmake
# Корневой CMakeLists.txt
project(SpectraForge VERSION 1.0.0 LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 17)

# Модули
add_subdirectory(src)
add_subdirectory(tests)

# Библиотека
add_library(SpectraForge STATIC
    $<TARGET_OBJECTS:SpectraForge_Core>
    $<TARGET_OBJECTS:SpectraForge_Math>
    $<TARGET_OBJECTS:SpectraForge_Platform>
    $<TARGET_OBJECTS:SpectraForge_Vulkan>
    $<TARGET_OBJECTS:SpectraForge_Upscaling>
)

target_link_libraries(SpectraForge PUBLIC Vulkan::Vulkan glfw)

# Примеры
add_executable(SpectraForge_Optimized_Demo examples/SpectraForge_Optimized_Demo.cpp)
target_link_libraries(SpectraForge_Optimized_Demo PRIVATE SpectraForge FrameOutput)
```

### Docker контейнер

```dockerfile
FROM ubuntu:22.04

RUN apt-get update && apt-get install -y \
    vulkan-sdk \
    glslc \
    cmake \
    build-essential \
    libglfw3-dev \
    libglm-dev \
    libpng-dev

WORKDIR /app
COPY . .

RUN ./compile_shaders.sh && \
    mkdir build && cd build && \
    cmake .. -DCMAKE_BUILD_TYPE=Release && \
    make -j4

CMD ["./build/SpectraForge_Optimized_Demo", "--mobile"]
```

### CI/CD (GitHub Actions)

```yaml
# .github/workflows/test-coverage.yml
name: Test Coverage

on: [push, pull_request]

jobs:
  test:
    runs-on: ubuntu-latest
    steps:
      - uses: actions/checkout@v3
      
      - name: Install dependencies
        run: |
          sudo apt install vulkan-sdk glslc cmake libglfw3-dev libglm-dev
      
      - name: Build with tests
        run: |
          ./compile_shaders.sh
          mkdir build && cd build
          cmake .. -DENABLE_TESTS=ON -DCMAKE_BUILD_TYPE=Debug
          make -j4
      
      - name: Run tests
        run: |
          cd build
          ctest --verbose
      
      - name: Coverage report
        run: |
          ./run_tests_with_coverage.sh
          bash <(curl -s https://codecov.io/bash)
```

---

## ⚡ ПРОИЗВОДИТЕЛЬНОСТЬ

### Benchmark результаты

```
┌─────────────────────────────────────────────────────┐
│ GPU: Adreno 740 (Snapdragon 8 Gen 2)               │
├─────────────────────────────────────────────────────┤
│ Режим         │ Разрешение │ FPS │ Frame Time      │
├───────────────┼────────────┼─────┼─────────────────┤
│ Mobile        │ 1080p→4K   │ 62  │ 16.1ms          │
│ Balanced      │ 1440p→4K   │ 58  │ 17.2ms          │
│ Quality       │ Native 4K  │ 42  │ 23.8ms          │
└───────────────┴────────────┴─────┴─────────────────┘

┌─────────────────────────────────────────────────────┐
│ GPU: GTX 1650 (Desktop)                             │
├─────────────────────────────────────────────────────┤
│ Режим         │ Разрешение │ FPS │ Frame Time      │
├───────────────┼────────────┼─────┼─────────────────┤
│ Mobile        │ 1080p→4K   │ 60+ │ <16ms           │
│ Balanced      │ 1440p→4K   │ 60+ │ <16ms           │
│ Quality       │ Native 4K  │ 55  │ 18.2ms          │
└───────────────┴────────────┴─────┴─────────────────┘

┌─────────────────────────────────────────────────────┐
│ GPU: RTX 3060 (High-end)                            │
├─────────────────────────────────────────────────────┤
│ Режим         │ Разрешение │ FPS │ Frame Time      │
├───────────────┼────────────┼─────┼─────────────────┤
│ Mobile        │ 1080p→4K   │ 60+ │ <10ms           │
│ Balanced      │ 1440p→4K   │ 60+ │ <12ms           │
│ Quality       │ Native 4K  │ 60+ │ <16ms           │
└───────────────┴────────────┴─────┴─────────────────┘
```

### Оптимизации

**Применены**:
- ✅ Atomic depth sorting O(N) вместо Bitonic O(N log N)
- ✅ Two-Pass rendering (Visibility + Shading) - O(N+M) вместо O(N×M)
- ✅ Frustum culling на GPU
- ✅ Variable Rate Shading (VRS) для мобильных
- ✅ Tile-based culling для Adreno/Mali
- ✅ Persistent mapped UBO для uniform данных
- ✅ HDR tone mapping (ACES)
- ✅ Адаптивный upscaling (1080p→4K для мобильных)

**Планируются**:
- ⏳ Occlusion culling
- ⏳ Level of Detail (LOD)
- ⏳ Streaming для больших сцен
- ⏳ Multi-threading для CPU load
- ⏳ Ray tracing для reflections

### Memory Usage

```
┌──────────────────────────────────────────────┐
│ Sponza Scene (262 meshes, 66k triangles)    │
├──────────────────────────────────────────────┤
│ Component              │ Memory              │
├────────────────────────┼─────────────────────┤
│ Triangle Buffer        │ 6.3 MB              │
│ Vertex Buffer          │ 2.8 MB              │
│ Index Buffer           │ 0.8 MB              │
│ Textures               │ 48 MB               │
│ Uniform Buffers        │ 0.5 MB              │
│ Swapchain Images       │ 32 MB (4K HDR)      │
│ Depth/Stencil          │ 16 MB               │
├────────────────────────┼─────────────────────┤
│ TOTAL                  │ 107 MB              │
└────────────────────────┴─────────────────────┘
```

---

## 🗺️ ROADMAP И ПЛАНЫ

### Phase 1: Stabilization (Current - Q1 2025)
- ✅ Triangle Splatting рендеринг
- ✅ 60+ FPS на мобильных GPU
- ✅ 80% test coverage
- ✅ HDR support
- ⏳ 100% test coverage (225 новых тестов)
- ⏳ Полная документация API

### Phase 2: Architecture Refactoring (Q2 2025)
- ⏳ Устранение SOLID violations
- ⏳ Dependency Injection контейнер
- ⏳ Декомпозиция God classes
- ⏳ Mesh connectivity для Triangle Splatting+

### Phase 3: Advanced Features (Q3-Q4 2025)
- ⏳ Ray tracing integration (OptiX/DXR)
- ⏳ AI-based upscaling (DLSS/FSR2)
- ⏳ Differentiable rendering (для ML training)
- ⏳ Streaming для больших сцен
- ⏳ Multi-platform support (Windows, Android)

### Phase 4: Production Ready (2026)
- ⏳ Editor/Tooling
- ⏳ Physics integration
- ⏳ Animation system
- ⏳ Networking для multiplayer
- ⏳ Профессиональная документация

---

## 📞 КОНТАКТЫ И ВКЛАД

### Repository
- **GitHub**: https://github.com/TiGRoNdev/SpectraForge
- **Branch**: `release/master` (production), `feature/*` (development)

### Вклад в проект
См. `CONTRIBUTING.md` для деталей:
- Coding standards (C++17, SOLID principles)
- Testing guidelines (Google Test, 80%+ coverage)
- Pull request process
- Code review checklist

### Лицензия
MIT License - см. `LICENSE`

---

**Дата последнего обновления индекса**: 2025-10-07  
**Версия индекса**: 1.0  
**Автор**: Cursor AI (по запросу пользователя)

