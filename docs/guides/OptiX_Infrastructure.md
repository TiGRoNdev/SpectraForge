# OptiX Infrastructure - Система Ray Tracing для Вторичных Эффектов

## Обзор

OptiX Infrastructure представляет собой систему ray tracing для рендеринга вторичных эффектов, основанную на NVIDIA OptiX 7.x. Система предназначена для создания высококачественных отражений, теней и глобального освещения в реальном времени.

## Архитектура

### Основные компоненты

1. **OptiXRayTracer** - Главный класс ray tracer
2. **AccelerationStructure** - Управление acceleration structures (GAS/TLAS)
3. **ShaderBindingTable** - Таблица привязки шейдеров
4. **Ray Shaders** - CUDA kernels для трассировки лучей

### Диаграмма архитектуры

```
OptiXRayTracer
├── AccelerationStructure (управление GAS)
├── ShaderBindingTable (SBT)
├── Ray Generation Shaders
├── Miss Shaders
├── Hit Group Shaders
└── Выходные буферы (отражения, тени, GI)
```

## Ключевые возможности

### 1. Вторичные эффекты
- **Отражения**: Реалистичные отражения с поддержкой рекурсии
- **Тени**: Точные тени от источников света
- **Глобальное освещение**: Рассеянное освещение и цветовое кровотечение

### 2. Оптимизация производительности
- **Shader Execution Reordering (SER)**: Повышение когерентности выполнения
- **Acceleration Structures**: Оптимизированные структуры для быстрого пересечения
- **Многоуровневая трассировка**: Контроль глубины рекурсии

### 3. Интеграция с Vulkan
- **Interop буферы**: Совместное использование памяти CUDA-Vulkan
- **Синхронизация**: Координация выполнения между API
- **Resource управление**: Единый менеджер ресурсов

## Использование

### Базовая инициализация

```cpp
#include "Engine3D/OptiX/OptiXRayTracer.h"

// Создание ray tracer
OptiXRayTracer rayTracer;

// Инициализация с CUDA контекстом
CUcontext cudaContext;
if (!rayTracer.init(cudaContext)) {
    // Обработка ошибки
}
```

### Подготовка геометрии

```cpp
// Создание геометрии сцены
SceneGeometry geometry = {};
geometry.vertices = vertexData;
geometry.indices = indexData;
geometry.vertexCount = numVertices;
geometry.triangleCount = numTriangles;
geometry.vertexStride = sizeof(Vertex);

// Построение acceleration structures
rayTracer.buildAccelerationStructures(geometry);
```

### Настройка параметров рендеринга

```cpp
LaunchParams params = {};
params.viewMatrix = camera.getViewMatrix();
params.projMatrix = camera.getProjectionMatrix();
params.cameraPos = camera.getPosition();
params.lightPos = light.getPosition();
params.lightColor = light.getColor();
params.lightIntensity = light.getIntensity();
params.width = renderWidth;
params.height = renderHeight;
params.maxDepth = 3; // Максимальная глубина рекурсии
```

### Выполнение трассировки

```cpp
// Трассировка лучей
RawEffects effects = rayTracer.traceRays(params);

// Получение результатов
float* reflections = effects.reflections;
float* shadows = effects.shadows;
float* globalIllumination = effects.globalIllumination;
float* motionVectors = effects.motionVectors;
float* albedo = effects.albedo;
float* normals = effects.normals;
```

### Оптимизация производительности

```cpp
// Настройка Shader Execution Reordering
CoherencyHints hints = {};
hints.rayCoherence = 0.8f;      // Высокая когерентность лучей
hints.materialCoherence = 0.6f; // Средняя когерентность материалов
hints.geometryCoherence = 0.9f; // Высокая когерентность геометрии

rayTracer.applySER(hints);
```

## Шейдеры

### Ray Generation Shaders

#### Отражения
```cuda
extern "C" __global__ void __raygen__reflections() {
    // Генерация первичных лучей для отражений
    // Трассировка с рекурсивными отражениями
}
```

#### Тени
```cuda
extern "C" __global__ void __raygen__shadows() {
    // Генерация лучей теней к источникам света
    // Простая бинарная проверка видимости
}
```

### Miss Shaders

```cuda
extern "C" __global__ void __miss__reflections() {
    // Обработка промахов для отражений
    // Возврат цвета окружения
}

extern "C" __global__ void __miss__shadows() {
    // Обработка промахов для теней
    // Возврат полного освещения
}
```

### Hit Shaders

```cuda
extern "C" __global__ void __closesthit__reflections() {
    // Обработка пересечений для отражений
    // Вычисление освещения и рекурсивных отражений
}

extern "C" __global__ void __closesthit__shadows() {
    // Обработка пересечений для теней
    // Простое затенение
}
```

