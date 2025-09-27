# Vulkan-Based Hybrid 3D Rendering Engine Architecture

## Обзор

Данный документ описывает новую архитектуру HyperEngine, основанную на Vulkan API и реализующую современные техники рендеринга:

- **FlashGS** - CUDA-ускоренный 3D Gaussian Splatting
- **OptiX Ray Tracing** - высокопроизводительный ray tracing для вторичных эффектов
- **AI Denoising** - деноизинг с использованием OptiX AI
- **Hybrid Upscaling** - поддержка DLSS и FSR

## Архитектурная диаграмма

```
┌─────────────────┐
│   VulkanEngine  │
└─────────┬───────┘
          │
    ┌─────┴─────┐
    │           │
┌───▼───┐   ┌───▼────┐
│Renderer│   │Scene   │
│        │   │Manager │
└───┬───┘   └────────┘
    │
┌───▼────────────────┐
│  Hardware Detector │
└────────────────────┘
    │
┌───▼─────┐ ┌─────────┐ ┌──────────┐
│FlashGS  │ │OptiX RT │ │Upscaler  │
│Splatter │ │Tracer   │ │(DLSS/FSR)│
└─────────┘ └─────────┘ └──────────┘
```

## Основные компоненты

### 1. VulkanEngine

Главный класс движка, координирующий работу всех подсистем:

```cpp
class VulkanEngine {
public:
    bool init(vk::Instance instance);
    void renderFrame(const CameraParams& params);
    void shutdown();
    
private:
    std::unique_ptr<VulkanRenderer> renderer;
    std::unique_ptr<SceneManager> sceneManager;
    std::unique_ptr<ResourceManager> resourceManager;
    std::unique_ptr<HardwareDetector> hardwareDetector;
};
```

### 2. HardwareDetector

Определяет возможности GPU и выбирает оптимальные пути рендеринга:

```cpp
class HardwareDetector {
public:
    VendorType detectVendor();
    bool supportsRayTracing();
    UpscalerType selectUpscalerPath();
    bool supportsCUDA();
    bool supportsOptiX();
};
```

### 3. ResourceManager

Управляет памятью и ресурсами с поддержкой CUDA-Vulkan interop:

```cpp
class ResourceManager {
public:
    vk::Buffer allocateBuffer(size_t size);
    vk::Image createTexture(const ImageData& data);
    vk::DeviceMemory manageInterop(const CUDAResource& cudaRes);
};
```

### 4. FlashGSSplatter

CUDA-ускоренный 3D Gaussian Splatting:

```cpp
class FlashGSSplatter {
public:
    void optimizeGaussians(const MultiViewImages& images);
    PrimaryImage rasterizeGaussians(const CameraParams& params);
    
private:
    std::unique_ptr<TileBasedRasterizer> rasterizer;
    cudaStream_t cudaStream;
};
```

### 5. OptiXRayTracer

Ray tracing для вторичных эффектов:

```cpp
class OptiXRayTracer {
public:
    void buildAccelerationStructures(const SceneGeometry& geo);
    RawEffects traceRays(const LaunchParams& params);
    void applySER(const CoherencyHints& hints);
    
private:
    OptixDeviceContext context;
    OptixPipeline pipeline;
};
```

### 6. Upscaler System

Абстрактная система upscaling с поддержкой DLSS и FSR:

```cpp
class Upscaler {
public:
    virtual FinalImage upscaleImage(const DenoisedImage& image, 
                                   const ResolutionTarget& target) = 0;
    virtual bool isSupported(const HardwareConfig& config) const = 0;
};

class DLSSUpscaler : public Upscaler {
    void superResolution(const MotionVectors& vectors);
    void rayReconstruction();
    void multiFrameGeneration();
};
```

## Rendering Pipeline

### Этап 1: Primary Rasterization (FlashGS)
1. Оптимизация параметров гауссианов на CUDA
2. Tile-based растеризация
3. Генерация primary image с depth/normal buffers

### Этап 2: Secondary Ray Tracing (OptiX)
1. Построение acceleration structures
2. Трассировка лучей для отражений/теней/GI
3. Применение Shader Execution Reordering
4. Генерация raw effects

