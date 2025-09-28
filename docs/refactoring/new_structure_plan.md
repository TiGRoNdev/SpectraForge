# План новой структуры проекта HyperEngine

## 🎯 Цели реорганизации

1. **Единая структура**: Объединить src3D и srcVulkan в coherent структуру
2. **Четкое разделение слоев**: Разделить по функциональным областям
3. **Модульность**: Каждый компонент как отдельная библиотека
4. **Расширяемость**: Простое добавление новых backend'ов рендеринга

## 📂 Текущая структура (проблемы):

```
include/Engine3D/          # Смешение 3D и Vulkan компонентов
├── Core/                  # Основные компоненты
├── Math/                  # Математическая библиотека
├── Rendering/             # Система рендеринга (смешанная)
├── Physics/               # Физическая система
├── Input/                 # Система ввода
├── CUDA/                  # CUDA интеграция
└── Vulkan/                # Vulkan специфичные классы

src3D/                     # Дублирование с srcVulkan
├── Core/
├── Math/
├── Physics/
├── Input/
└── Rendering/

srcVulkan/                 # Отдельная структура для Vulkan
├── CUDA/
├── OptiX/
├── Upscaling/
└── Vulkan/
```

**Проблемы:**
- Дублирование функциональности между src3D и srcVulkan
- Отсутствие четкого API между слоями
- Смешение абстракций с конкретными реализациями
- Сложность добавления новых backend'ов

## 🎯 Целевая структура:

```
src/
├── core/                  # Основные компоненты движка
│   ├── engine/           # Главный класс движка
│   ├── memory/           # Управление памятью
│   ├── threading/        # Потоки и синхронизация
│   └── events/           # Система событий
│
├── math/                  # Математическая библиотека
│   ├── vector/           # Векторы (Vector3, Vector4)
│   ├── matrix/           # Матрицы и кватернионы
│   └── geometry/         # Геометрические примитивы
│
├── rendering/             # Система рендеринга
│   ├── common/           # Общие интерфейсы и структуры
│   ├── opengl/           # OpenGL backend (будущее)
│   ├── vulkan/           # Vulkan backend
│   │   ├── core/         # Основа Vulkan
│   │   ├── compute/      # CUDA интеграция
│   │   ├── raytracing/   # OptiX интеграция
│   │   └── upscaling/    # DLSS/FSR
│   ├── shaders/          # Система шейдеров
│   └── materials/        # Система материалов
│
├── physics/               # Физическая система
│   ├── collision/        # Обнаружение столкновений
│   ├── dynamics/         # Динамика тел
│   └── constraints/      # Ограничения
│
├── input/                 # Система ввода
│   ├── devices/          # Устройства ввода
│   ├── actions/          # Действия
│   └── bindings/         # Привязки клавиш
│
└── platform/              # Платформо-зависимый код
    ├── windows/          # Windows специфичный код
    ├── linux/            # Linux специфичный код
    └── common/           # Общий платформенный код

include/HyperEngine/       # Новые заголовочные файлы
├── Core/                 # Интерфейсы основных компонентов
├── Math/                 # Математические классы
├── Rendering/            # Интерфейсы рендеринга
│   ├── Common/           # Общие интерфейсы
│   ├── Vulkan/           # Vulkan специфичные классы
│   └── RenderStages/     # Этапы рендеринга
├── Physics/              # Физические интерфейсы
├── Input/                # Интерфейсы ввода
└── Platform/             # Платформенные интерфейсы
```

## 🔧 План миграции файлов

### Математическая библиотека (Приоритет 1)
```
src3D/Math/Vector3.cpp     → src/math/vector/Vector3.cpp
src3D/Math/Vector4.cpp     → src/math/vector/Vector4.cpp  
src3D/Math/Matrix4.cpp     → src/math/matrix/Matrix4.cpp
src3D/Math/Quaternion.cpp  → src/math/matrix/Quaternion.cpp

include/Engine3D/Math/Vector3.h    → include/HyperEngine/Math/Vector3.h
include/Engine3D/Math/Vector4.h    → include/HyperEngine/Math/Vector4.h
include/Engine3D/Math/Matrix4.h    → include/HyperEngine/Math/Matrix4.h
include/Engine3D/Math/Quaternion.h → include/HyperEngine/Math/Quaternion.h
```

### Система рендеринга (Приоритет 2)
```
src3D/Rendering/           → src/rendering/opengl/
srcVulkan/Vulkan/          → src/rendering/vulkan/core/
srcVulkan/CUDA/            → src/rendering/vulkan/compute/
srcVulkan/OptiX/           → src/rendering/vulkan/raytracing/
srcVulkan/Upscaling/       → src/rendering/vulkan/upscaling/
```

### Физическая система (Приоритет 3)
```
src3D/Physics/             → src/physics/
```

### Остальные компоненты (Приоритет 4)
```
src3D/Core/                → src/core/
src3D/Input/               → src/input/
```

## 🔄 Последовательность действий

1. **Создание структуры директорий**
2. **Миграция математической библиотеки** (наименее связанная)
3. **Обновление namespace'ов**: `Engine3D` → `HyperEngine`
4. **Создание модульной структуры CMakeLists.txt**
5. **Тестирование каждого модуля отдельно**
6. **Постепенная миграция остальных компонентов**

## 📋 Новые библиотеки

### HyperEngine_Math
- Vector3, Vector4, Matrix4, Quaternion
- Независимая от других компонентов
- Высокая производительность

### HyperEngine_Core  
- Базовые классы движка
- Управление памятью
- Система событий

### HyperEngine_Rendering_Common
- Интерфейсы рендеринга
- Общие структуры данных
- Базовые классы

### HyperEngine_Rendering_Vulkan
- Vulkan реализация
- CUDA интеграция
- OptiX поддержка
- Upscaling (DLSS/FSR)

### HyperEngine_Physics
- Физическая симуляция
- Обнаружение столкновений

### HyperEngine_Input
- Обработка ввода
- Система действий

## ⚠️ Критические моменты

1. **Сохранение API**: Внешний API должен остаться совместимым
2. **Постепенность**: Каждый шаг должен давать рабочий результат
3. **Тестирование**: Каждый модуль тестируется отдельно
4. **Зависимости**: Четкий контроль межмодульных зависимостей

## 🎯 Ожидаемые результаты

- **Модульность**: Каждый компонент как отдельная библиотека
- **Переиспользование**: Математическая библиотека в других проектах
- **Расширяемость**: Простое добавление DirectX12, Metal backend'ов
- **Читаемость**: Четкая структура для новых разработчиков
- **Тестируемость**: Изолированное тестирование каждого модуля

Дата создания: 28 сентября 2025
Этап: 2 из 8