## Конфигурация

### CMake настройки

```cmake
# Включение поддержки OptiX
option(BUILD_WITH_OPTIX "Build with OptiX support" ON)

# Поиск OptiX SDK
find_package(OptiX REQUIRED)

# Настройка target
target_include_directories(YourTarget PRIVATE ${OptiX_INCLUDE_DIRS})
target_compile_definitions(YourTarget PUBLIC VULKAN_RENDERER_OPTIX_SUPPORT)
```

### Переменные окружения

```bash
# Путь к OptiX SDK
export OPTIX_ROOT="/path/to/optix/sdk"

# Путь к CUDA Toolkit
export CUDA_PATH="/path/to/cuda"
```

## Производительность

### Ожидаемые показатели

| Разрешение | Сложность сцены | FPS (RTX 3080) | Время кадра |
|------------|----------------|----------------|-------------|
| 1920x1080  | Простая        | 120-150        | 6-8 мс      |
| 1920x1080  | Средняя        | 80-100         | 10-12 мс    |
| 1920x1080  | Сложная        | 45-60          | 16-22 мс    |
| 2560x1440  | Средняя        | 50-70          | 14-20 мс    |

### Факторы производительности

1. **Глубина трассировки**: Каждый уровень рекурсии удваивает время
2. **Количество источников света**: Линейное влияние на время теней
3. **Сложность материалов**: Металлические поверхности требуют больше вычислений
4. **Разрешение**: Квадратичное влияние на время выполнения

### Рекомендации по оптимизации

1. **Ограничьте глубину трассировки** до 2-3 уровней
2. **Используйте SER** для повышения когерентности
3. **Оптимизируйте acceleration structures** для вашей геометрии
4. **Применяйте culling** для невидимых объектов

## Отладка

### Логирование

```cpp
// Включение подробного логирования OptiX
OptixDeviceContextOptions options = {};
options.logCallbackLevel = 4; // TRACE уровень
```

### Профилирование

```bash
# Профилирование CUDA kernels
nvprof ./your_optix_app

# Анализ производительности
nsight-compute ./your_optix_app
```

### Частые проблемы

1. **OptiX не найден**: Проверьте переменную `OPTIX_ROOT`
2. **CUDA ошибки**: Убедитесь в совместимости версий CUDA и OptiX
3. **Низкая производительность**: Проверьте настройки SER и глубину трассировки

## Интеграция с остальной системой

### Vulkan Renderer

```cpp
// Интеграция с Vulkan рендерером
class VulkanRenderer {
    std::unique_ptr<OptiXRayTracer> rayTracer;
    
    void renderFrame() {
        // 1. Первичная растеризация (Vulkan)
        renderPrimaryGeometry();
        
        // 2. Вторичные эффекты (OptiX)
        RawEffects effects = rayTracer->traceRays(params);
        
        // 3. Композиция результатов
        compositeEffects(effects);
    }
};
```

### Denoising Module

```cpp
// Передача результатов в деноизер
DenoiseModule denoiser;
DenoisedImage result = denoiser.denoise(effects, auxBuffers);
```

## Будущие улучшения

1. **Multi-GPU поддержка**: Распределение нагрузки между GPU
2. **Temporal accumulation**: Накопление данных между кадрами
3. **Adaptive sampling**: Динамическое изменение качества
4. **Hardware RT cores**: Использование специализированного железа

## Примеры

### Простой рендеринг отражений

```cpp
// Полный пример рендеринга отражений
#include "Engine3D/OptiX/OptiXRayTracer.h"

int main() {
    // Инициализация
    OptiXRayTracer rayTracer;
    rayTracer.init(cudaContext);
    
    // Геометрия
    SceneGeometry geometry = createScene();
    rayTracer.buildAccelerationStructures(geometry);
    
    // Рендеринг
    LaunchParams params = setupCamera();
    RawEffects effects = rayTracer.traceRays(params);
    
    // Сохранение результата
    saveImage("reflections.png", effects.reflections, params.width, params.height);
    
    return 0;
}
```

## Заключение

OptiX Infrastructure предоставляет мощные возможности для создания высококачественных вторичных эффектов в реальном времени. Система оптимизирована для производительности и легко интегрируется с существующим рендереров на Vulkan.

Для получения дополнительной информации обратитесь к:
- [OptiX Programming Guide](https://raytracing-docs.nvidia.com/optix7/guide/index.html)
- [CUDA Programming Guide](https://docs.nvidia.com/cuda/cuda-c-programming-guide/)
- [Vulkan-CUDA Interop](https://www.khronos.org/registry/vulkan/specs/1.3-extensions/html/vkspec.html#memory-external)
