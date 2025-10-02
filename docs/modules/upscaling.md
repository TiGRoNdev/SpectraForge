# Upscaling Module: DLSS / FSR2 Abstraction

**Версия:** 2.0  
**Дата:** 2025-10-02  
**Статус:** В разработке

## Обзор

Модуль upscaling предоставляет единый интерфейс для AI-апскейлинга (NVIDIA DLSS, AMD FSR2) с автоматическим выбором оптимальной технологии на основе GPU vendor.

## Архитектура (SOLID)

### IUpscaler (ISP: минимальный интерфейс)

```cpp
class IUpscaler {
public:
    virtual bool initialize(...) = 0;
    virtual void execute(vk::CommandBuffer, const UpscaleResources&, ...) = 0;
    virtual void cleanup() = 0;
    virtual bool isSupported() const = 0;
};
```

### UpscalerFactory (DIP: возвращает абстракции)

```cpp
enum class UpscalerType {
    AUTO,    // Автоопределение по GPU vendor
    DLSS,    // NVIDIA Streamline DLSS
    FSR2,    // AMD FidelityFX FSR2
    NONE     // Passthrough (для тестирования)
};

auto upscaler = UpscalerFactory::createUpscaler(
    UpscalerType::AUTO,
    gpuVendorID
);
```

## Реализации

### 1. DLSS (NVIDIA RTX)

**Требования:**
- NVIDIA RTX GPU (Tensor Cores)
- Streamline SDK (включен в vcpkg)
- Vulkan extension: `VK_KHR_timeline_semaphore`

**Preset Quality:**

| Качество | Input Scale | Perf Gain | Use Case |
|----------|-------------|-----------|----------|
| Performance | 0.5x | 2-4x FPS | Максимальная производительность |
| Balanced | 0.67x | 1.7-2.3x FPS | Баланс |
| Quality | 0.75x | 1.5-2x FPS | Высокое качество |
| Ultra Quality | 0.88x | 1.3x FPS | Максимальное качество |

**Пример:**

```cpp
#include <SpectraForge/upscaling/DLSSUpscaler.h>

UpscaleConfig config;
config.inputWidth = 1920;   // Native render resolution
config.inputHeight = 1080;
config.outputWidth = 3840;  // Display resolution
config.outputHeight = 2160;
config.quality = UpscaleQuality::BALANCED;

auto dlss = std::make_unique<DLSSUpscaler>();
if (!dlss->isSupported()) {
    // Fallback to FSR2
}

dlss->initialize(instance, physicalDevice, device, config);

// Per frame
UpscaleResources resources;
resources.inputColor = renderTarget;
resources.inputDepth = depthBuffer;
resources.inputMotionVectors = motionVectors;
resources.outputColor = presentImage;

float jitterX, jitterY;
dlss->getJitterOffset(frameIndex, jitterX, jitterY);

dlss->execute(cmd, resources, frameIndex, jitterX, jitterY);
```

### 2. FSR2 (Cross-Vendor)

**Требования:**
- Любой Vulkan 1.3 GPU
- FidelityFX FSR2 SDK (включен в vcpkg)

**Преимущества:**
- Кросс-вендорная поддержка (NVIDIA, AMD, Intel, mobile)
- Открытый исходный код
- Хорошее качество при ~2x FPS boost

**Ограничения:**
- Нет Tensor Cores оптимизации
- Немного ниже качество чем DLSS на RTX

**Пример:**

```cpp
#include <SpectraForge/upscaling/FSR2Upscaler.h>

auto fsr2 = std::make_unique<FSR2Upscaler>();
// FSR2 всегда доступен
fsr2->initialize(instance, physicalDevice, device, config);
fsr2->execute(cmd, resources, frameIndex, jitterX, jitterY);
```

### 3. AUTO Mode (Рекомендуется)

```cpp
// Автоматический выбор лучшей технологии
auto upscaler = UpscalerFactory::createUpscaler(
    UpscalerType::AUTO,
    physicalDeviceProperties.vendorID
);

// Логика выбора:
// - NVIDIA GPU + RTX → DLSS
// - AMD/Intel/Mobile → FSR2
```

## Jitter и TAA

Для корректной работы upscaler'ов требуется jitter (TAA):

```cpp
float jitterX, jitterY;
upscaler->getJitterOffset(frameIndex, jitterX, jitterY);

// Применить к projection matrix
projectionMatrix[2][0] += jitterX / viewportWidth;
projectionMatrix[2][1] += jitterY / viewportHeight;
```

## CMake Опции

```bash
cmake -DSPECTRAFORGE_UPSCALER=AUTO ..   # Автовыбор (по умолчанию)
cmake -DSPECTRAFORGE_UPSCALER=DLSS ..   # Только DLSS
cmake -DSPECTRAFORGE_UPSCALER=FSR2 ..   # Только FSR2
cmake -DSPECTRAFORGE_UPSCALER=NONE ..   # Без upscaling
```

## Зависимости

### vcpkg.json

```json
{
  "dependencies": [
    "streamline",          // DLSS (NVIDIA)
    "fidelityfx-fsr2"      // FSR2 (AMD)
  ]
}
```

### Runtime DLL/SO

**Windows:**
- `sl.interposer.dll` (Streamline для DLSS)
- `ffx_fsr2_api_x64.dll` (FSR2)

**Linux:**
- `libsl.interposer.so`
- `libffx_fsr2_api_x64.so`

## Производительность

### Целевые метрики @ 8K

| Config | Native Res | Upscale to | FPS (без upscale) | FPS (с upscale) | Gain |
|--------|------------|------------|-------------------|-----------------|------|
| Performance | 3840×2160 (4K) | 7680×4320 (8K) | 250 FPS | 500+ FPS | ~2x |
| Balanced | 5120×2880 | 7680×4320 (8K) | 200 FPS | 400 FPS | ~2x |

## Тестирование

### Unit тесты

```bash
./build/tests/unit/Upscaler_Test
./build/tests/unit/DLSSUpscaler_Test
./build/tests/unit/FSR2Upscaler_Test
```

### Integration тесты

```bash
./build/tests/integration/UpscalingPipeline_Test
```

Верификация через PSNR/SSIM с reference frames.

## Troubleshooting

### DLSS не инициализируется

1. Проверить наличие RTX GPU:
```cpp
if (!DLSSUpscaler::isDLSSAvailable(physicalDevice)) {
    // Fallback to FSR2
}
```

2. Проверить наличие `sl.interposer.dll`:
```bash
ls build/Release/sl.interposer.dll
```

3. Проверить логи Streamline:
```cpp
// Включить debug логи
export SL_LOG_LEVEL=2
```

### FSR2 артефакты

1. Проверить motion vectors:
   - Формат: RG16_SFLOAT, normalized [-1, 1]
   - Направление: screen-space velocity

2. Настроить sharpening:
```cpp
config.sharpness = 0.3f;  // Уменьшить для мягкости
```

## См. также

- [Rendering.md](rendering.md) - Интеграция с рендер-пайплайном
- [DLSS Integration Guide](../guides/DLSS_Integration.md)
- [FSR2 Integration Guide](../guides/FSR2_Integration.md)

