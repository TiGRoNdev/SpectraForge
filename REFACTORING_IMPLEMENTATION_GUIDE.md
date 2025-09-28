# 📋 Руководство по реализации рефакторинга HyperEngine

## 🎯 Обзор проекта рефакторинга

Данное руководство содержит детальные пошаговые инструкции для выполнения полного рефакторинга проекта HyperEngine согласно современным принципам разработки и стандартам качества кода.

### 📌 Принципы рефакторинга (по Refactoring.Guru)

1. **Код должен стать чище** - каждое изменение должно улучшать читаемость и структуру
2. **Не создавать новую функциональность** - фокус только на улучшении существующего кода
3. **Все тесты должны проходить** - сохранение работоспособности на каждом этапе
4. **Мелкие изменения** - избегать больших комплексных изменений за раз

### 🎪 Структура руководства

Руководство разделено на 8 основных этапов:
- **Этап 1**: Подготовка и настройка инфраструктуры
- **Этап 2**: Реорганизация структуры проекта
- **Этап 3**: Рефакторинг архитектуры (SOLID принципы)
- **Этап 4**: Улучшение системы тестирования
- **Этап 5**: Настройка CI/CD пайплайна
- **Этап 6**: Улучшение документации
- **Этап 7**: Инструменты качества кода
- **Этап 8**: Финализация и валидация

### ⚠️ Важные предупреждения

- **НЕ** смешивайте рефакторинг с добавлением новых функций
- Каждый этап должен завершаться рабочим состоянием проекта
- Обязательно создавайте резервные копии перед началом каждого этапа
- Тестируйте после каждого значимого изменения

### 🔧 Требования перед началом

1. Visual Studio 2019/2022 с поддержкой C++20
2. CMake 3.16+
3. vcpkg установлен и настроен
4. Git настроен для работы с репозиторием
5. Права на запись в директорию проекта

---

## 📋 Этап 1: Подготовка и настройка инфраструктуры

### 1.1 Создание резервной копии

```bash
# Создать ветку для рефакторинга
git checkout -b refactoring/main-restructure

# Создать тег текущего состояния
git tag -a v0.0.11-pre-refactor -m "Состояние перед рефакторингом"

# Убедиться, что все изменения закоммичены
git status
```

### 1.2 Настройка рабочего окружения

```bash
# Обновить submodules
git submodule update --init --recursive

# Проверить сборку текущего состояния
cd build-vcpkg
cmake --build . --config Release

# Запустить существующие тесты
cd tests
./unit_tests.exe
```

### 1.3 Создание структуры для планирования

```bash
# Создать директорию для документации рефакторинга
mkdir -p docs/refactoring
mkdir -p docs/refactoring/progress
mkdir -p docs/refactoring/decisions
```

### 1.4 Анализ текущего состояния кода

```bash
# Установить инструменты анализа кода (если не установлены)
# clang-tidy уже настроен в проекте

# Запустить анализ текущего состояния
find src3D srcVulkan include -name "*.cpp" -o -name "*.h" | head -20 | xargs clang-tidy --checks="-*,readability-*,performance-*,bugprone-*" > docs/refactoring/initial_analysis.txt

# Подсчитать строки кода
find src3D srcVulkan include -name "*.cpp" -o -name "*.h" | xargs wc -l > docs/refactoring/initial_metrics.txt
```

### 1.5 Создание чек-листа прогресса

Создать файл `docs/refactoring/progress/checklist.md`:

```markdown
# Чек-лист прогресса рефакторинга

## Этап 1: Подготовка ✅
- [x] Создание резервной копии
- [x] Настройка окружения
- [x] Анализ текущего состояния
- [ ] Планирование структуры

## Этап 2: Структура проекта
- [ ] Создание новых директорий
- [ ] Планирование миграции файлов
- [ ] Обновление CMakeLists.txt

## Этап 3: Архитектурный рефакторинг
- [ ] Выделение интерфейсов
- [ ] Применение SOLID принципов
- [ ] Внедрение паттернов проектирования

## [Продолжение следует...]
```

### 1.6 Настройка инструментов разработки

Проверить и обновить конфигурационные файлы:

1. **Проверить .clang-format**:
```bash
# Файл уже существует, проверить настройки
cat .clang-format
```

2. **Проверить .clang-tidy**:
```bash
# Файл уже существует, проверить правила
cat .clang-tidy
```

3. **Обновить .gitignore** при необходимости:
```bash
# Добавить новые директории для игнорирования
echo "docs/refactoring/temp/" >> .gitignore
echo "build-refactor/" >> .gitignore
```

### 1.7 Валидация этапа 1

Перед переходом к следующему этапу убедиться:

- [x] Проект собирается без ошибок
- [x] Все существующие тесты проходят
- [x] Созданы необходимые директории для документации
- [x] Настроены инструменты анализа кода
- [x] Создан план отката (git tag + branch)

