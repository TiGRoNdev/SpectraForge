# FreqVox Renderer Integration Guide

## Обзор

FreqVox (Adaptive Frequency-Domain Sparse Neural-Voxel Rendering) - это инновационный алгоритм рендеринга, реализованный в HyperEngine. Он сочетает фовеированную выборку вокселей, частотное шейдинг, временную репроекцию и нейронный апскейлинг для достижения высокой производительности при сохранении качества изображения.

## Архитектура

### Основные компоненты

1. **FoveatedSelector** - фовеированная выборка вокселей с LOD
2. **FrequencyShadingPipeline** - частотное шейдинг (DCT/FFT)
3. **FFT Backends**:
   - `cuFFTBackend` - NVIDIA CUDA (требует CUDA)
   - `VkFFTBackend` - кроссплатформенный Vulkan
   - `SimpleDctBackend` - простая заглушка для тестов
4. **TemporalReprojection** - временная репроекция с motion vectors
5. **NeuralUpscaler** - нейронный апскейлинг (DLSS/FSR2/Bilinear)

### Интеграция в движок

FreqVox интегрируется как **IRenderStrategy** в систему `ModernRenderer3D`:

```cpp
// Создание через фабрику
auto strategy = create_strategy_by_name("freqvox");

// Передача в ModernRenderer3D через DI
auto renderer = std::make_shared<ModernRenderer3D>(
    strategy,
    lightingSystem,
    cameraSystem,
    statistics,
    logger
);
```

## Сборка

### Опции CMake

```cmake
option(ENABLE_FREQVOX "Enable FreqVox Renderer modules" ON)
option(BUILD_WITH_CUDA "Enable CUDA support for cuFFT backend" ON)
```

### Зависимости

- **Обязательные**: C++20, Vulkan SDK
- **Опциональные**:
  - CUDA Toolkit 11.0+ (для cuFFTBackend)
  - VkFFT (автоматически через FetchContent/vcpkg)

### Команды сборки

```bash
# Linux
mkdir build && cd build
cmake .. -DENABLE_FREQVOX=ON -DBUILD_WITH_CUDA=ON
cmake --build . --config Release

# Тесты
ctest --output-on-failure
```

## Использование

### Пример 1: Базовая инициализация

```cpp
#include "HyperEngine/Rendering/FreqVox/FreqVoxStrategy.h"

using namespace HyperEngine::Rendering::FreqVox;

// Создание стратегии
auto strategy = std::make_shared<FreqVoxStrategy>();

// Инициализация
if (!strategy->initialize()) {
    SAFE_ERROR("Ошибка инициализации FreqVox");
    return false;
}

// Рендеринг кадра
FrameData frameData = prepareFrameData();
strategy->render(frameData);

// Cleanup
strategy->shutdown();
```

### Пример 2: Выбор FFT бэкенда

```cpp
#include "HyperEngine/Rendering/FreqVox/BackendFactory.h"

// Автовыбор лучшего бэкенда
auto backend = BackendFactory::create(BackendFactory::BackendType::Auto);

// Или явно cuFFT (требует CUDA)
auto cudaBackend = BackendFactory::create(BackendFactory::BackendType::CuFFT);

// Или VkFFT (кроссплатформенный)
auto vkBackend = BackendFactory::create(BackendFactory::BackendType::VkFFT);
```

### Пример 3: Демо приложение

Запустите пример:

```bash
./build/FreqVox_Demo
```

Исходник: `examples/freqvox_demo.cpp`

## API Reference

### BackendFactory

Фабрика FFT/DCT бэкендов:

```cpp
namespace HyperEngine::Rendering::FreqVox {

class BackendFactory {
public:
    enum class BackendType {
        Auto,      // Автовыбор
        CuFFT,     // NVIDIA CUDA
        VkFFT,     // Vulkan
        Simple     // Заглушка
    };

    static std::unique_ptr<IFrequencyBackend> create(BackendType type = Auto);
    static bool isAvailable(BackendType type);
};

}
```

### FreqVoxStrategy

Стратегия рендеринга:

```cpp
class FreqVoxStrategy : public IRenderStrategy {
public:
    bool initialize() override;
    void render(const FrameData& frameData) override;
    void shutdown() override;
    std::string getName() const override;
};
```

### TemporalReprojection

Временная репроекция:

```cpp
class TemporalReprojection {
public:
    bool initialize(uint32_t width, uint32_t height, 
                    const TemporalReprojectionParams& params);
    bool reproject(const std::vector<float>& current_frame,
                   const std::vector<Math::Vector3>& motion_vectors,
                   const std::vector<float>& depth_buffer,
                   std::vector<float>& output_frame);
    void resetHistory();
    void shutdown();
};
```

### NeuralUpscaler

Нейронный апскейлинг:

```cpp
class NeuralUpscaler {
public:
    bool initialize(UpscalerType type, uint32_t input_width, 
                    uint32_t input_height, const NeuralUpscalingParams& params);
    bool upscale(const std::vector<float>& input_image,
                 std::vector<float>& output_image,
                 const std::vector<float>* motion_vectors = nullptr);
    std::pair<uint32_t, uint32_t> getOutputSize() const;
    void shutdown();
};
```

## Производительность

### Ожидаемые улучшения

- **Снижение вокселей**: V_eff ≈ 0.2 * V_total (фовеация)
- **FFT батчинг**: ~4x ускорение vs per-pixel shading
- **Апскейлинг**: 2-8x FPS boost (DLSS/FSR2)
- **Temporal stability**: снижение шума, сглаживание

### Бенчмарки

```bash
# Запуск бенчмарков (в разработке)
./build/tests/performance/freqvox_benchmark
```

## Тестирование

### Unit тесты

```bash
# Все тесты FreqVox
ctest -R freqvox

# Отдельные категории
ctest -R freqvox_backends_test
ctest -R freqvox_strategy_test
ctest -R freqvox_stage_test
```

### Покрытие кода

```bash
# Генерация отчёта покрытия
cmake .. -DENABLE_CODE_COVERAGE=ON
make coverage
```

Цель: ≥80% покрытие для всех модулей FreqVox.

## Troubleshooting

### CUDA не найдена

```
Ошибка: cuFFT backend недоступен
```

**Решение**: Установите CUDA Toolkit и пересоберите с `-DBUILD_WITH_CUDA=ON`

### VkFFT ошибка линковки

```
Ошибка: VkFFT headers not found
```

**Решение**: VkFFT интегрируется автоматически. Проверьте наличие `vcpkg` или CMake FetchContent.

### Низкая производительность

**Чеклист**:
1. Используется ли cuFFTBackend на NVIDIA GPU?
2. Включен ли DLSS/FSR2 для апскейлинга?
3. Оптимальны ли параметры foveation (радиус, угол)?
4. Проверьте профилирование: `nsys profile ./FreqVox_Demo`

## Roadmap

- [x] Базовая архитектура и FFT бэкенды
- [x] Temporal reprojection
- [x] Neural upscaling заглушка
- [ ] Реальная интеграция DLSS через Streamline
- [ ] Реальная интеграция FSR2
- [ ] VkFFT полная поддержка DCT-II/III
- [ ] Профилирование и оптимизация аллокаций
- [ ] GPU-driven LOD culling
- [ ] Интеграция с OptiX для secondary rays

## Ссылки

- [FreqVox Концепция](concept/FreqVox%20Renderer.md)
- [Математическая модель](concept/FreqVox%20Renderer%20Math.md)
- [План реализации](../FREQVOX_IMPL.md)
- [Архитектура HyperEngine](architecture/)

## Лицензия

См. [LICENSE](../LICENSE) в корне репозитория.

