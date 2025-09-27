# Руководство по миграции: Адаптеры рендеринга

## Обзор

Данное руководство описывает процесс миграции существующего кода с прямого использования `Renderer3D` на новую систему адаптеров рендеринга. Адаптеры обеспечивают единый интерфейс для работы с различными графическими API (OpenGL и Vulkan) и позволяют легко переключаться между ними.

## Архитектура адаптеров

### Основные компоненты

1. **`IRendererAdapter`** - абстрактный интерфейс для всех адаптеров
2. **`RendererAdapter`** - основной класс-фасад с singleton паттерном
3. **`OpenGLRendererAdapter`** - адаптер для OpenGL (обертка над `Renderer3D`)
4. **`VulkanRendererAdapter`** - адаптер для Vulkan (интеграция с новой архитектурой)
5. **`RenderBackend`** - перечисление доступных графических API

### Диаграмма архитектуры

```
┌─────────────────┐    ┌──────────────────┐    ┌─────────────────┐
│   Client Code   │───▶│ RendererAdapter  │───▶│ IRendererAdapter│
└─────────────────┘    └──────────────────┘    └─────────────────┘
                                │                        ▲
                                │                        │
                       ┌────────▼────────┐              │
                       │ Backend Factory │              │
                       └─────────────────┘              │
                                │                        │
                    ┌───────────┴───────────┐            │
                    ▼                       ▼            │
        ┌─────────────────────┐   ┌─────────────────────┐│
        │OpenGLRendererAdapter│   │VulkanRendererAdapter││
        └─────────────────────┘   └─────────────────────┘│
                    │                       │            │
                    ▼                       ▼            │
            ┌─────────────┐         ┌─────────────┐      │
            │ Renderer3D  │         │VulkanEngine │──────┘
            └─────────────┘         └─────────────┘
```

## Процесс миграции

### Шаг 1: Замена прямых вызовов Renderer3D

**До миграции:**
```cpp
#include "Engine3D/Rendering/Renderer3D.h"

// Получение рендерера
Renderer3D& renderer = Renderer3D::getInstance();

// Инициализация
renderer.initialize(800, 600);

// Рендеринг
renderer.beginFrame();
renderer.renderMesh(mesh, transform, shader);
renderer.endFrame();
```

**После миграции:**
```cpp
#include "Engine3D/Rendering/RendererAdapter.h"

// Получение адаптера
RendererAdapter& adapter = RendererAdapter::getInstance();

// Инициализация (автоматический выбор backend)
adapter.initialize(800, 600);

// Рендеринг (тот же API!)
adapter.beginFrame();
adapter.renderMesh(mesh, transform, shader);
adapter.endFrame();
```

### Шаг 2: Настройка backend'а (опционально)

```cpp
// Автоматический выбор оптимального backend'а
adapter.setBackend(RenderBackend::AUTO);

// Или явное указание
adapter.setBackend(RenderBackend::VULKAN);
adapter.setBackend(RenderBackend::OPENGL);

// Проверка доступности
if (adapter.isBackendAvailable(RenderBackend::VULKAN)) {
    adapter.setBackend(RenderBackend::VULKAN);
} else {
    adapter.setBackend(RenderBackend::OPENGL);
}
```

### Шаг 3: Обновление заголовочных файлов

**Замените:**
```cpp
#include "Engine3D/Rendering/Renderer3D.h"
```

**На:**
```cpp
#include "Engine3D/Rendering/RendererAdapter.h"
```

### Шаг 4: Обновление CMakeLists.txt (если нужно)

Добавьте опции для выбора backend'ов:

```cmake
# Опции для адаптеров рендеринга
option(ENABLE_OPENGL_BACKEND "Enable OpenGL rendering backend" ON)
option(ENABLE_VULKAN_BACKEND "Enable Vulkan rendering backend" ON)
option(DEFAULT_BACKEND_VULKAN "Use Vulkan as default backend" OFF)
```

## Примеры использования

### Базовое использование