```bash
# Коммит изменений этапа 1
git add .
git commit -m "Этап 1: Подготовка инфраструктуры для рефакторинга

- Создана резервная копия (tag v0.0.11-pre-refactor)
- Настроены инструменты анализа
- Созданы директории для документации
- Проведен анализ текущего состояния кода"
```

---

## 📋 Этап 2: Реорганизация структуры проекта

### 2.1 Планирование новой структуры

Создать файл `docs/refactoring/new_structure_plan.md`:

```markdown
# План новой структуры проекта

## Текущая структура (проблемы):
- Смешение 3D и Vulkan кода в разных директориях
- Отсутствие четкого разделения слоев
- Дублирование функциональности

## Целевая структура:
```
src/
├── core/           # Основные компоненты движка
├── math/           # Математическая библиотека
├── rendering/      # Система рендеринга
│   ├── common/     # Общие интерфейсы
│   ├── opengl/     # OpenGL backend
│   └── vulkan/     # Vulkan backend
├── physics/        # Физическая система
├── input/          # Система ввода
└── platform/       # Платформо-зависимый код
```
```

### 2.2 Создание новой структуры директорий

```bash
# Создать основные директории
mkdir -p src/core/engine
mkdir -p src/core/memory  
mkdir -p src/core/threading
mkdir -p src/core/events

mkdir -p src/math/vector
mkdir -p src/math/matrix
mkdir -p src/math/geometry

mkdir -p src/rendering/common
mkdir -p src/rendering/opengl
mkdir -p src/rendering/vulkan
mkdir -p src/rendering/shaders
mkdir -p src/rendering/materials

mkdir -p src/physics/collision
mkdir -p src/physics/dynamics
mkdir -p src/physics/constraints

mkdir -p src/input/devices
mkdir -p src/input/actions
mkdir -p src/input/bindings

mkdir -p src/platform/windows
mkdir -p src/platform/linux
mkdir -p src/platform/common
```

### 2.3 Реорганизация заголовочных файлов

```bash
# Создать новую структуру include
mkdir -p include/HyperEngine/Core
mkdir -p include/HyperEngine/Math
mkdir -p include/HyperEngine/Rendering
mkdir -p include/HyperEngine/Physics
mkdir -p include/HyperEngine/Input
mkdir -p include/HyperEngine/Platform
```

### 2.4 Планирование миграции файлов

Создать `docs/refactoring/migration_plan.md`:

```markdown
# План миграции файлов

## Математические классы
src3D/Math/ → src/math/
- Vector3.cpp → src/math/vector/Vector3.cpp
- Vector4.cpp → src/math/vector/Vector4.cpp  
- Matrix4.cpp → src/math/matrix/Matrix4.cpp
- Quaternion.cpp → src/math/matrix/Quaternion.cpp

## Система рендеринга
src3D/Rendering/ → src/rendering/opengl/
srcVulkan/Vulkan/ → src/rendering/vulkan/

## Физическая система
src3D/Physics/ → src/physics/
```

### 2.5 Начало миграции математической библиотеки

```bash
# Копировать файлы математики (сначала копия, потом удаление оригиналов)
cp src3D/Math/Vector3.cpp src/math/vector/
cp src3D/Math/Vector4.cpp src/math/vector/
cp src3D/Math/Matrix4.cpp src/math/matrix/
cp src3D/Math/Quaternion.cpp src/math/matrix/

cp include/Engine3D/Math/Vector3.h include/HyperEngine/Math/
cp include/Engine3D/Math/Vector4.h include/HyperEngine/Math/
cp include/Engine3D/Math/Matrix4.h include/HyperEngine/Math/
cp include/Engine3D/Math/Quaternion.h include/HyperEngine/Math/
```

### 2.6 Обновление namespace'ов

В скопированных файлах математической библиотеки обновить namespace:

```cpp
// Было:
namespace Engine3D::Math {
    // ...
}

// Стало:
namespace HyperEngine::Math {
    // ...
}
```

### 2.7 Создание новой структуры CMakeLists.txt

Создать `src/CMakeLists.txt`:

```cmake
# Основной CMakeLists для новой структуры
cmake_minimum_required(VERSION 3.16)

# Математическая библиотека
add_subdirectory(math)

# Система рендеринга
add_subdirectory(rendering)

# Физическая система
add_subdirectory(physics)

# Система ввода
add_subdirectory(input)

# Основные компоненты
add_subdirectory(core)

# Платформо-зависимый код
add_subdirectory(platform)
```

### 2.8 Создание CMakeLists для математической библиотеки

Создать `src/math/CMakeLists.txt`:

```cmake
# Математическая библиотека HyperEngine
set(MATH_SOURCES
    vector/Vector3.cpp
    vector/Vector4.cpp
    matrix/Matrix4.cpp
    matrix/Quaternion.cpp
)

set(MATH_HEADERS
    ${CMAKE_SOURCE_DIR}/include/HyperEngine/Math/Vector3.h
    ${CMAKE_SOURCE_DIR}/include/HyperEngine/Math/Vector4.h
    ${CMAKE_SOURCE_DIR}/include/HyperEngine/Math/Matrix4.h
    ${CMAKE_SOURCE_DIR}/include/HyperEngine/Math/Quaternion.h
)

add_library(HyperEngine_Math STATIC ${MATH_SOURCES} ${MATH_HEADERS})

target_include_directories(HyperEngine_Math 
    PUBLIC ${CMAKE_SOURCE_DIR}/include
    PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}
)

target_compile_features(HyperEngine_Math PUBLIC cxx_std_20)
```

### 2.9 Тестирование новой структуры

```bash
# Создать временную сборку для тестирования новой структуры
mkdir build-refactor
cd build-refactor

# Настроить сборку с новой структурой
cmake .. -DCMAKE_BUILD_TYPE=Debug -DBUILD_TESTING=ON

# Попытаться собрать только математическую библиотеку
cmake --build . --target HyperEngine_Math

# Если есть ошибки - исправить и повторить
```

### 2.10 Валидация этапа 2

Перед переходом к этапу 3:

- [ ] Создана новая структура директорий
- [ ] Скопированы и адаптированы файлы математической библиотеки
- [ ] Обновлены namespace'ы в скопированных файлах
- [ ] Создан CMakeLists.txt для новой структуры
- [ ] Математическая библиотека собирается без ошибок
- [ ] Все существующие тесты продолжают проходить

```bash
# Коммит изменений этапа 2
git add .
git commit -m "Этап 2: Начало реорганизации структуры проекта

- Создана новая структура директорий
- Начата миграция математической библиотеки
- Обновлены namespace'ы на HyperEngine
- Создан базовый CMakeLists.txt для новой структуры"
```

---

## 📋 Этап 3: Рефакторинг архитектуры (SOLID принципы)

### 3.1 Анализ нарушений SOLID принципов

Создать `docs/refactoring/solid_violations.md`:

```markdown
# Анализ нарушений SOLID принципов

## Single Responsibility Principle (SRP) - Нарушения:

### VulkanRenderer.cpp
**Проблема**: Класс выполняет множество ответственностей:
- Управление Vulkan объектами
- Рендеринг (rasterization, ray tracing)
- Управление памятью
- AI деноизинг
- Upscaling

**Решение**: Разделить на отдельные классы согласно ответственностям

### RendererAdapter.cpp  
**Проблема**: Переключение между backend'ами + управление состоянием
**Решение**: Strategy pattern + State management

## Open/Closed Principle (OCP) - Нарушения:

### Отсутствие интерфейсов
**Проблема**: Прямая зависимость от конкретных классов
**Решение**: Создать абстрактные интерфейсы

## Dependency Inversion Principle (DIP) - Нарушения:

### Жесткие зависимости
**Проблема**: Классы создают свои зависимости напрямую
**Решение**: Dependency Injection
```

### 3.2 Создание базовых интерфейсов

Создать `include/HyperEngine/Rendering/IRenderer.h`:

```cpp
#pragma once
#include <memory>
#include <string>

namespace HyperEngine::Rendering {

/**
 * @brief Базовый интерфейс для всех рендереров
 * 
 * Применяет Open/Closed Principle - закрыт для модификации,
 * открыт для расширения через наследование
 */
class IRenderer {
public:
    virtual ~IRenderer() = default;
    
    /**
     * @brief Инициализация рендерера
     * @return true если инициализация прошла успешно
     */
    virtual bool initialize() = 0;
    
    /**
     * @brief Рендеринг кадра
     * @param frameData Данные для рендеринга кадра
     */
    virtual void renderFrame(const FrameData& frameData) = 0;
    
    /**
     * @brief Завершение работы рендерера
     */
    virtual void shutdown() = 0;
    
    /**
     * @brief Получить тип рендерера
     */
    virtual RendererType getType() const = 0;
    
    /**
     * @brief Проверить поддержку функции
     */
    virtual bool supportsFeature(RenderingFeature feature) const = 0;
};

} // namespace HyperEngine::Rendering
```

### 3.3 Создание интерфейса для управления ресурсами

Создать `include/HyperEngine/Rendering/IResourceManager.h`:

```cpp
#pragma once
#include <memory>
#include <cstddef>

namespace HyperEngine::Rendering {

/**
 * @brief Интерфейс для управления ресурсами рендеринга
 * 
 * Применяет Interface Segregation Principle - 
 * клиенты зависят только от нужных им методов
 */
class IResourceManager {
public:
    virtual ~IResourceManager() = default;
    
    /**
     * @brief Выделить буфер
     * @param size Размер буфера в байтах
     * @param usage Тип использования буфера
     * @return Дескриптор буфера
     */
    virtual BufferHandle allocateBuffer(size_t size, BufferUsage usage) = 0;
    
    /**
     * @brief Создать текстуру
     * @param desc Описание текстуры
     * @return Дескриптор текстуры
     */
    virtual TextureHandle createTexture(const TextureDesc& desc) = 0;
    
    /**
     * @brief Освободить ресурс
     * @param handle Дескриптор ресурса
     */
    virtual void releaseResource(ResourceHandle handle) = 0;
    
    /**
     * @brief Получить статистику использования памяти
     */
    virtual MemoryStats getMemoryStats() const = 0;
};

} // namespace HyperEngine::Rendering
```

