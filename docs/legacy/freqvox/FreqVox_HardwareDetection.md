# FreqVox Hardware-Aware Backend Selection

## Обзор

FreqVox Renderer теперь поддерживает интеллектуальный выбор FFT/DCT бэкенда на основе доступного оборудования. Система автоматически определяет GPU и выбирает оптимальный бэкенд для максимальной производительности.

## Архитектура

### Приоритет выбора бэкендов

1. **cuFFT (NVIDIA + CUDA)** → Самая высокая производительность (~10-20x против CPU)
   - Требования: NVIDIA GPU + CUDA Toolkit + Runtime CUDA поддержка
   - Использует: CUDA-ускоренное FFT преобразование

2. **VkFFT (Vulkan)** → Хорошая производительность (~5-10x против CPU)
   - Требования: Любой Vulkan-совместимый GPU (NVIDIA/AMD/Intel)
   - Использует: Compute shaders через Vulkan

3. **SimpleDCT (CPU)** → Fallback для совместимости
   - Требования: Нет
   - Использует: Простой CPU-based алгоритм

### Интеграция с HardwareDetector

`BackendFactory` интегрирован с `HardwareDetector` для runtime проверок:

```cpp
// Проверка вендора GPU
VendorType vendor = hwDetector->detectVendor();

// Проверка CUDA поддержки (runtime)
bool cudaAvailable = hwDetector->supportsCUDA();

// Проверка Vulkan возможностей
bool vulkanSupported = hwDetector->supportsExtension("VK_KHR_compute");
```

## API Reference

### BackendFactory::createWithHardwareDetection()

Создает бэкенд с учетом runtime проверок железа.

```cpp
static std::unique_ptr<IFrequencyBackend> createWithHardwareDetection(
    BackendType type,
    SpectraForge::Vulkan::HardwareDetector* hwDetector
);
```

**Параметры:**
- `type` - Тип бэкенда (`Auto` для автовыбора)
- `hwDetector` - Указатель на инициализированный HardwareDetector

**Возвращает:** Умный указатель на бэкенд или `nullptr` при ошибке

**Пример:**
```cpp
// Инициализировать Vulkan и HardwareDetector
vk::Instance instance = createVulkanInstance();
vk::PhysicalDevice device = selectPhysicalDevice(instance);

HardwareDetector hwDetector;
hwDetector.init(device);

// Создать оптимальный бэкенд
auto backend = BackendFactory::createWithHardwareDetection(
    BackendFactory::BackendType::Auto,
    &hwDetector
);
```

### BackendFactory::selectBestBackend()

Определяет оптимальный тип бэкенда для данного железа.

```cpp
static BackendType selectBestBackend(
    SpectraForge::Vulkan::HardwareDetector* hwDetector
);
```

**Возвращает:** Рекомендуемый `BackendType`

**Логика выбора:**
```
NVIDIA GPU + CUDA runtime → cuFFT
AMD/Intel/NVIDIA GPU + Vulkan → VkFFT
Неизвестный вендор → Simple
```

### BackendFactory::isAvailableOnHardware()

Проверяет доступность бэкенда на конкретном железе (runtime).

```cpp
static bool isAvailableOnHardware(
    BackendType type,
    SpectraForge::Vulkan::HardwareDetector* hwDetector
);
```

**Отличие от `isAvailable()`:**
- `isAvailable()` - compile-time проверка (проверяет только флаги компиляции)
- `isAvailableOnHardware()` - runtime проверка (проверяет реальное железо)

## Примеры использования

### Базовое использование (Auto)

Самый простой способ - использовать `Auto` режим:

```cpp
#include "SpectraForge/Rendering/FreqVox/BackendFactory.h"
#include "SpectraForge/Vulkan/HardwareDetector.h"

// Setup Vulkan
vk::Instance instance = /* ... */;
vk::PhysicalDevice physicalDevice = /* ... */;

// Инициализация HardwareDetector
HardwareDetector hwDetector;
if (!hwDetector.init(physicalDevice)) {
    std::cerr << "Ошибка инициализации HardwareDetector!" << std::endl;
    return;
}

// Автоматический выбор бэкенда
auto backend = BackendFactory::createWithHardwareDetection(
    BackendFactory::BackendType::Auto,
    &hwDetector
);

if (!backend) {
    std::cerr << "Не удалось создать бэкенд!" << std::endl;
    return;
}

// Использование бэкенда
DctBlockConfig config{8, 8, 4};
backend->initialize(config);
// ...
```

### Ручной выбор с проверкой

Если нужно явно указать бэкенд:

```cpp
using BT = BackendFactory::BackendType;

// Хотим использовать cuFFT
BT preferredType = BT::CuFFT;

// Проверяем доступность на железе
if (!BackendFactory::isAvailableOnHardware(preferredType, &hwDetector)) {
    std::cerr << "cuFFT недоступен на этом GPU!" << std::endl;
    
    // Fallback на VkFFT
    preferredType = BT::VkFFT;
}

// Создаем выбранный бэкенд
auto backend = BackendFactory::createWithHardwareDetection(
    preferredType,
    &hwDetector
);
```

### Без HardwareDetector (Fallback)

Если Vulkan недоступен, можно использовать старый API:

```cpp
// Используется compile-time выбор
auto backend = BackendFactory::create(
    BackendFactory::BackendType::Auto
);

// Система выберет:
// - cuFFT если HYPERENGINE_CUDA_AVAILABLE определен
// - VkFFT иначе
```

⚠️ **Внимание:** Без HardwareDetector не выполняются runtime проверки CUDA!

### Проверка compile-time vs runtime доступности

```cpp
using BT = BackendFactory::BackendType;

// Compile-time проверка
bool compiledWithCUDA = BackendFactory::isAvailable(BT::CuFFT);
std::cout << "CUDA скомпилирован: " << (compiledWithCUDA ? "ДА" : "НЕТ") << std::endl;

// Runtime проверка (требует HardwareDetector)
bool cudaWorksOnGPU = BackendFactory::isAvailableOnHardware(BT::CuFFT, &hwDetector);
std::cout << "CUDA работает на GPU: " << (cudaWorksOnGPU ? "ДА" : "НЕТ") << std::endl;

// Возможная ситуация: compiledWithCUDA=true, cudaWorksOnGPU=false
// Причина: CUDA скомпилирован, но на системе AMD GPU
```

## Логирование

Система автоматически логирует процесс выбора:

```
[BackendFactory] === Анализ железа для выбора оптимального бэкенда ===
[BackendFactory] GPU: NVIDIA GeForce RTX 3080
[BackendFactory] Вендор: NVIDIA
[BackendFactory] CUDA поддержка: ДА
[BackendFactory] ✅ Выбран cuFFT (NVIDIA + CUDA)
[BackendFactory] Ожидаемое ускорение: ~10-20x против CPU
[BackendFactory] Создан cuFFT backend (NVIDIA: NVIDIA GeForce RTX 3080)
```

Для AMD GPU:

```
[BackendFactory] === Анализ железа для выбора оптимального бэкенда ===
[BackendFactory] GPU: AMD Radeon RX 6800 XT
[BackendFactory] Вендор: AMD
[BackendFactory] ✅ Выбран VkFFT (AMD GPU через Vulkan)
[BackendFactory] Ожидаемое ускорение: ~5-10x против CPU
[BackendFactory] Создан VkFFT backend (GPU: AMD Radeon RX 6800 XT)
```

## Производительность

### Таблица производительности на разных GPU

| GPU | Выбранный бэкенд | Относительная скорость | Примечание |
|-----|-----------------|----------------------|-----------|
| NVIDIA RTX 3080 | cuFFT | **1.0x** (референс) | Максимальная скорость |
| NVIDIA GTX 1060 | cuFFT | ~0.6x | Старая архитектура |
| AMD RX 6800 XT | VkFFT | ~0.7x | Зависит от драйвера |
| Intel Arc A770 | VkFFT | ~0.5x | Новая архитектура |
| CPU (16 ядер) | Simple | ~0.05x | Только для fallback |

### Рекомендации по оптимизации

1. **NVIDIA GPU**: Убедитесь что установлен CUDA Toolkit последней версии
2. **AMD/Intel GPU**: Обновите драйверы Vulkan до последней версии
3. **Батчинг**: Используйте большие значения `batch_size` для лучшей утилизации GPU
4. **Размер блоков**: Оптимальные размеры: 8x8, 16x16, 32x32

## Troubleshooting

### cuFFT не выбирается на NVIDIA GPU

**Проблема:**
```
[BackendFactory] NVIDIA GPU найден, но CUDA runtime недоступен
[BackendFactory] Fallback на VkFFT
```

**Решения:**
1. Установите CUDA Toolkit (https://developer.nvidia.com/cuda-downloads)
2. Проверьте переменные окружения: `echo $CUDA_PATH`
3. Убедитесь что драйвер NVIDIA поддерживает вашу версию CUDA
4. Пересоберите проект с `-DHYPERENGINE_CUDA_AVAILABLE=ON`

### VkFFT падает с ошибкой валидации

**Проблема:**
```
[VkFFT] Validation layer error: vkCreateComputePipeline failed
```

**Решения:**
1. Обновите драйверы GPU до последней версии
2. Проверьте что компьютные шейдеры поддерживаются: `vulkaninfo | grep compute`
3. Попробуйте уменьшить `batch_size` в конфиге

### Падение производительности на больших данных

**Проблема:** FFT медленно работает на больших батчах

**Решения:**
1. Профилируйте память: VkFFT требует больше VRAM чем cuFFT
2. Разбейте большие батчи на несколько маленьких
3. Используйте `VK_EXT_memory_budget` для мониторинга VRAM

## См. также

- [FreqVox Renderer Documentation](FreqVox_Integration.md)
- [Hardware Detector API](../api/HardwareDetector.md)
- [Backend Performance Comparison](FreqVox_Performance.md)
- [CUDA Integration Guide](guides/cuda_integration.md)

## История изменений

### v2.0 (2025-10-02)
- ✅ Добавлена интеграция с HardwareDetector
- ✅ Реализован runtime выбор бэкенда на основе GPU
- ✅ Добавлены методы `createWithHardwareDetection()`, `selectBestBackend()`, `isAvailableOnHardware()`
- ✅ Улучшено логирование процесса выбора
- ✅ Обновлен freqvox_demo.cpp с примерами

### v1.0 (2025-09-30)
- Базовая фабрика с compile-time выбором
- Поддержка cuFFT, VkFFT, Simple бэкендов