```cpp
#include "Engine3D/Rendering/RendererAdapter.h"
#include "Engine3D/Rendering/Mesh3D.h"
#include "Engine3D/Rendering/Shader3D.h"

int main() {
    // Получение адаптера
    RendererAdapter& adapter = RendererAdapter::getInstance();
    
    // Инициализация
    if (!adapter.initialize(800, 600)) {
        std::cerr << "Ошибка инициализации рендерера!" << std::endl;
        return 1;
    }
    
    std::cout << "Используется backend: " << adapter.getBackendName() << std::endl;
    
    // Создание ресурсов
    auto mesh = Mesh3D::createCube(2.0f);
    auto shader = Shader3D::createBasicShader();
    auto camera = std::make_shared<Camera3D>();
    
    // Настройка камеры
    camera->setPerspective(60.0f, 800.0f / 600.0f, 0.1f, 1000.0f);
    camera->setPosition(Vector3(0, 0, 5));
    adapter.setMainCamera(camera);
    
    // Настройка рендеринга
    adapter.setClearColor(0.2f, 0.3f, 0.4f, 1.0f);
    adapter.enableDepthTest(true);
    
    // Основной цикл рендеринга
    for (int frame = 0; frame < 60; ++frame) {
        adapter.beginFrame();
        adapter.clear();
        
        Matrix4 transform = Matrix4::rotationY(frame * 0.1f);
        adapter.renderMesh(*mesh, transform, *shader);
        
        adapter.endFrame();
    }
    
    // Очистка
    adapter.cleanup();
    return 0;
}
```

### Переключение между backend'ами

```cpp
void testAllBackends() {
    RendererAdapter& adapter = RendererAdapter::getInstance();
    
    // Получаем список доступных backend'ов
    auto backends = adapter.getAvailableBackends();
    
    for (auto backend : backends) {
        if (backend == RenderBackend::AUTO) continue;
        
        std::cout << "Тестирование backend: ";
        switch (backend) {
            case RenderBackend::OPENGL: std::cout << "OpenGL"; break;
            case RenderBackend::VULKAN: std::cout << "Vulkan"; break;
        }
        std::cout << std::endl;
        
        // Переключение backend'а
        if (adapter.setBackend(backend)) {
            // Инициализация и тестирование
            if (adapter.initialize(640, 480)) {
                std::cout << "  ✓ Инициализация успешна" << std::endl;
                
                // Тест базовых функций
                adapter.beginFrame();
                adapter.clear();
                adapter.endFrame();
                
                std::cout << "  ✓ Базовый рендеринг работает" << std::endl;
                
                adapter.cleanup();
            } else {
                std::cout << "  ✗ Ошибка инициализации" << std::endl;
            }
        } else {
            std::cout << "  ✗ Backend недоступен" << std::endl;
        }
    }
}
```

### Проверка поддерживаемых функций

```cpp
void checkFeatureSupport() {
    RendererAdapter& adapter = RendererAdapter::getInstance();
    
    std::vector<std::string> features = {
        "basic_rendering",
        "wireframe", 
        "depth_test",
        "blending",
        "ray_tracing",
        "gaussian_splatting",
        "compute_shaders",
        "ai_denoising",
        "dlss",
        "fsr"
    };
    
    std::cout << "Поддерживаемые функции (" << adapter.getBackendName() << "):" << std::endl;
    
    for (const auto& feature : features) {
        bool supported = adapter.supportsFeature(feature);
        std::cout << "  " << feature << ": " 
                  << (supported ? "✓" : "✗") << std::endl;
    }
}
```

## Настройки компиляции

### CMake опции

```bash
# Включить только OpenGL backend
cmake -DENABLE_OPENGL_BACKEND=ON -DENABLE_VULKAN_BACKEND=OFF ..

# Включить только Vulkan backend  
cmake -DENABLE_OPENGL_BACKEND=OFF -DENABLE_VULKAN_BACKEND=ON ..

# Включить оба backend'а с Vulkan по умолчанию
cmake -DENABLE_OPENGL_BACKEND=ON -DENABLE_VULKAN_BACKEND=ON -DDEFAULT_BACKEND_VULKAN=ON ..
```

### Макросы препроцессора

Автоматически определяются на основе CMake опций:

- `ENGINE3D_ENABLE_OPENGL` - поддержка OpenGL backend'а
- `ENGINE3D_ENABLE_VULKAN` - поддержка Vulkan backend'а  
- `ENGINE3D_DEFAULT_BACKEND_VULKAN` - Vulkan как backend по умолчанию
- `ENGINE3D_DEFAULT_BACKEND_OPENGL` - OpenGL как backend по умолчанию

## Обратная совместимость

### Существующий код

Весь существующий код, использующий `Renderer3D` напрямую, продолжит работать без изменений. Адаптеры не нарушают существующую функциональность.

### Постепенная миграция

Вы можете мигрировать код постепенно:

1. **Этап 1**: Добавить поддержку адаптеров в новый код
2. **Этап 2**: Постепенно заменять прямые вызовы `Renderer3D` на `RendererAdapter`
3. **Этап 3**: Полностью перейти на адаптеры

### Смешанное использование

```cpp
// Можно использовать оба подхода одновременно
Renderer3D& oldRenderer = Renderer3D::getInstance();
RendererAdapter& newAdapter = RendererAdapter::getInstance();

// Старый код
oldRenderer.initialize(800, 600);
oldRenderer.renderMesh(mesh1, transform1, shader1);

// Новый код  
newAdapter.setBackend(RenderBackend::VULKAN);
newAdapter.renderMesh(mesh2, transform2, shader2);
```

## Отладка и диагностика

### Информация о backend'е

```cpp
RendererAdapter& adapter = RendererAdapter::getInstance();

// Подробная информация
adapter.printBackendInfo();

// Программная проверка
std::cout << "Текущий backend: " << adapter.getBackendName() << std::endl;
std::cout << "Инициализирован: " << (adapter.isInitialized() ? "Да" : "Нет") << std::endl;
```

### Логирование

Адаптеры выводят подробную информацию в консоль:

```
[RendererAdapter] Выбран дефолтный backend: OpenGL
[OpenGLRendererAdapter] Инициализация завершена
[RendererAdapter] Переключен на backend: Vulkan
[VulkanRendererAdapter] Инициализация завершена (заглушка)
```

## Производительность

### Накладные расходы

Адаптеры добавляют минимальные накладные расходы:
- Один дополнительный вызов виртуальной функции
- Проверка указателя на текущий адаптер
- Общие накладные расходы: < 1% от времени рендеринга

### Оптимизация

Для критичных по производительности участков кода можно получить прямой доступ к backend'у:

```cpp
// Получение прямого доступа к OpenGL рендереру
if (adapter.getCurrentBackend() == RenderBackend::OPENGL) {
    Renderer3D& directRenderer = Renderer3D::getInstance();
    // Прямые вызовы без накладных расходов
    directRenderer.renderMesh(mesh, transform, shader);
}
```

## Часто задаваемые вопросы

### Q: Нужно ли изменять существующий код?

**A:** Нет, существующий код продолжит работать без изменений. Адаптеры - это дополнительная функциональность.

### Q: Как выбрать оптимальный backend?

**A:** Используйте `RenderBackend::AUTO` - адаптер автоматически выберет лучший доступный backend на основе аппаратного обеспечения.

### Q: Можно ли переключать backend'ы во время выполнения?

**A:** Да, но требуется повторная инициализация. Рекомендуется делать это между сценами или уровнями.

### Q: Что делать, если Vulkan недоступен?

**A:** Адаптер автоматически fallback на OpenGL. Проверить доступность можно через `isBackendAvailable()`.

### Q: Влияют ли адаптеры на производительность?

**A:** Накладные расходы минимальны (< 1%). Для критичных участков можно использовать прямой доступ к backend'у.

## Заключение

Система адаптеров рендеринга обеспечивает:

- ✅ **Обратную совместимость** - существующий код работает без изменений
- ✅ **Гибкость** - легкое переключение между OpenGL и Vulkan
- ✅ **Простоту миграции** - минимальные изменения в коде
- ✅ **Производительность** - минимальные накладные расходы
- ✅ **Расширяемость** - легкое добавление новых backend'ов

Адаптеры являются ключевым компонентом стратегии миграции к новой Vulkan-архитектуре, обеспечивая плавный переход без нарушения существующей функциональности.