### 3.4 Применение Single Responsibility Principle

Создать отдельные классы для различных этапов рендеринга:

`include/HyperEngine/Rendering/RenderStages/IRenderStage.h`:

```cpp
#pragma once

namespace HyperEngine::Rendering {

/**
 * @brief Базовый интерфейс для этапов рендеринга
 * 
 * Каждый этап имеет единственную ответственность
 */
class IRenderStage {
public:
    virtual ~IRenderStage() = default;
    
    /**
     * @brief Выполнить этап рендеринга
     * @param context Контекст рендеринга
     */
    virtual void execute(RenderContext& context) = 0;
    
    /**
     * @brief Получить имя этапа
     */
    virtual std::string getName() const = 0;
    
    /**
     * @brief Проверить готовность к выполнению
     */
    virtual bool isReady() const = 0;
};

} // namespace HyperEngine::Rendering
```

### 3.5 Реализация Strategy Pattern для Upscaling

Создать `include/HyperEngine/Rendering/Upscaling/IUpscalingStrategy.h`:

```cpp
#pragma once

namespace HyperEngine::Rendering::Upscaling {

/**
 * @brief Стратегия для upscaling изображений
 * 
 * Применяет Strategy Pattern для переключения между
 * различными алгоритмами upscaling (DLSS, FSR, etc.)
 */
class IUpscalingStrategy {
public:
    virtual ~IUpscalingStrategy() = default;
    
    /**
     * @brief Инициализация стратегии
     * @param config Конфигурация upscaling
     */
    virtual bool initialize(const UpscalingConfig& config) = 0;
    
    /**
     * @brief Применить upscaling к изображению
     * @param input Входное изображение
     * @param output Выходное изображение
     * @param params Параметры upscaling
     */
    virtual void upscale(
        const Image& input, 
        Image& output, 
        const UpscalingParams& params
    ) = 0;
    
    /**
     * @brief Получить тип стратегии
     */
    virtual UpscalingType getType() const = 0;
    
    /**
     * @brief Проверить поддержку на текущем оборудовании
     */
    virtual bool isSupported() const = 0;
};

} // namespace HyperEngine::Rendering::Upscaling
```

### 3.6 Внедрение Dependency Injection

Создать `include/HyperEngine/Core/DependencyInjection/Container.h`:

```cpp
#pragma once
#include <memory>
#include <unordered_map>
#include <typeindex>
#include <functional>

namespace HyperEngine::Core::DI {

/**
 * @brief Контейнер для Dependency Injection
 * 
 * Применяет Dependency Inversion Principle -
 * высокоуровневые модули не зависят от низкоуровневых,
 * оба зависят от абстракций
 */
class Container {
public:
    /**
     * @brief Зарегистрировать тип как Singleton
     */
    template<typename Interface, typename Implementation>
    void registerSingleton() {
        auto factory = []() -> std::shared_ptr<Interface> {
            return std::make_shared<Implementation>();
        };
        
        singletonFactories[std::type_index(typeid(Interface))] = 
            [factory]() -> std::shared_ptr<void> {
                return std::static_pointer_cast<void>(factory());
            };
    }
    
    /**
     * @brief Зарегистрировать фабрику
     */
    template<typename Interface>
    void registerFactory(std::function<std::shared_ptr<Interface>()> factory) {
        factories[std::type_index(typeid(Interface))] = 
            [factory]() -> std::shared_ptr<void> {
                return std::static_pointer_cast<void>(factory());
            };
    }
    
    /**
     * @brief Получить экземпляр типа
     */
    template<typename Interface>
    std::shared_ptr<Interface> resolve() {
        auto typeIndex = std::type_index(typeid(Interface));
        
        // Проверить singleton
        if (auto it = singletons.find(typeIndex); it != singletons.end()) {
            return std::static_pointer_cast<Interface>(it->second);
        }
        
        // Создать singleton если есть фабрика
        if (auto it = singletonFactories.find(typeIndex); it != singletonFactories.end()) {
            auto instance = it->second();
            singletons[typeIndex] = instance;
            return std::static_pointer_cast<Interface>(instance);
        }
        
        // Создать через обычную фабрику
        if (auto it = factories.find(typeIndex); it != factories.end()) {
            return std::static_pointer_cast<Interface>(it->second());
        }
        
        throw std::runtime_error("Тип не зарегистрирован в контейнере");
    }

private:
    std::unordered_map<std::type_index, std::shared_ptr<void>> singletons;
    std::unordered_map<std::type_index, std::function<std::shared_ptr<void>()>> singletonFactories;
    std::unordered_map<std::type_index, std::function<std::shared_ptr<void>()>> factories;
};

} // namespace HyperEngine::Core::DI
```

