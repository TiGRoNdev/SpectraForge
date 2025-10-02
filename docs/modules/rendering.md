# Rendering Module: Hybrid DWT + FreGS Pipeline

**Версия:** 2.0  
**Дата:** 2025-10-02  
**Статус:** В разработке

## Обзор

Модуль рендеринга реализует гибридный пайплайн **Hybrid DWT + FreGS** (Wavelet Lifting + Frequency-Encoded Gaussian Splatting) для достижения 8K @ ≥500 FPS на мобильных GPU с энергопотреблением ≤5W.

### Ключевые преимущества над FreqVox

| Аспект | FreqVox | Hybrid DWT + FreGS |
|--------|---------|-------------------|
| Сложность трансформ | O(N log N) FFT/DCT | O(N) Wavelet Lifting |
| Сложность сплаттинга | O(M log M + P) | O(M + P) |
| Память | Плотные SH + DCT буферы | Разреженные wavelet subbands fp16 |
| FPS @ 8K | ~300 | ~500 |
| Зависимости | VkFFT, внешние библиотеки | Только Vulkan 1.3 Subgroups |

## Архитектура

### 1. Render Passes

```
Input Image (8K rgba16f)
      ↓
[WaveletPass] → Wavelet Lifting (Daubechies-4)
      ↓ (subbands: LL, LH, HL, HH @ fp16)
[FreGSPass] → Frequency Gaussian Splatting
      ↓ (accumulated image)
[FoveationStage] → Foveation Alignment (optional)
      ↓
[TemporalReprojection] → Temporal AA (optional)
      ↓
[Upscaler] → DLSS/FSR2 (optional)
      ↓
Output Image (8K rgba16f)
```

### 2. Классы и интерфейсы (SOLID)

#### IRenderPass (ISP: минимальный интерфейс)

```cpp
class IRenderPass {
public:
    virtual bool initialize(const VulkanContext&) = 0;
    virtual void execute(vk::CommandBuffer, uint32_t frameIndex) = 0;
    virtual void cleanup() = 0;
    virtual const char* getName() const = 0;
};
```

#### WaveletPass (SRP: только wavelet декомпозиция)

```cpp
class WaveletPass : public RenderPassBase {
    // Input: Image (rgba16f)
    // Output: 4 subbands (LL, LH, HL, HH @ rg16f)
    // Shader: WaveletLifting.comp
    // Complexity: O(N), ~0.9ms @ 8K
};
```

#### FreGSPass (SRP: только частотная аккумуляция)

```cpp
class FreGSPass : public RenderPassBase {
    // Input: WaveletSubbands + GaussianSplats
    // Output: Accumulated image (rgba16f)
    // Shader: GaussFreqSplat.comp
    // Complexity: O(M+P), ~0.8ms @ 8K
};
```

### 3. Фовеация

Естественная поддержка через wavelet subbands:
- **Фовеа (центральные 10°):** Полный спектр (LL + LH + HL + HH)
- **Периферия:** Только LL (75% экономия операций)

### 4. Temporal Reprojection

- Фовеа: полноспектральная репроекция
- Периферия: low-pass фильтр
- Velocity/depth thresholding для статичных областей

## Использование

### Базовый пример

```cpp
#include <SpectraForge/rendering/WaveletPass.h>
#include <SpectraForge/rendering/FreGSPass.h>

// 1. Создание passes
WaveletPassConfig waveletConfig;
waveletConfig.inputWidth = 3840;
waveletConfig.inputHeight = 2160;
waveletConfig.threshold = 0.01f;

auto waveletPass = std::make_unique<WaveletPass>(waveletConfig);
waveletPass->initialize(vulkanContext);

FreGSPassConfig fregsConfig;
fregsConfig.outputWidth = 3840;
fregsConfig.outputHeight = 2160;

auto fregsPass = std::make_unique<FreGSPass>(fregsConfig);
fregsPass->initialize(vulkanContext);

// 2. Execution loop
vk::CommandBuffer cmd = ...;
uint32_t frameIndex = ...;

// Wavelet decomposition
waveletPass->setInputImage(inputImage, inputView);
waveletPass->execute(cmd, frameIndex);

// Gaussian splatting
fregsPass->setInputSubbands(waveletPass->getSubbands());
fregsPass->uploadGaussians(myGaussians);
fregsPass->execute(cmd, frameIndex);

// Result
vk::ImageView result = fregsPass->getOutputView();
```

### Интеграция с фовеацией (eye tracking)

```cpp
// Обновление foveation от eye tracker
glm::vec2 gazePoint = eyeTracker->getGazePoint();  // Normalized [0, 1]

waveletPass->updateConfig({
    .foveationLevel = 1  // 0 = full, 1 = peripheral reduction
});

fregsPass->updateFoveation(gazePoint, 0.1f);  // radius = 0.1 (10%)
```

## Требования Vulkan

### Обязательные features

```cpp
VkPhysicalDeviceVulkan13Features features13{};
features13.subgroupSizeControl = VK_TRUE;

VkPhysicalDeviceSubgroupProperties subgroupProps{};
// Требуется: subgroupSupportedStages & VK_SHADER_STAGE_COMPUTE_BIT
// Требуется: subgroupSupportedOperations & VK_SUBGROUP_FEATURE_ARITHMETIC_BIT
// Опционально: shaderSubgroupExtendedTypes для fp16 subgroup ops
```

### Расширения

- `VK_KHR_shader_subgroup_basic` (core в 1.3)
- `VK_KHR_shader_subgroup_arithmetic` (core в 1.3)
- `VK_EXT_subgroup_size_control` (опционально, для оптимизации)
- `VK_EXT_shader_subgroup_extended_types_float16` (для fp16 в subgroup ops)

## Производительность

### Целевые метрики

- **Разрешение:** 8K per eye (7680×4320)
- **Frame Rate:** ≥500 FPS
- **Frame Time:** ≤2ms
- **Энергопотребление:** ≤5W (Adreno 650, Mali-G77)

### Профилирование

```cpp
PassStatistics stats = waveletPass->getStatistics();
std::cout << "Execution time: " << stats.executionTimeMs << "ms\n";
std::cout << "Memory used: " << stats.memoryUsedBytes / 1024 / 1024 << "MB\n";
```

### Breakdown (типичный)

| Pass | Time @ 8K | GPU Util |
|------|-----------|----------|
| WaveletLifting | 0.9ms | 85% |
| GaussFreqSplat | 0.8ms | 90% |
| Foveation | 0.1ms | 40% |
| Temporal Reproj | 0.2ms | 60% |
| **Total** | **2.0ms** | **82%** |

## Тестирование

### Unit тесты

```bash
./build/tests/unit/WaveletPass_Test
./build/tests/unit/FreGSPass_Test
```

Покрытие: ≥80% (AAA pattern, GoogleTest)

### Integration тесты

```bash
./build/tests/integration/HybridPipeline_Test
```

Golden images для верификации качества.

## См. также

- [Renderer.md](../architecture/Renderer.md) - Математическое обоснование
- [WaveletLifting.comp](../../shaders/WaveletLifting.comp) - GLSL shader
- [GaussFreqSplat.comp](../../shaders/GaussFreqSplat.comp) - GLSL shader
- [Upscaling.md](upscaling.md) - DLSS/FSR2 интеграция
- [FreqVox Legacy](../legacy/freqvox/README.md) - Предыдущий подход