### Этап 3: AI Denoising
1. OptiX AI denoiser для ray traced эффектов
2. Использование auxiliary buffers (motion vectors, albedo, normals)
3. Генерация denoised image

### Этап 4: Upscaling
1. Выбор upscaler на основе железа (DLSS/FSR)
2. Temporal upscaling с motion vectors
3. Генерация final image в целевом разрешении

### Этап 5: Presentation
1. Композитинг всех слоев
2. Tone mapping и color grading
3. Презентация через Vulkan swapchain

## Производительность

### Ожидаемые показатели:

| Конфигурация | Разрешение | FPS | Особенности |
|-------------|------------|-----|-------------|
| RTX 4090 | 4K | 60+ | DLSS 3, OptiX RT |
| RTX 3070 | 1440p | 60+ | DLSS 2, OptiX RT |
| RX 7800 XT | 1440p | 45+ | FSR 2, Vulkan RT |
| GTX 1660 Ti | 1080p | 30+ | Fallback paths |

### Оптимизации:

- **GPU-driven rendering** - минимизация CPU-GPU синхронизации
- **Async compute** - параллельное выполнение compute и graphics
- **Memory pooling** - эффективное управление памятью через VMA
- **Shader caching** - кэширование скомпилированных шейдеров

## Совместимость

### Минимальные требования:
- **GPU**: Vulkan 1.3 support
- **VRAM**: 4GB
- **Driver**: Latest stable

### Рекомендуемые требования:
- **GPU**: RTX 20 series / RX 6000 series или новее
- **VRAM**: 8GB+
- **Features**: Ray Tracing support

### Опциональные компоненты:
- **CUDA**: Для FlashGS acceleration
- **OptiX**: Для ray tracing (NVIDIA only)
- **DLSS**: Для NVIDIA upscaling
- **FSR**: Для cross-vendor upscaling

## Сборка

### Зависимости:
```json
{
  "dependencies": [
    "vulkan", "vulkan-hpp", "vulkan-memory-allocator",
    "spirv-cross", "shaderc", "imgui[vulkan-binding]",
    "assimp", "stb", "glfw3", "glm"
  ]
}
```

### CMake опции:
```cmake
option(BUILD_VULKAN_RENDERER "Build Vulkan-based Hybrid Renderer" ON)
option(BUILD_WITH_CUDA "Build with CUDA support" ON)
option(BUILD_WITH_OPTIX "Build with OptiX support" ON)
option(BUILD_WITH_DLSS "Build with DLSS support" OFF)
option(BUILD_WITH_FSR "Build with FSR support" ON)
```

### Сборка:
```bash
mkdir build && cd build
cmake .. -DCMAKE_TOOLCHAIN_FILE=../vcpkg/scripts/buildsystems/vcpkg.cmake
cmake --build . --config Release
```

## Использование

### Базовый пример:
```cpp
#include <Engine3D/Vulkan/VulkanEngine.h>

int main() {
    // Создание Vulkan instance
    vk::Instance instance = createVulkanInstance();
    
    // Инициализация движка
    Engine3D::Vulkan::VulkanEngine engine;
    if (!engine.init(instance)) {
        return -1;
    }
    
    // Главный цикл
    while (running) {
        CameraParams camera = getCameraParams();
        engine.renderFrame(camera);
    }
    
    engine.shutdown();
    return 0;
}
```

## Roadmap

### v0.1.0 (Текущая версия)
- ✅ Базовая Vulkan инфраструктура
- ✅ HardwareDetector
- ✅ ResourceManager с VMA
- 🔄 FlashGS integration
- 🔄 OptiX ray tracing
- 🔄 DLSS/FSR upscaling

### v0.2.0 (Планируется)
- Полная интеграция всех компонентов
- Performance optimizations
- Extensive testing
- Documentation completion

### v1.0.0 (Цель)
- Production-ready release
- Cross-platform support
- Comprehensive examples
- Performance benchmarks

## Вклад в проект

См. [CONTRIBUTING.md](../CONTRIBUTING.md) для информации о том, как внести вклад в проект.

## Лицензия

Проект распространяется под лицензией MIT. См. [LICENSE](../LICENSE) для подробностей.