### 3.7 Создание Factory Pattern для рендереров

Создать `include/HyperEngine/Rendering/RendererFactory.h`:

```cpp
#pragma once
#include "IRenderer.h"
#include <memory>

namespace HyperEngine::Rendering {

enum class RendererType {
    OpenGL,
    Vulkan,
    DirectX12
};

/**
 * @brief Фабрика для создания рендереров
 * 
 * Применяет Factory Pattern - централизованное создание объектов
 * с возможностью выбора конкретной реализации во время выполнения
 */
class RendererFactory {
public:
    /**
     * @brief Создать рендерер указанного типа
     * @param type Тип рендерера
     * @param config Конфигурация рендерера
     * @return Умный указатель на созданный рендерер
     */
    static std::unique_ptr<IRenderer> createRenderer(
        RendererType type, 
        const RendererConfig& config
    );
    
    /**
     * @brief Проверить поддержку типа рендерера на текущей платформе
     * @param type Тип рендерера
     * @return true если поддерживается
     */
    static bool isSupported(RendererType type);
    
    /**
     * @brief Получить список поддерживаемых рендереров
     * @return Вектор поддерживаемых типов
     */
    static std::vector<RendererType> getSupportedRenderers();

private:
    RendererFactory() = default; // Запретить создание экземпляров
};

} // namespace HyperEngine::Rendering
```

### 3.8 Рефакторинг VulkanRenderer согласно SRP

Создать отдельные классы для каждой ответственности:

`src/rendering/vulkan/VulkanRenderer.cpp`:

```cpp
#include "HyperEngine/Rendering/IRenderer.h"
#include "HyperEngine/Rendering/IResourceManager.h"
#include "RenderPipeline.h"

namespace HyperEngine::Rendering::Vulkan {

/**
 * @brief Vulkan реализация рендерера
 * 
 * Отвечает ТОЛЬКО за координацию этапов рендеринга
 * Конкретные задачи делегированы специализированным классам
 */
class VulkanRenderer : public IRenderer {
private:
    std::unique_ptr<RenderPipeline> renderPipeline;
    std::shared_ptr<IResourceManager> resourceManager;
    bool initialized = false;

public:
    VulkanRenderer(std::shared_ptr<IResourceManager> resourceMgr)
        : resourceManager(std::move(resourceMgr)) {}
    
    bool initialize() override {
        if (initialized) return true;
        
        // Создать pipeline с внедренными зависимостями
        renderPipeline = std::make_unique<RenderPipeline>(resourceManager);
        
        if (!renderPipeline->initialize()) {
            return false;
        }
        
        initialized = true;
        return true;
    }
    
    void renderFrame(const FrameData& frameData) override {
        if (!initialized) {
            throw std::runtime_error("Рендерер не инициализирован");
        }
        
        renderPipeline->execute(frameData);
    }
    
    void shutdown() override {
        if (renderPipeline) {
            renderPipeline->shutdown();
            renderPipeline.reset();
        }
        initialized = false;
    }
    
    RendererType getType() const override {
        return RendererType::Vulkan;
    }
    
    bool supportsFeature(RenderingFeature feature) const override {
        return renderPipeline ? renderPipeline->supportsFeature(feature) : false;
    }
};

} // namespace HyperEngine::Rendering::Vulkan
```

### 3.9 Создание RenderPipeline с этапами

`src/rendering/vulkan/RenderPipeline.h`:

```cpp
#pragma once
#include "HyperEngine/Rendering/RenderStages/IRenderStage.h"
#include <vector>
#include <memory>

namespace HyperEngine::Rendering::Vulkan {

/**
 * @brief Конвейер рендеринга Vulkan
 * 
 * Управляет последовательностью этапов рендеринга
 * Применяет Chain of Responsibility pattern
 */
class RenderPipeline {
private:
    std::vector<std::unique_ptr<IRenderStage>> stages;
    std::shared_ptr<IResourceManager> resourceManager;
    
public:
    explicit RenderPipeline(std::shared_ptr<IResourceManager> resourceMgr);
    
    bool initialize();
    void execute(const FrameData& frameData);
    void shutdown();
    
    bool supportsFeature(RenderingFeature feature) const;
    
    /**
     * @brief Добавить этап в конвейер
     */
    void addStage(std::unique_ptr<IRenderStage> stage);
    
    /**
     * @brief Удалить этап из конвейера
     */
    void removeStage(const std::string& stageName);

private:
    void setupDefaultStages();
};

} // namespace HyperEngine::Rendering::Vulkan
```

### 3.10 Создание конкретных этапов рендеринга

`src/rendering/vulkan/stages/PrimaryRasterizationStage.h`:

```cpp
#pragma once
#include "HyperEngine/Rendering/RenderStages/IRenderStage.h"

namespace HyperEngine::Rendering::Vulkan {

/**
 * @brief Этап первичной растеризации
 * 
 * Отвечает ТОЛЬКО за Gaussian Splatting
 */
class PrimaryRasterizationStage : public IRenderStage {
private:
    std::unique_ptr<FlashGSSplatter> splatter;
    
public:
    void execute(RenderContext& context) override;
    std::string getName() const override { return "PrimaryRasterization"; }
    bool isReady() const override;
    
    bool initialize(std::shared_ptr<IResourceManager> resourceManager);
    void shutdown();
};

/**
 * @brief Этап вторичного ray tracing
 * 
 * Отвечает ТОЛЬКО за трассировку лучей для эффектов
 */
class SecondaryRayTracingStage : public IRenderStage {
private:
    std::unique_ptr<OptiXRayTracer> rayTracer;
    
public:
    void execute(RenderContext& context) override;
    std::string getName() const override { return "SecondaryRayTracing"; }
    bool isReady() const override;
    
    bool initialize(std::shared_ptr<IResourceManager> resourceManager);
    void shutdown();
};

/**
 * @brief Этап AI деноизинга
 * 
 * Отвечает ТОЛЬКО за деноизинг эффектов
 */
class AIDenoiseStage : public IRenderStage {
private:
    std::unique_ptr<DenoiseModule> denoiser;
    
public:
    void execute(RenderContext& context) override;
    std::string getName() const override { return "AIDenoising"; }
    bool isReady() const override;
    
    bool initialize(std::shared_ptr<IResourceManager> resourceManager);
    void shutdown();
};

/**
 * @brief Этап upscaling
 * 
 * Отвечает ТОЛЬКО за увеличение разрешения
 */
class UpscalingStage : public IRenderStage {
private:
    std::unique_ptr<IUpscalingStrategy> strategy;
    
public:
    void execute(RenderContext& context) override;
    std::string getName() const override { return "Upscaling"; }
    bool isReady() const override;
    
    bool initialize(std::shared_ptr<IResourceManager> resourceManager);
    void setStrategy(std::unique_ptr<IUpscalingStrategy> newStrategy);
    void shutdown();
};

} // namespace HyperEngine::Rendering::Vulkan
```

### 3.11 Валидация этапа 3

Проверить применение SOLID принципов:

```bash
# Создать тестовый файл для проверки компиляции интерфейсов
echo '#include "HyperEngine/Rendering/IRenderer.h"
#include "HyperEngine/Rendering/RendererFactory.h"
int main() { return 0; }' > test_interfaces.cpp

# Попытаться скомпилировать
g++ -std=c++20 -I include test_interfaces.cpp -c

# Удалить тестовый файл
rm test_interfaces.cpp
```

Чек-лист этапа 3:

- [ ] Проанализированы нарушения SOLID принципов
- [ ] Созданы базовые интерфейсы (IRenderer, IResourceManager)
- [ ] Реализован Strategy Pattern для upscaling
- [ ] Создан DI контейнер
- [ ] Применен Factory Pattern для рендереров
- [ ] VulkanRenderer разделен согласно SRP
- [ ] Созданы отдельные этапы рендеринга
- [ ] Интерфейсы компилируются без ошибок

```bash
# Коммит изменений этапа 3
git add .
git commit -m "Этап 3: Рефакторинг архитектуры согласно SOLID

- Созданы базовые интерфейсы для всех компонентов
- Применен Strategy Pattern для upscaling
- Реализован DI контейнер
- Добавлен Factory Pattern для рендереров
- VulkanRenderer разделен на отдельные этапы
- Каждый класс теперь имеет единственную ответственность"
```

---

## 📋 Этап 4: Улучшение системы тестирования

### 4.1 Анализ текущего состояния тестов

```bash
# Проверить существующие тесты
find tests -name "*.cpp" -exec wc -l {} +

# Запустить существующие тесты и проанализировать покрытие
cd build-vcpkg
ctest --verbose
```

Создать `docs/refactoring/testing_analysis.md`:

```markdown
# Анализ системы тестирования

## Текущее состояние:
- ✅ Есть базовые тесты для математической библиотеки (278 строк)
- ❌ Отсутствуют тесты для системы рендеринга
- ❌ Отсутствуют интеграционные тесты
- ❌ Отсутствуют тесты производительности
- ❌ Нет mock объектов для изоляции компонентов

## Целевое состояние:
- 90%+ покрытие кода тестами
- Unit тесты для всех компонентов
- Integration тесты для критических путей
- Performance тесты для узких мест
- Mock объекты для внешних зависимостей
```

### 4.2 Создание базовой инфраструктуры тестирования

Создать `tests/TestFramework.h`:

```cpp
#pragma once
#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include "HyperEngine/Core/Console.h"

namespace HyperEngine::Testing {

/**
 * @brief Базовый класс для всех тестов HyperEngine
 */
class HyperEngineTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Инициализация консоли для тестов
        Console::initialize();
        
        // Настройка логирования для тестов
        setupTestLogging();
    }
    
    void TearDown() override {
        // Очистка после тестов
        Console::cleanup();
    }

private:
    void setupTestLogging();
};

/**
 * @brief Макросы для удобного тестирования
 */
#define EXPECT_NO_THROW_WITH_MESSAGE(statement, message) \
    try { \
        statement; \
    } catch (const std::exception& e) { \
        FAIL() << message << ": " << e.what(); \
    }

#define EXPECT_PERFORMANCE_UNDER(statement, max_milliseconds) \
    { \
        auto start = std::chrono::high_resolution_clock::now(); \
        statement; \
        auto end = std::chrono::high_resolution_clock::now(); \
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start); \
        EXPECT_LT(duration.count(), max_milliseconds) \
            << "Операция заняла " << duration.count() << "ms, ожидалось < " << max_milliseconds << "ms"; \
    }

} // namespace HyperEngine::Testing
```

### 4.3 Создание Mock объектов

Создать `tests/mocks/MockRenderer.h`:

```cpp
#pragma once
#include <gmock/gmock.h>
#include "HyperEngine/Rendering/IRenderer.h"

namespace HyperEngine::Testing::Mocks {

class MockRenderer : public Rendering::IRenderer {
public:
    MOCK_METHOD(bool, initialize, (), (override));
    MOCK_METHOD(void, renderFrame, (const Rendering::FrameData& frameData), (override));
    MOCK_METHOD(void, shutdown, (), (override));
    MOCK_METHOD(Rendering::RendererType, getType, (), (const override));
    MOCK_METHOD(bool, supportsFeature, (Rendering::RenderingFeature feature), (const override));
};

class MockResourceManager : public Rendering::IResourceManager {
public:
    MOCK_METHOD(Rendering::BufferHandle, allocateBuffer, 
                (size_t size, Rendering::BufferUsage usage), (override));
    MOCK_METHOD(Rendering::TextureHandle, createTexture, 
                (const Rendering::TextureDesc& desc), (override));
    MOCK_METHOD(void, releaseResource, (Rendering::ResourceHandle handle), (override));
    MOCK_METHOD(Rendering::MemoryStats, getMemoryStats, (), (const override));
};

class MockUpscalingStrategy : public Rendering::Upscaling::IUpscalingStrategy {
public:
    MOCK_METHOD(bool, initialize, (const Rendering::Upscaling::UpscalingConfig& config), (override));
    MOCK_METHOD(void, upscale, 
                (const Rendering::Image& input, Rendering::Image& output, 
                 const Rendering::Upscaling::UpscalingParams& params), (override));
    MOCK_METHOD(Rendering::Upscaling::UpscalingType, getType, (), (const override));
    MOCK_METHOD(bool, isSupported, (), (const override));
};

} // namespace HyperEngine::Testing::Mocks
```

### 4.4 Создание тестов для новых интерфейсов

Создать `tests/unit/rendering/RendererFactoryTest.cpp`:

```cpp
#include "TestFramework.h"
#include "HyperEngine/Rendering/RendererFactory.h"
#include "mocks/MockRenderer.h"

using namespace HyperEngine;
using namespace HyperEngine::Testing;

class RendererFactoryTest : public HyperEngineTest {
protected:
    void SetUp() override {
        HyperEngineTest::SetUp();
    }
};

TEST_F(RendererFactoryTest, CreateVulkanRenderer) {
    // Arrange
    Rendering::RendererConfig config;
    config.width = 1920;
    config.height = 1080;
    
    // Act & Assert
    EXPECT_NO_THROW_WITH_MESSAGE({
        auto renderer = Rendering::RendererFactory::createRenderer(
            Rendering::RendererType::Vulkan, config);
        EXPECT_NE(renderer, nullptr);
        EXPECT_EQ(renderer->getType(), Rendering::RendererType::Vulkan);
    }, "Создание Vulkan рендерера");
}

TEST_F(RendererFactoryTest, CheckSupportedRenderers) {
    // Act
    auto supported = Rendering::RendererFactory::getSupportedRenderers();
    
    // Assert
    EXPECT_FALSE(supported.empty());
    
    for (auto type : supported) {
        EXPECT_TRUE(Rendering::RendererFactory::isSupported(type));
    }
}

TEST_F(RendererFactoryTest, CreateUnsupportedRenderer) {
    // Arrange
    Rendering::RendererConfig config;
    
    // Act & Assert
    if (!Rendering::RendererFactory::isSupported(Rendering::RendererType::DirectX12)) {
        EXPECT_THROW(
            Rendering::RendererFactory::createRenderer(Rendering::RendererType::DirectX12, config),
            std::runtime_error
        );
    }
}
```

### 4.5 Создание интеграционных тестов

