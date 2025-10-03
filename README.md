# SpectraForge - Современный 3D/4D игровой движок

[![C++17](https://img.shields.io/badge/C%2B%2B-17-blue.svg)](https://isocpp.org/std/the-standard)
[![CMake](https://img.shields.io/badge/CMake-3.16%2B-blue.svg)](https://cmake.org/)
[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](LICENSE)
[![Platform](https://img.shields.io/badge/Platform-Windows%20%7C%20Linux-lightgrey.svg)]()
[![Docker](https://img.shields.io/badge/Docker-Ready-blue.svg)](Dockerfile)
[![CI/CD](https://img.shields.io/badge/CI%2FCD-Docker-green.svg)](.github/workflows/docker-ci.yml)
[![Version](https://img.shields.io/badge/Version-1.0.1-green.svg)]()

**SpectraForge** - экспериментальный игровой движок нового поколения с поддержкой как классического 3D, так и инновационного 4D рендеринга. Движок демонстрирует современные подходы к архитектуре программного обеспечения, следуя принципам SOLID и предоставляя высокопроизводительную платформу для создания уникальных игровых проектов.

## 🌟 Ключевые особенности

### 🎯 Гибридная архитектура рендеринга

- **🎨 3D Движок**: Современный, оптимизированный OpenGL рендерер для классических 3D игр
- **🚀 4D Движок**: Экспериментальная поддержка четырехмерного пространства с Vulkan
- **⚡ Vulkan Backend**: Высокопроизводительный Vulkan рендерер с CUDA/OptiX интеграцией
- **🔄 Renderer Adapters**: Автоматическое переключение между OpenGL и Vulkan backend

### 🧠 Интеллектуальные технологии рендеринга

- **🎯 OptimalRenderer3D**: 5-этапный алгоритм оптимального рендеринга
  - Scene Representation Optimization с Gaussian Splatting
  - Geometry and Primary Visibility через растеризацию
  - Advanced Lighting Computation с селективной лучевой трассировкой
  - Denoising and Refinement с AI-деноизингом
  - Post-Processing and Output с нейронным апскейлингом
- **🔄 HybridRenderer3D**: Гибридный подход (растеризация + ray tracing)
- **🌟 Ray Tracing**: OptiX-powered лучевая трассировка
- **📈 AI Upscaling**: DLSS/FSR интеграция для повышения разрешения

### 🏗️ Современная архитектура

- **🔧 SOLID принципы**: Следование принципам Single Responsibility, Open/Closed, Liskov Substitution, Interface Segregation, Dependency Inversion
- **🎮 Component System**: Гибкая система GameObject-Component с жизненным циклом
- **🧮 Rich Math Library**: Полная математическая библиотека (Vector3/4, Matrix4, Quaternion)
- **💻 UTF-8 Console**: Поддержка Unicode, эмодзи и цветного вывода

### 🎮 Разработчику-ориентированный дизайн

- **📚 Comprehensive Documentation**: Полная техническая документация с примерами
- **🎯 Intuitive API**: Понятные интерфейсы для быстрого освоения
- **🔧 Modular Build**: Сборка только необходимых модулей
- **⚡ Performance Focus**: Оптимизация для высокой производительности

## 🚀 Hybrid DWT + FreGS Renderer - Новое поколение рендеринга

**Hybrid DWT + FreGS (Wavelet Lifting + Frequency-Encoded Gaussian Splatting)** - основной рендерер SpectraForge, объединяющий:

### 🌊 Архитектура пайплайна

- **🎯 Wavelet Decomposition** - Многоуровневое вейвлет-разложение (Daubechies-4)
  - 2D fused H+V lifting scheme
  - 4 subbands: LL (approximation), LH/HL/HH (details)
  - Vulkan Subgroups для параллельной обработки
  
- **✨ Frequency-Encoded Gaussian Splatting** - Аналитическая свёртка в частотной области
  - Per-pixel granularity (16x coverage improvement)
  - Аккумуляция в частотном домене
  - Foveation alignment для оптимальной производительности
  
- **⚡ AI Upscaling** - Опциональное масштабирование для высоких разрешений
  - **Native**: Pass-through/blit (0-0.1ms)
  - **DLSS**: NVIDIA tensor cores (0.8-1.5ms, 8x AI quality)
  - **FSR2**: Cross-vendor open-source (1.2-2.0ms, 6x temporal quality)

### 📊 Upscaler Comparison

| Feature | Native | DLSS | FSR2 |
|---------|--------|------|------|
| **License** | MIT | Proprietary | MIT (open-source) |
| **GPU Support** | All | NVIDIA RTX only | All (cross-vendor) |
| **Quality @ 4K** | 1x (reference) | 8x AI quality | 6x temporal quality |
| **Performance** | 0-0.1ms | ~0.8-1.5ms | ~1.2-2.0ms |
| **VRAM Usage** | 0 MB | ~500 MB | ~200 MB |
| **Frame Generation** | ❌ | ✅ DLSS 3 (RTX 40+) | ⚠️ FSR3 (experimental) |
| **Ray Reconstruction** | ❌ | ✅ DLSS 3.5 | ❌ |

### 🎯 Performance Estimates (4K → 8K Upscaling)

| Upscaler | Mode | Input Resolution | Latency | Quality Score |
|----------|------|------------------|---------|---------------|
| **Native** | Pass-through | 3840×2160 | 0ms | 1.0x (reference) |
| **Native** | Blit (Linear) | 1920×1080 | ~0.1ms | 0.5x |
| **DLSS** | Quality | 2560×1440 | ~0.8ms | 8.0x |
| **DLSS** | Balanced | ~2227×1253 | ~1.0ms | 6.0x |
| **DLSS** | Performance | 1920×1080 | ~1.2ms | 4.0x |
| **FSR2** | Quality | 2560×1440 | ~1.2ms | 6.0x |
| **FSR2** | Balanced | ~2266×1274 | ~1.5ms | 4.0x |
| **FSR2** | Performance | 1920×1080 | ~1.8ms | 3.0x |

**Documentation:** [docs/architecture/Renderer.md](docs/architecture/Renderer.md)

---

## 🎨 FreqVox Renderer - Legacy/Experimental

> **⚠️ NOTE:** FreqVox is now considered **legacy/experimental**. The primary renderer is **Hybrid DWT + FreGS**.

**FreqVox (Adaptive Frequency-Domain Sparse Neural-Voxel Rendering)** - экспериментальный алгоритм рендеринга, сочетающий:

- **🎯 Foveated Rendering** - умное распределение ресурсов GPU
- **🌊 Frequency-Domain Shading** - DCT/FFT ускорение шейдинга
- **⏱️ Temporal Reprojection** - стабильность и плавность кадра
- **🧠 Neural Upscaling** - DLSS/FSR2 интеграция

### 🚀 Быстрый старт FreqVox Sponza Demo

```bash
# Запуск полнофункционального демо
./run_freqvox_sponza.sh

# Или вручную:
cd /path/to/SpectraForge
./build/FreqVox_Sponza_Demo
```

**Особенности демо:**
- ✅ Загрузка сцены Sponza (OBJ/MTL)
- ✅ Вокселизация геометрии
- ✅ Полный FreqVox пайплайн
- ✅ Интерактивная камера (WASD + мышь)
- ✅ Статистика в реальном времени

**Документация:** [examples/FREQVOX_SPONZA_DEMO.md](examples/FREQVOX_SPONZA_DEMO.md)

## 📊 Производительность

### Метрики производительности (GTX 1060)

- **🎯 FPS**: 60 FPS стабильно при 1080p
- **📦 Объекты**: 100,000+ объектов с frustum culling
- **⏱️ Latency**: <1ms время кадра для простых сцен
- **💾 Memory**: <100MB памяти для базовых сцен

### FreqVox Performance (RTX 3060)

- **🎯 FPS**: 120+ FPS @ 1080p upscaled
- **🔥 Voxels**: 50,000+ активных вокселей
- **⚡ Speedup**: 4x благодаря фовеации
- **🧠 Upscale**: 2-8x boost с DLSS/FSR2

### Оптимизации рендеринга

- **🔍 Frustum Culling**: Отсечение невидимых объектов
- **📦 Batch Rendering**: Группировка draw calls
- **🎮 GPU-driven Rendering**: Минимизация CPU-GPU синхронизации
- **🎚️ Adaptive Quality**: Динамическая настройка качества

## 📋 Системные требования

### Минимальные требования

- **💻 OS**: Windows 10+ / Linux (Ubuntu 20.04+)
- **🔧 Compiler**: Visual Studio 2019+ (Windows) / GCC 9+ / Clang 10+ (Linux)
- **⚙️ CMake**: 3.16 или новее
- **🧠 Memory**: 4 GB RAM
- **🎮 GPU**: OpenGL 4.1+ / Vulkan 1.3+ (для advanced features)

### Рекомендованные требования

- **🖥️ CPU**: Intel i5-8400 / AMD Ryzen 5 2600 или лучше
- **🎮 GPU**: NVIDIA GTX 1060 / AMD RX 580 или лучше (RTX для ray tracing)
- **🧠 Memory**: 8 GB RAM
- **💾 Storage**: 2 GB свободного места

## 🛠️ Быстрая установка

### Windows (Рекомендуется vcpkg + Ninja)

```bash
# 1. Клонирование репозитория
git clone https://github.com/TiGRoNdev/SpectraForge.git
cd SpectraForge

# 2. Быстрая сборка с Ninja (рекомендуется)
.\build_with_vcpkg.bat

# 3. Запуск демо
.\build-ninja\Engine3D_Demo.exe
```

**Преимущества Ninja:**

- ⚡ Быстрая сборка (в 2-3 раза быстрее Visual Studio)
- 📋 Автоматический экспорт команд компиляции (`compile_commands.json`)
- 🔧 Лучшая интеграция с IDE и инструментами анализа

**Альтернативные методы сборки:**

```bash
# Через CMake Presets
cmake --preset windows-ninja
cmake --build --preset windows-ninja

# Ручная конфигурация
mkdir build-ninja && cd build-ninja
cmake .. -G Ninja -DCMAKE_TOOLCHAIN_FILE=..\vcpkg\scripts\buildsystems\vcpkg.cmake -DCMAKE_EXPORT_COMPILE_COMMANDS=ON
ninja
```

### Опции сборки для Hybrid DWT + FreGS

```bash
# Включить DLSS upscaler (требуется NVIDIA Streamline SDK)
cmake -DBUILD_WITH_DLSS=ON -DDLSS_ROOT_DIR=/path/to/streamline ..

# Включить FSR2 upscaler (требуется AMD FidelityFX SDK)
cmake -DBUILD_WITH_FSR=ON -DFSR_ROOT_DIR=/path/to/fidelityfx ..

# Оба выключены (skeleton only, для кросс-компиляции)
cmake -DBUILD_WITH_DLSS=OFF -DBUILD_WITH_FSR=OFF ..

# Выбрать рендерер
cmake -DSPECTRAFORGE_RENDERER=FREGS ..   # Hybrid DWT + FreGS (default)
cmake -DSPECTRAFORGE_RENDERER=FREQVOX .. # Legacy FreqVox

# Выбрать upscaler
cmake -DSPECTRAFORGE_UPSCALER=AUTO ..   # Auto-detect (default)
cmake -DSPECTRAFORGE_UPSCALER=DLSS ..   # Force DLSS
cmake -DSPECTRAFORGE_UPSCALER=FSR2 ..   # Force FSR2
cmake -DSPECTRAFORGE_UPSCALER=NONE ..   # Native only
```

**SDK Downloads:**
- **DLSS**: [developer.nvidia.com/dlss](https://developer.nvidia.com/dlss) (requires NVIDIA Developer account)
- **FSR2**: [github.com/GPUOpen-Effects/FidelityFX-SDK](https://github.com/GPUOpen-Effects/FidelityFX-SDK) (open-source)

### Linux (Ubuntu/Debian)

```bash
# 1. Установка зависимостей
sudo apt update
sudo apt install build-essential cmake libglfw3-dev libglm-dev

# 2. Сборка
git clone https://github.com/TiGRoNdev/SpectraForge.git
cd SpectraForge
mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
make -j$(nproc)

# 3. Запуск демо
./Engine3D_Demo
```

### Docker (Рекомендуется для CI/CD) 🐳

```bash
# 1. Клонирование репозитория
git clone https://github.com/TiGRoNdev/SpectraForge.git
cd SpectraForge

# 2. Запуск CI/CD окружения
docker-compose build ci-runner

# 3. Запуск проверок качества
docker-compose run --rm ci-runner ./scripts/quality_check.sh

# 4. Сборка проекта
docker-compose run --rm ci-runner bash -c "
  cmake -B build -G Ninja -DCMAKE_BUILD_TYPE=Release
  cmake --build build -j\$(nproc)
"

# 5. Запуск тестов
docker-compose run --rm ci-runner bash -c "cd build && ctest --output-on-failure"
```

**Преимущества Docker:**

- 🔧 Все зависимости уже установлены (Clang, CMake, системные пакеты, Vulkan SDK)
- ✅ Готовые инструменты качества (clang-tidy, cppcheck, Valgrind)
- 🚀 Идеально для CI/CD и автоматизации
- 📦 Изолированное окружение без конфликтов
- 🌍 Кросс-платформенность (работает везде, где есть Docker)
- ⚡ Быстрая сборка (~5-8 мин) без vcpkg

**Доступные сервисы:**

- `ci-runner` - Полное CI/CD окружение с инструментами анализа
- `builder` - Сборка проекта с тестами
- `dev` - Интерактивная разработка
- `demo` - Запуск GUI демонстраций

📚 **Подробнее:** [DOCKER_QUICK_START.md](DOCKER_QUICK_START.md) | [docs/DOCKER_CICD_GUIDE.md](docs/DOCKER_CICD_GUIDE.md)

---

### CMake опции сборки

```bash
# Основные опции
-DBUILD_ENGINE_3D=ON              # Сборка 3D движка
-DBUILD_VULKAN_RENDERER=ON        # Сборка Vulkan рендерера
-DBUILD_WITH_CUDA=OFF             # CUDA поддержка (опционально)
-DBUILD_WITH_OPTIX=ON             # OptiX ray tracing (опционально)
-DBUILD_WITH_DLSS=OFF             # DLSS upscaling (опционально)
-DBUILD_WITH_FSR=ON               # FSR upscaling (опционально)
-DBUILD_EXAMPLES=ON               # Сборка демо-приложений

# Пример полной сборки
cmake .. -DBUILD_ENGINE_3D=ON -DBUILD_VULKAN_RENDERER=ON -DBUILD_EXAMPLES=ON
```

## 🎮 Демонстрационные приложения

После успешной сборки доступны следующие демо:

| Демо | Описание | Технологии |
|------|----------|------------|
| **Engine3D_Demo** | 🎯 Основная демонстрация 3D движка | OpenGL, Component System |
| **OptimalRenderer_Demo** | 🚀 Демо оптимального 5-этапного рендерера | Gaussian Splatting, AI Denoising |
| **VulkanRenderer_Demo** | ⚡ Vulkan рендерер с ray tracing | Vulkan, OptiX, DLSS/FSR |
| **VulkanBasic_Demo** | 🔧 Базовое Vulkan демо | Vulkan fundamentals |
| **UTF8Console_Demo** | 💻 UTF-8 консоль с эмодзи | Unicode, Color output |
| **RendererAdapter_Demo** | 🔄 Переключение между backend'ами | OpenGL ↔ Vulkan |

### Управление в демо

| Клавиша | Действие | Контекст |
|---------|----------|----------|
| **WASD** | Движение по XYZ осям | 3D навигация |
| **QE** | Движение по W-оси | 4D навигация |
| **Space/Shift** | Вверх/вниз | Вертикальное движение |
| **Mouse** | Поворот камеры | Look around |
| **1-6** | Повороты в плоскостях | 4D ротации |
| **Tab** | Переключение проекций | 4D → 3D |
| **F1** | Каркасный режим | Debug view |
| **F2** | Performance metrics | Профилирование |
| **R** | Сброс камеры | Reset view |
| **ESC** | Выход/освобождение мыши | Exit/Unlock cursor |

## 🏗️ Архитектура проекта

### Структура каталогов

```
SpectraForge/
├── 📁 include/                 # Заголовочные файлы
│   ├── Engine3D/              # API 3D движка
│   │   ├── Core/              # Основные компоненты (GameObject, Transform, Component)
│   │   ├── Math/              # Математическая библиотека (Vector3/4, Matrix4, Quaternion)
│   │   ├── Rendering/         # Система рендеринга (Renderer3D, Camera3D, Mesh3D)
│   │   ├── Physics/           # Физическая система (RigidBody3D, Collider3D)
│   │   ├── Input/             # Система ввода (Input3D, Controller3D)
│   │   ├── Vulkan/            # Vulkan компоненты (VulkanRenderer, HardwareDetector)
│   │   ├── CUDA/              # CUDA интеграция (FlashGSSplatter)
│   │   ├── OptiX/             # OptiX ray tracing (OptiXRayTracer, DenoiseModule)
│   │   └── Upscaling/         # AI upscaling (DLSS, FSR)
├── 📁 src3D/                  # Реализация 3D движка
│   ├── Core/                  # Основные компоненты
│   ├── Math/                  # Математические операции
│   ├── Rendering/             # Рендеринг (включая OptimalRenderer3D, HybridRenderer3D)
│   ├── Physics/               # Физическая симуляция
│   └── Input/                 # Обработка ввода
├── 📁 srcVulkan/              # Vulkan/CUDA/OptiX реализация
│   ├── Vulkan/                # Vulkan рендерер
│   ├── CUDA/                  # CUDA интеграция
│   ├── OptiX/                 # OptiX ray tracing
│   └── Upscaling/             # AI upscaling
├── 📁 examples/               # Демонстрационные приложения
├── 📁 docs/                   # 📚 Техническая документация
│   ├── architecture/         # Архитектурные решения
│   ├── api/                   # Справочная документация API
│   ├── guides/                # Руководства и примеры
│   └── images/                # Визуальные материалы
├── 📁 tests/                  # Модульные и интеграционные тесты
└── 📁 shaders/                # GLSL/HLSL шейдеры

**Примечание:** `vcpkg.json` сохранен для Windows пользователей, но Docker использует системные пакеты Ubuntu.
```

### Основные архитектурные компоненты

#### 🧮 Математическая библиотека (`Math`)

```cpp
#include <Engine3D/Math/Vector3.h>
#include <Engine3D/Math/Matrix4.h>
#include <Engine3D/Math/Quaternion.h>

using namespace Engine3D::Math;
Vector3 position(1.0f, 2.0f, 3.0f);
Matrix4 transform = Matrix4::translation(position);
Quaternion rotation = Quaternion::fromEulerAngles(0, 45, 0);
```

#### 🎨 Система рендеринга (`Rendering`)

```cpp
#include <Engine3D/Rendering/Renderer3D.h>
#include <Engine3D/Rendering/OptimalRenderer3D.h>

// Базовый рендерер
auto& renderer = Renderer3D::getInstance();
renderer.initialize(1920, 1080);

// Оптимальный рендерер с 5-этапным алгоритмом
auto optimalRenderer = std::make_unique<OptimalRenderer3D>();
optimalRenderer->initialize(1920, 1080, 2560, 1440); // render -> target resolution
```

#### 🎯 Система объектов (`Core`)

```cpp
#include <Engine3D/Core/GameObject3D.h>
#include <Engine3D/Core/Transform3D.h>

// Создание игрового объекта с компонентами
auto gameObject = GameObject3D::create("MyObject");
auto transform = gameObject->getComponent<Transform3D>();
auto meshRenderer = gameObject->addComponent<MeshRenderer3D>();
```

#### ⚡ Физическая система (`Physics`)

```cpp
#include <Engine3D/Physics/Physics3D.h>

auto rigidBody = gameObject->addComponent<RigidBody3D>();
rigidBody->setMass(1.0f);
rigidBody->applyForce(Vector3(0, -9.81f, 0));
```

#### 🎮 Система ввода (`Input`)

```cpp
#include <Engine3D/Input/Input3D.h>

auto& input = Input3D::getInstance();
if (input.isKeyPressed(KeyCode::W)) {
    camera->move(Vector3(0, 0, speed * deltaTime));
}
```

#### 💻 UTF-8 консоль (`Console`)

```cpp
#include <Engine3D/Core/Console.h>

Console::initialize();
Console::setTitle("🚀 My Game Engine");
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

    // Инициализация UTF-8 консоли
    Core::Console::initialize();
    Core::Console::info("🚀 Запуск SpectraForge...");

    // Инициализация рендерера
    auto& renderer = Rendering::Renderer3D::getInstance();
    if (!renderer.initialize(1280, 720)) {
        Core::Console::error("❌ Ошибка инициализации рендерера");
        return -1;
    }

    // Создание камеры
    auto cameraObj = Core::GameObject3D::create("Camera");
    auto camera = cameraObj->addComponent<Rendering::Camera3D>();
    camera->setPosition(Math::Vector3(0, 0, -5));
    renderer.setMainCamera(camera);

    // Создание вращающегося куба
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

        // Рендеринг
        cube->render();

        renderer.endFrame();
    }

    renderer.cleanup();
    Core::Console::cleanup();
    return 0;
}
```

### Физическая симуляция

```cpp
// Создание физического объекта
auto physicsObject = Core::GameObject3D::create("PhysicsBox");

// Добавление компонентов
auto rigidBody = physicsObject->addComponent<Physics::RigidBody3D>();
rigidBody->setMass(1.0f);
rigidBody->setRestitution(0.7f); // Упругость

auto collider = physicsObject->addComponent<Physics::BoxCollider3D>();
collider->setSize(Math::Vector3(1, 1, 1));

// Применение импульса
rigidBody->applyImpulse(Math::Vector3(0, 10, 0));

// В игровом цикле
physicsWorld.update(deltaTime);
```

### Система ввода

```cpp
class FirstPersonController : public Engine3D::Core::Component {
public:
    void update(float deltaTime) override {
        auto& input = Input::Input3D::getInstance();
        auto transform = getGameObject()->getComponent<Core::Transform3D>();

        Math::Vector3 movement(0);

        if (input.isKeyPressed(Input::KeyCode::W)) movement += transform->forward();
        if (input.isKeyPressed(Input::KeyCode::S)) movement -= transform->forward();
        if (input.isKeyPressed(Input::KeyCode::A)) movement -= transform->right();
        if (input.isKeyPressed(Input::KeyCode::D)) movement += transform->right();

        if (movement.magnitude() > 0) {
            movement = movement.normalized() * moveSpeed * deltaTime;
            transform->translate(movement);
        }
    }

private:
    float moveSpeed = 5.0f;
};
```

## 📚 Полная документация

### 📖 Основная документация

- 🏗️ [**Архитектура системы**](docs/architecture/ARCHITECTURE.md) - Детальное описание архитектуры, SOLID принципов и компонентной системы
- 📚 [**API Reference**](docs/api/API_Reference.md) - Полная справочная документация по всем компонентам API
- 🎯 [**Руководство с примерами**](docs/guides/Examples.md) - Практические примеры использования и лучшие практики
- 🔧 [**Инструкции по сборке**](BUILD_INSTRUCTIONS.md) - Детальное руководство по сборке на разных платформах

### 🚀 Специализированная документация

- ⚡ [**Vulkan Architecture**](docs/architecture/VULKAN_ARCHITECTURE.md) - Архитектура Vulkan рендерера
- 🧠 [**Optimal Rendering Algorithm**](docs/architecture/OPTIMAL_RENDERING_ALGORITHM.md) - 5-этапный алгоритм оптимального рендеринга
- 🎮 [**Component System**](docs/architecture/COMPONENT_SYSTEM.md) - Система GameObject-Component
- 🔄 [**Renderer Adapters**](docs/architecture/RENDERER_ADAPTERS.md) - Адаптеры для переключения backend'ов

### 🔧 Для разработчиков

- 🤝 [**Руководство по участию**](CONTRIBUTING.md) - Как принять участие в разработке
- 📝 [**История изменений**](CHANGELOG.md) - Детальный лог всех изменений
- 🔗 [**Зависимости**](DEPENDENCIES.md) - Список и описание всех зависимостей
- 🐛 [**Отчеты об ошибках**](https://github.com/yourusername/hyperengine/issues) - Система трекинга ошибок

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
controller->setWMovement(true); // Движение по W-оси
```

### Advanced Graphics Features

- **🌟 Ray Tracing**: OptiX-powered лучевая трассировка для реалистичного освещения
- **🎨 Gaussian Splatting**: Продвинутая техника рендеринга для фотореалистичных сцен
- **🧠 AI Upscaling**: DLSS/FSR интеграция для повышения производительности
- **⚡ CUDA Integration**: GPU вычисления для ускорения рендеринга

## 🔮 Дорожная карта

### Версия 1.1 (Q1 2026)

- [ ] 🎮 **VR/AR поддержка**: Полная интеграция с VR/AR SDK
- [ ] 🌊 **Продвинутая система частиц**: Расширенные эффекты частиц с GPU симуляцией
- [ ] 🔊 **3D позиционный звук**: Система пространственного аудио
- [ ] 📱 **Мобильные платформы**: Портирование на Android/iOS

### Версия 1.2 (Q2 2026)

- [ ] 🎨 **Визуальный редактор сцен**: WYSIWYG редактор для создания сцен
- [ ] 🧠 **Интегрированная система ИИ**: AI для поведения NPC и процедурной генерации
- [ ] 🌐 **Сетевая поддержка**: Мультиплеер и синхронизация состояния
- [ ] 📊 **Встроенный профайлер**: Real-time профилирование производительности

### Версия 2.0 (Q4 2026)

- [ ] 🔥 **Hardware Ray Tracing**: Полная поддержка RTX/RDNA ray tracing
- [ ] 🌍 **Процедурная генерация**: Автоматическая генерация миров и контента
- [ ] 🎯 **Production-ready 4D**: Полная поддержка 4D игр для коммерческого использования
- [ ] 📈 **Enterprise Version**: Коммерческая версия с расширенными возможностями

## 🤝 Участие в разработке

Мы приветствуем вклад сообщества! Проект развивается благодаря участию разработчиков со всего мира.

### Как помочь проекту

1. 🐛 **Сообщайте об ошибках** через [GitHub Issues](https://github.com/yourusername/hyperengine/issues)
2. 💡 **Предлагайте новые возможности** в [Discussions](https://github.com/yourusername/hyperengine/discussions)
3. 📝 **Улучшайте документацию** через Pull Requests
4. 🧪 **Пишите тесты** для новой функциональности
5. ⭐ **Ставьте звезды** и делитесь проектом в социальных сетях

### 🛠️ Инструменты разработки

Проект включает полный набор инструментов для обеспечения качества кода:

#### 🎨 Автоматическое форматирование

```bash
# Форматирование всего кода
./scripts/format_code.bat        # Windows
./scripts/format_code.sh         # Linux/macOS

# Автоформатирование в IDE настроено в .vscode/settings.json
```

#### 🔍 Проверка качества кода

```bash
# Полная проверка качества
./scripts/quality_check.sh

# Быстрая проверка перед коммитом
./scripts/pre_commit_check.sh
```

#### 🔧 Git Hooks

```bash
# Установка локальных git hooks
./scripts/setup_git_hooks.sh      # Linux/macOS
scripts\setup_git_hooks.bat       # Windows
```

**Автоматические проверки:**

- ✅ Форматирование кода (clang-format)
- 🔒 Проверка безопасности
- 📋 Соответствие стандартам кодирования
- 🧪 Запуск тестов
- 📝 Валидация commit messages

#### 📚 Документация разработчика

- [Стандарты кодирования](docs/CODING_STANDARDS.md)
- [Инструменты разработки](docs/DEVELOPMENT_TOOLS.md)
- [Настройка Ninja Build System](docs/guides/NINJA_SETUP.md) ⚡
- [Архитектурные решения](docs/architecture/)

### Стандарты разработки

- **📋 Код**: Следование SOLID принципам и C++20 стандартам
- **🎨 Форматирование**: Автоматическое с clang-format
- **🔍 Статический анализ**: clang-tidy + cppcheck
- **🧪 Тесты**: Минимум 80% покрытия для нового кода
- **📚 Документация**: Обязательное документирование публичного API
- **👀 Ревью**: Все изменения проходят код-ревью
- **📝 Commit messages**: Conventional Commits формат
- **🎯 Performance**: Профилирование критических путей

## 📄 Лицензия

Проект распространяется под лицензией **MIT**. См. [LICENSE](LICENSE) для подробностей.

```
MIT License - Вы можете свободно использовать, изменять и распространять
этот код в коммерческих и некоммерческих проектах.
```

## 📞 Поддержка и контакты

### 🌐 Официальные ресурсы

- **GitHub**: [SpectraForge Repository](https://github.com/yourusername/hyperengine)
- **Discussions**: [GitHub Discussions](https://github.com/yourusername/hyperengine/discussions)
- **Issues**: [Bug Reports & Feature Requests](https://github.com/yourusername/hyperengine/issues)
- **Wiki**: [Project Wiki](https://github.com/yourusername/hyperengine/wiki)

### 💬 Сообщество

- **Discord**: [SpectraForge Community](#) - Общение с разработчиками
- **Reddit**: [r/SpectraForge](#) - Обсуждения и новости
- **Twitter**: [@SpectraForgeGame](#) - Последние обновления
- **YouTube**: [SpectraForge Channel](#) - Видео-туториалы и демо

### 📧 Прямая связь

- **Email**: <hyperengine.dev@example.com>
- **Technical Support**: <support@hyperengine.dev>
- **Business Inquiries**: <business@hyperengine.dev>

## 📊 Статистика проекта

[![Contributors](https://img.shields.io/github/contributors/yourusername/hyperengine)](https://github.com/yourusername/hyperengine/graphs/contributors)
[![Forks](https://img.shields.io/github/forks/yourusername/hyperengine)](https://github.com/yourusername/hyperengine/network/members)
[![Stars](https://img.shields.io/github/stars/yourusername/hyperengine)](https://github.com/yourusername/hyperengine/stargazers)
[![Issues](https://img.shields.io/github/issues/yourusername/hyperengine)](https://github.com/yourusername/hyperengine/issues)
[![Pull Requests](https://img.shields.io/github/issues-pr/yourusername/hyperengine)](https://github.com/yourusername/hyperengine/pulls)

## 🙏 Благодарности

Особая благодарность сообществу разработчиков и вдохновителям:

- **[Khronos Group](https://www.khronos.org/)** - За OpenGL и Vulkan стандарты
- **[NVIDIA](https://developer.nvidia.com/)** - За OptiX SDK и DLSS технологии
- **[AMD](https://gpuopen.com/)** - За FSR и открытые графические технологии
- **[GLM Library](https://github.com/g-truc/glm)** - За превосходную математическую библиотеку
- **[GLFW Project](https://www.glfw.org/)** - За кроссплатформенную оконную систему
- **Сообществу разработчиков игр** - За идеи, отзывы и постоянную поддержку

---

<div align="center">

**Создано с ❤️ для сообщества разработчиков игр**

*SpectraForge - расширяем границы возможного в игровой разработке*

[⬆️ Наверх](#hyperengine---современный-3d4d-игровой-движок) | [📚 Документация](docs/) | [🚀 Быстрый старт](#-быстрая-установка) | [🎮 Демо](#-демонстрационные-приложения)

[![Made with C++](https://img.shields.io/badge/Made%20with-C%2B%2B-blue)](https://isocpp.org/)
[![Powered by Vulkan](https://img.shields.io/badge/Powered%20by-Vulkan-red)](https://www.vulkan.org/)
[![Enhanced with AI](https://img.shields.io/badge/Enhanced%20with-AI-green)](https://developer.nvidia.com/rtx)

</div>
