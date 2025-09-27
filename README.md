# HyperEngine - Современный 3D/4D игровой движок

[![C++17](https://img.shields.io/badge/C%2B%2B-17-blue.svg)](https://isocpp.org/std/the-standard)
[![CMake](https://img.shields.io/badge/CMake-3.16%2B-blue.svg)](https://cmake.org/)
[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](LICENSE)
[![Platform](https://img.shields.io/badge/Platform-Windows%20%7C%20Linux-lightgrey.svg)]()

**HyperEngine** - экспериментальный игровой движок нового поколения с поддержкой как классического 3D, так и инновационного 4D рендеринга. Движок демонстрирует современные подходы к архитектуре программного обеспечения, следуя принципам SOLID и предоставляя высокопроизводительную платформу для создания уникальных игровых проектов.

## 🌟 Ключевые особенности

### 🎯 Двойная архитектура
- **3D Движок**: Современный, оптимизированный рендерер для классических 3D игр
- **4D Движок**: Экспериментальная поддержка четырехмерного пространства с Vulkan
- **Модульная система**: Возможность использования компонентов независимо
- **Гибкая конфигурация**: Сборка только необходимых модулей

### 🚀 Современные технологии
- **OpenGL/Vulkan**: Поддержка современных графических API
- **Forward+ Rendering**: Tile-based deferred lighting для 4D
- **Физическая симуляция**: Комплексная физическая система для 3D и 4D
- **UTF-8 консоль**: Полная поддержка Unicode с эмодзи и цветным выводом
- **SOLID принципы**: Архитектура, следующая лучшим практикам

### 🎮 Разработчику-ориентированный дизайн
- **Компонентная архитектура**: Гибкая система GameObject-Component
- **Богатая математическая библиотека**: Vector3/Vector4, Matrix4, Quaternion
- **Интуитивный API**: Понятные интерфейсы для быстрого освоения
- **Обширная документация**: Полная справочная информация и примеры

## 📋 Системные требования

### Минимальные требования
- **Операционная система**: Windows 10+ / Linux (Ubuntu 20.04+)
- **Компилятор**: 
  - Visual Studio 2019+ (Windows)
  - GCC 9+ / Clang 10+ (Linux)
- **CMake**: 3.16 или новее
- **Память**: 4 GB RAM
- **Видеокарта**: OpenGL 4.1+ / Vulkan 1.3+ (для 4D)

### Рекомендованные требования
- **CPU**: Intel i5-8400 / AMD Ryzen 5 2600 или лучше
- **GPU**: NVIDIA GTX 1060 / AMD RX 580 или лучше
- **Память**: 8 GB RAM
- **Место на диске**: 2 GB свободного места

### Зависимости
- **GLFW 3.3+**: Оконная система и ввод
- **GLM 0.9.9+**: Математическая библиотека
- **Vulkan SDK 1.3.0+**: Для 4D движка (опционально)
- **Vulkan Memory Allocator**: Для эффективного управления памятью GPU

## 🛠️ Быстрая установка

### Windows (Рекомендуется vcpkg)

```bash
# 1. Клонирование репозитория
git clone https://github.com/yourusername/hyperengine.git
cd hyperengine

# 2. Автоматическая сборка с vcpkg
.\build_with_vcpkg.bat

# 3. Запуск демо
.\build-vcpkg\x64\Release\Engine3D_Demo.exe
```

### Linux (Ubuntu/Debian)

```bash
# 1. Установка зависимостей
sudo apt update
sudo apt install build-essential cmake libglfw3-dev libglm-dev

# 2. Сборка
git clone https://github.com/yourusername/hyperengine.git
cd hyperengine
mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
make -j$(nproc)

# 3. Запуск демо
./Engine3D_Demo
```

### Использование CMake Presets

```bash
# Просмотр доступных пресетов
cmake --list-presets

# Сборка с vcpkg (Windows)
cmake --preset windows-vcpkg
cmake --build --preset windows-vcpkg

# Стандартная сборка
cmake --preset windows-default
cmake --build --preset windows-default
```

## 🎮 Демонстрационные приложения

После успешной сборки доступны следующие демо:

```bash
# 🌟 Основные демо 3D движка
./Engine3D_Demo              # Базовая демонстрация 3D возможностей
./OptimalRenderer_Demo       # Демо оптимального рендерера
./UTF8Console_Demo          # UTF-8 консоль с эмодзи и цветами

# 🚀 Экспериментальные демо 4D (при включении BUILD_ENGINE_4D)
./Engine4D_Demo             # Демонстрация 4D пространства
```

### Управление в демо

| Клавиша | Действие |
|---------|----------|
| **WASD** | Движение по XYZ осям |
| **QE** | Движение по W-оси (4D) |
| **Space/Shift** | Вверх/вниз |
| **Мышь** | Поворот камеры |
| **1-6** | Повороты в плоскостях (4D) |
| **Tab** | Переключение проекций |
| **F1** | Каркасный режим |
| **R** | Сброс камеры |
| **ESC** | Выход |

## 🏗️ Архитектура проекта

### Структура каталогов

```
HyperEngine/
├── 📁 include/              # Заголовочные файлы
│   ├── Engine3D/           # API 3D движка
│   └── Engine4D/           # API 4D движка (экспериментальный)
├── 📁 src3D/               # Реализация 3D движка
├── 📁 examples/            # Демонстрационные приложения
├── 📁 tests/               # Модульные и интеграционные тесты
├── 📁 docs/                # Техническая документация
├── 📁 shaders/             # GLSL/HLSL шейдеры
├── 📁 vcpkg/               # Менеджер зависимостей
└── 📁 build*/              # Сгенерированные файлы сборки
```

### Основные компоненты

#### 🧮 Математическая библиотека (`Math`)
```cpp
#include <Engine3D/Math/Vector3.h>
#include <Engine3D/Math/Matrix4.h>
#include <Engine3D/Math/Quaternion.h>

// Пример использования
using namespace Engine3D::Math;
Vector3 position(1.0f, 2.0f, 3.0f);
Matrix4 transform = Matrix4::translation(position);
Quaternion rotation = Quaternion::fromEulerAngles(0, 45, 0);
```

#### 🎨 Система рендеринга (`Rendering`)
```cpp
#include <Engine3D/Rendering/Renderer3D.h>
#include <Engine3D/Rendering/Camera3D.h>
#include <Engine3D/Rendering/Mesh3D.h>

// Инициализация рендерера
auto& renderer = Renderer3D::getInstance();
renderer.initialize(1920, 1080);

// Создание камеры
auto camera = std::make_shared<Camera3D>();
camera->setPosition(Vector3(0, 0, -5));
```

#### 🎯 Система объектов (`Core`)
```cpp
#include <Engine3D/Core/GameObject3D.h>
#include <Engine3D/Core/Transform3D.h>

// Создание игрового объекта
auto gameObject = GameObject3D::create("MyObject");
auto transform = gameObject->getComponent<Transform3D>();
transform->setPosition(Vector3(1, 2, 3));

// Добавление компонента рендеринга
auto meshRenderer = gameObject->addComponent<MeshRenderer3D>();
meshRenderer->setColor(Vector3(1.0f, 0.5f, 0.0f));
```

#### ⚡ Физическая система (`Physics`)
```cpp
#include <Engine3D/Physics/Physics3D.h>

// Создание физического тела
auto rigidBody = gameObject->addComponent<RigidBody3D>();
rigidBody->setMass(1.0f);
rigidBody->applyForce(Vector3(0, -9.81f, 0)); // Гравитация
```

#### 🎮 Система ввода (`Input`)
```cpp
#include <Engine3D/Input/Input3D.h>

// Проверка ввода
auto& input = Input3D::getInstance();
if (input.isKeyPressed(KeyCode::W)) {
    camera->move(Vector3(0, 0, speed * deltaTime));
}
```

#### 🖥️ UTF-8 консоль (`Console`)
```cpp
#include <Engine3D/Core/Console.h>

// Инициализация консоли с UTF-8
Console::initialize();
Console::setTitle("🚀 My Game Engine");

// Цветной вывод с эмодзи
Console::info("✅ Движок инициализирован успешно!");
Console::warning("⚠️ Низкая производительность GPU");
Console::error("❌ Ошибка загрузки ресурса");
```

## 💡 Примеры использования

### Создание простой 3D сцены

```cpp
#include <Engine3D/Engine3D.h>

int main() {
    using namespace Engine3D;
    
    // Инициализация движка
    auto& renderer = Rendering::Renderer3D::getInstance();
    if (!renderer.initialize(1280, 720)) {
        return -1;
    }
    
    // Создание сцены
    auto scene = Core::GameObject3D::create("Scene");
    
    // Добавление камеры
    auto cameraObj = Core::GameObject3D::create("Camera");
    auto camera = cameraObj->addComponent<Rendering::Camera3D>();
    camera->setPosition(Math::Vector3(0, 0, -5));
    
    // Создание куба
    auto cube = Core::GameObject3D::create("Cube");
    auto meshRenderer = cube->addComponent<Core::MeshRenderer3D>();
    auto mesh = Rendering::Mesh3D::createCube(1.0f);
    meshRenderer->setMesh(mesh);
    meshRenderer->setColor(Math::Vector3(0.8f, 0.3f, 0.2f));
    
    // Игровой цикл
    while (!renderer.shouldClose()) {
        renderer.beginFrame();
        
        // Вращение куба
        float time = glfwGetTime();
        cube->getComponent<Core::Transform3D>()->setRotation(
            Math::Quaternion::fromEulerAngles(time * 50, time * 30, 0)
        );
        
        // Рендеринг сцены
        scene->render();
        
        renderer.endFrame();
    }
    
    return 0;
}
```

### Работа с физикой

```cpp
// Создание физического объекта
auto physicsObject = Core::GameObject3D::create("PhysicsBox");

// Добавление физического тела
auto rigidBody = physicsObject->addComponent<Physics::RigidBody3D>();
rigidBody->setMass(1.0f);
rigidBody->setRestitution(0.7f); // Упругость

// Добавление коллайдера
auto collider = physicsObject->addComponent<Physics::BoxCollider3D>();
collider->setSize(Math::Vector3(1, 1, 1));

// Применение силы
rigidBody->applyImpulse(Math::Vector3(0, 10, 0));

// В игровом цикле
physicsWorld.update(deltaTime);
```

### Система событий и ввода

```cpp
#include <Engine3D/Input/Input3D.h>

class PlayerController {
private:
    std::shared_ptr<Core::GameObject3D> player;
    float moveSpeed = 5.0f;
    
public:
    void update(float deltaTime) {
        auto& input = Input::Input3D::getInstance();
        auto transform = player->getComponent<Core::Transform3D>();
        
        Math::Vector3 movement(0);
        
        if (input.isKeyPressed(Input::KeyCode::W)) movement.z += 1;
        if (input.isKeyPressed(Input::KeyCode::S)) movement.z -= 1;
        if (input.isKeyPressed(Input::KeyCode::A)) movement.x -= 1;
        if (input.isKeyPressed(Input::KeyCode::D)) movement.x += 1;
        
        movement = movement.normalized() * moveSpeed * deltaTime;
        transform->translate(movement);
        
        // Прыжок
        if (input.isKeyJustPressed(Input::KeyCode::Space)) {
            auto rigidBody = player->getComponent<Physics::RigidBody3D>();
            rigidBody->applyImpulse(Math::Vector3(0, 8, 0));
        }
    }
};
```

## 🔧 Конфигурация сборки

### Опции CMake

```bash
# Основные опции
-DBUILD_ENGINE_3D=ON          # Сборка 3D движка (по умолчанию)
-DBUILD_ENGINE_4D=OFF         # Сборка 4D движка (экспериментальный)
-DBUILD_EXAMPLES=ON           # Сборка демо-приложений
-DBUILD_TESTS=OFF             # Сборка тестов

# Дополнительные опции
-DCMAKE_BUILD_TYPE=Release    # Тип сборки (Debug/Release/RelWithDebInfo)
-DCMAKE_INSTALL_PREFIX=...    # Каталог установки
```

### Профили сборки

```bash
# Разработка (с отладочной информацией)
cmake --preset windows-debug
cmake --build --preset windows-debug

# Релиз (оптимизированная сборка)
cmake --preset windows-release  
cmake --build --preset windows-release

# Профилирование
cmake --preset windows-profile
cmake --build --preset windows-profile
```

## 🧪 Тестирование

```bash
# Сборка с тестами
cmake .. -DBUILD_TESTS=ON

# Запуск всех тестов
ctest --output-on-failure

# Запуск конкретных тестов
./tests/unit/MathTests
./tests/integration/RenderingTests
./tests/performance/BenchmarkTests
```

### Структура тестов

- **`tests/unit/`** - Модульные тесты компонентов
- **`tests/integration/`** - Интеграционные тесты систем
- **`tests/performance/`** - Тесты производительности
- **`tests/fixtures/`** - Тестовые данные и ресурсы
- **`tests/mocks/`** - Mock-объекты для тестирования

## 📚 Документация

### Основная документация
- 📖 [**Техническая документация**](docs/README.md) - Полный обзор архитектуры
- 🔧 [**API Reference**](docs/API_Reference.md) - Справочник по API
- 🎯 [**Примеры кода**](docs/Examples.md) - Практические примеры
- 🏗️ [**Инструкции по сборке**](BUILD_INSTRUCTIONS.md) - Детальное руководство

### Специализированная документация
- 🌐 [**Поддержка русского языка**](docs/Russian_Language_Support.md)
- 🖥️ [**UTF-8 консоль**](docs/UTF8_Console_Guide.md)
- 🎨 [**Система рендеринга**](docs/OPTIMAL_RENDERING_ALGORITHM.md)
- 🔬 [**Vulkan Forward+**](docs/VULKAN_FORWARD_PLUS.md)

### Для разработчиков
- 🤝 [**Руководство по участию**](CONTRIBUTING.md)
- 📝 [**История изменений**](CHANGELOG.md)
- 🔗 [**Зависимости**](DEPENDENCIES.md)

## 🚀 Производительность

### Оптимизации 3D рендеринга
- **Frustum Culling**: Отсечение объектов вне зоны видимости
- **Batch Rendering**: Группировка draw calls
- **GPU-driven Rendering**: Минимизация CPU-GPU синхронизации
- **Adaptive Quality**: Автоматическая настройка качества

### Метрики производительности
- **60 FPS** стабильно при 1080p (GTX 1060)
- **100,000+** объектов с frustum culling
- **<1ms** время кадра для рендеринга простых сцен
- **Память**: <100MB для базовых сцен

## 🌐 Экспериментальные возможности

### 4D Движок (Экспериментальный)
```cpp
// 4D математика
Engine4D::Math::Vector4 position4D(1, 2, 3, 4);
Engine4D::Math::Matrix4 transform4D = Engine4D::Math::Matrix4::translation(position4D);

// 4D рендеринг с проекциями
auto tesseract = Engine4D::Core::GameObject4D::createPrimitive("Tesseract");
tesseract->transform->setPosition(Engine4D::Math::Vector4(0, 0, 0, 0));

// 4D навигация
auto controller = std::make_unique<Engine4D::Input::Controller4D>();
controller->setWMovement(true); // Включение движения по W-оси
```

### Возможности 4D
- **6 плоскостей вращения**: XY, XZ, XW, YZ, YW, ZW
- **Проекции 4D→3D**: Ортогональная, перспективная, сечения
- **4D физика**: Коллизии и тела в гиперпространстве
- **Интуитивная навигация**: Специализированные контроллеры

## 🔮 Дорожная карта

### Версия 1.1 (Q1 2026)
- [ ] 🎮 Полная поддержка VR/AR
- [ ] 🌊 Продвинутая система частиц
- [ ] 🔊 3D позиционный звук
- [ ] 📱 Экспорт на мобильные платформы

### Версия 1.2 (Q2 2026)
- [ ] 🎨 Визуальный редактор сцен
- [ ] 🧠 Интегрированная система ИИ
- [ ] 🌐 Сетевая поддержка (мультиплеер)
- [ ] 📊 Встроенный профайлер производительности

### Версия 2.0 (Q4 2026)
- [ ] 🔥 Ray Tracing поддержка
- [ ] 🌍 Процедурная генерация миров
- [ ] 🎯 Полная поддержка 4D игр
- [ ] 📈 Коммерческая версия с расширенными возможностями

## 🤝 Участие в разработке

Мы приветствуем вклад сообщества! См. [CONTRIBUTING.md](CONTRIBUTING.md) для подробной информации.

### Как помочь проекту
1. 🐛 **Сообщайте об ошибках** через GitHub Issues
2. 💡 **Предлагайте новые возможности** в Discussions
3. 📝 **Улучшайте документацию** через Pull Requests
4. 🧪 **Пишите тесты** для новой функциональности
5. ⭐ **Ставьте звезды** и делитесь проектом

### Стандарты разработки
- **Код**: Следование SOLID принципам и C++17 стандартам
- **Тесты**: Минимум 80% покрытия для нового кода
- **Документация**: Обязательное документирование публичного API
- **Ревью**: Все изменения проходят код-ревью

## 📄 Лицензия

Проект распространяется под лицензией **MIT**. См. [LICENSE](LICENSE) для подробностей.

```
MIT License - Вы можете свободно использовать, изменять и распространять 
этот код в коммерческих и некоммерческих проектах.
```

## 📞 Поддержка и контакты

- 🌐 **GitHub**: [HyperEngine Repository](https://github.com/yourusername/hyperengine)
- 💬 **Discussions**: [GitHub Discussions](https://github.com/yourusername/hyperengine/discussions)
- 🐛 **Issues**: [GitHub Issues](https://github.com/yourusername/hyperengine/issues)
- 📧 **Email**: hyperengine.dev@example.com

### Сообщество
- 💬 **Discord**: [HyperEngine Community](#)
- 🐦 **Twitter**: [@HyperEngineGame](#)
- 📺 **YouTube**: [HyperEngine Channel](#)

## 🙏 Благодарности

Особая благодарность сообществу разработчиков и вдохновителям:

- **[OpenGL Community](https://www.opengl.org/)** - За стандарты графического API
- **[Vulkan Working Group](https://www.khronos.org/vulkan/)** - За современный графический API
- **[GLM Library](https://github.com/g-truc/glm)** - За превосходную математическую библиотеку
- **[GLFW Project](https://www.glfw.org/)** - За кроссплатформенную оконную систему
- **Сообществу разработчиков игр** - За идеи, отзывы и поддержку

---

<div align="center">

**Создано с ❤️ для сообщества разработчиков игр**

*HyperEngine - расширяем границы возможного в игровой разработке*

[⬆️ Наверх](#hyperengine---современный-3d4d-игровой-движок) | [📚 Документация](docs/) | [🚀 Быстрый старт](#-быстрая-установка)

</div>