Создать `tests/integration/RenderingPipelineTest.cpp`:

```cpp
#include "TestFramework.h"
#include "HyperEngine/Rendering/RendererFactory.h"
#include "HyperEngine/Core/DependencyInjection/Container.h"

using namespace HyperEngine;
using namespace HyperEngine::Testing;

class RenderingPipelineIntegrationTest : public HyperEngineTest {
protected:
    std::unique_ptr<Core::DI::Container> container;
    std::unique_ptr<Rendering::IRenderer> renderer;
    
    void SetUp() override {
        HyperEngineTest::SetUp();
        
        // Настройка DI контейнера
        container = std::make_unique<Core::DI::Container>();
        
        // Регистрация зависимостей
        container->registerSingleton<
            Rendering::IResourceManager, 
            Rendering::Vulkan::VulkanResourceManager
        >();
        
        // Создание рендерера
        Rendering::RendererConfig config;
        config.width = 800;
        config.height = 600;
        
        renderer = Rendering::RendererFactory::createRenderer(
            Rendering::RendererType::Vulkan, config);
        
        ASSERT_NE(renderer, nullptr);
    }
    
    void TearDown() override {
        if (renderer) {
            renderer->shutdown();
        }
        HyperEngineTest::TearDown();
    }
};

TEST_F(RenderingPipelineIntegrationTest, FullInitializationAndShutdown) {
    // Act & Assert
    EXPECT_NO_THROW_WITH_MESSAGE({
        EXPECT_TRUE(renderer->initialize());
        renderer->shutdown();
    }, "Полный цикл инициализации и завершения");
}

TEST_F(RenderingPipelineIntegrationTest, RenderEmptyFrame) {
    // Arrange
    ASSERT_TRUE(renderer->initialize());
    
    Rendering::FrameData frameData;
    frameData.camera.position = Math::Vector3(0, 0, 5);
    frameData.camera.target = Math::Vector3(0, 0, 0);
    frameData.renderTargetSize = {800, 600};
    
    // Act & Assert
    EXPECT_NO_THROW_WITH_MESSAGE({
        renderer->renderFrame(frameData);
    }, "Рендеринг пустого кадра");
}

TEST_F(RenderingPipelineIntegrationTest, MultipleFrameRendering) {
    // Arrange
    ASSERT_TRUE(renderer->initialize());
    
    Rendering::FrameData frameData;
    frameData.camera.position = Math::Vector3(0, 0, 5);
    frameData.camera.target = Math::Vector3(0, 0, 0);
    frameData.renderTargetSize = {800, 600};
    
    // Act & Assert
    EXPECT_PERFORMANCE_UNDER({
        for (int i = 0; i < 10; ++i) {
            renderer->renderFrame(frameData);
        }
    }, 100); // 10 кадров должны рендериться за < 100ms
}
```

### 4.6 Создание тестов производительности

Создать `tests/performance/MathPerformanceTest.cpp`:

```cpp
#include "TestFramework.h"
#include "HyperEngine/Math/Vector3.h"
#include "HyperEngine/Math/Matrix4.h"
#include <benchmark/benchmark.h>

using namespace HyperEngine;

// Бенчмарк для операций с векторами
static void BM_Vector3CrossProduct(benchmark::State& state) {
    Math::Vector3 v1(1.0f, 2.0f, 3.0f);
    Math::Vector3 v2(4.0f, 5.0f, 6.0f);
    
    for (auto _ : state) {
        Math::Vector3 result = v1.cross(v2);
        benchmark::DoNotOptimize(result);
    }
}
BENCHMARK(BM_Vector3CrossProduct);

// Бенчмарк для операций с матрицами
static void BM_Matrix4Multiplication(benchmark::State& state) {
    Math::Matrix4 m1 = Math::Matrix4::translation(1, 2, 3);
    Math::Matrix4 m2 = Math::Matrix4::scaling(2, 2, 2);
    
    for (auto _ : state) {
        Math::Matrix4 result = m1 * m2;
        benchmark::DoNotOptimize(result);
    }
}
BENCHMARK(BM_Matrix4Multiplication);

BENCHMARK_MAIN();
```

### 4.7 Валидация этапа 4

Чек-лист этапа 4:
- [ ] Создана базовая инфраструктура тестирования
- [ ] Созданы Mock объекты для всех интерфейсов
- [ ] Написаны Unit тесты для новых компонентов
- [ ] Созданы Integration тесты для критических путей
- [ ] Настроены Performance тесты с benchmark
- [ ] Все тесты проходят успешно

```bash
# Коммит изменений этапа 4
git add .
git commit -m "Этап 4: Улучшение системы тестирования

- Создана базовая инфраструктура тестирования
- Добавлены Mock объекты для всех интерфейсов
- Созданы Unit и Integration тесты
- Настроены Performance тесты
- Добавлен сбор метрик покрытия кода"
```

---

*Часть 3 из 8 завершена. Продолжение следует в следующих частях.*
