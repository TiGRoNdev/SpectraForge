# 4D Game Engine - Vulkan Forward+ Edition

Экспериментальный игровой движок для создания игр в четырехмерном пространстве с современным Vulkan API и технологией Forward+ рендеринга. Движок расширяет концепции 3D игровых движков, добавляя поддержку четвертого пространственного измерения (W-координата) для создания уникальных игровых механик и визуальных эффектов.

## 🚀 Особенности

### Современная графика
- **Vulkan API**: Низкоуровневый доступ к GPU для максимальной производительности  
- **Forward+ Rendering**: Tile-based deferred lighting для эффективной обработки множества источников света
- **Compute Shaders**: GPU-ускоренный light culling и обработка данных
- **PBR Materials**: Physically-based rendering с поддержкой материалов

### 4D Функциональность
- **4D Математика**: Полная поддержка 4D векторов, матриц и кватернионов
- **4D Рендеринг**: Проекции 4D→3D→2D с поддержкой сечений и перспективы  
- **4D Физика**: Коллизии, физические тела и система частиц в 4D пространстве
- **4D Навигация**: Интуитивное управление в гиперпространстве

### Архитектура и UI
- **GUI Система**: Полнофункциональный графический интерфейс с поддержкой различных компонентов и русского языка
- **Компонентная архитектура**: Гибкая система объектов и компонентов
- **Кроссплатформенность**: Windows, Linux (планируется)

## 📋 Требования

- **C++17** компилятор  
- **CMake 3.16+**
- **Vulkan SDK 1.3.0+** - [Скачать здесь](https://vulkan.lunarg.com/)
- **GLFW 3.3+**
- **GLM 0.9.9+**
- **Vulkan Memory Allocator**

## 🛠️ Быстрый старт

### Установка зависимостей

#### Windows (Рекомендуется)
```bash
# Через vcpkg
git clone https://github.com/Microsoft/vcpkg.git
cd vcpkg
.\bootstrap-vcpkg.bat
.\vcpkg install glfw3:x64-windows glew:x64-windows

# Или используйте готовый скрипт
.\build_with_vcpkg.bat
```

#### Linux (Ubuntu)
```bash
sudo apt install build-essential cmake libgl1-mesa-dev libglfw3-dev libglew-dev
```

#### macOS
```bash
brew install cmake glfw glew
```

### Сборка

#### Windows с vcpkg
```bash
git clone <repository-url>
cd 4DEngine
.\build_with_vcpkg.bat
```

#### Стандартная сборка
```bash
git clone <repository-url>
cd 4DEngine
mkdir build && cd build
cmake ..
cmake --build . --config Release
```

#### Использование CMake Presets
```bash
# С vcpkg
cmake --preset windows-vcpkg
cmake --build --preset windows-vcpkg

# Стандартная
cmake --preset windows-default
cmake --build --preset windows-default
```

### Решение проблем сборки

Если возникают проблемы с зависимостями, см. [BUILD_INSTRUCTIONS.md](BUILD_INSTRUCTIONS.md) для подробных инструкций.

### Запуск демо

```bash
# Основная 4D демонстрация
./Engine4D_Demo

# GUI демонстрация (полная версия)
./Engine4D_GUI_Demo

# Простой тестер GUI
./Engine4D_GUI_Simple

# Русскоязычная демонстрация GUI
./Engine4D_GUI_Russian
```

## 🎮 Управление

- **WASD** - движение в 3D пространстве
- **QE** - движение в W-измерении
- **Space/Shift** - движение вверх/вниз
- **Мышь** - поворот камеры
- **1-6** - повороты в различных плоскостях (XY, XZ, XW, YZ, YW, ZW)
- **Tab** - переключение режима сечения
- **F1** - каркасный режим
- **R** - сброс вида
- **ESC** - выход

## 📚 Документация

- [Техническая документация](docs/README.md)
- [API Reference](docs/API_Reference.md)
- [GUI Система](docs/GUI_System.md)
- [Поддержка русского языка](docs/Russian_Language_Support.md)
- [Примеры использования](docs/Examples.md)
- [Инструкции по сборке](BUILD_INSTRUCTIONS.md)

## 🏗️ Архитектура

### Основные компоненты

1. **Математическая библиотека** (`Engine4D::Math`)
   - 4D векторы и матрицы
   - 4D кватернионы для поворотов
   - Проекции 4D→3D→2D

2. **Система рендеринга** (`Engine4D::Rendering`)
   - 4D меши и геометрия
   - Шейдеры для 4D объектов
   - Камера с поддержкой 4D навигации

3. **Физическая система** (`Engine4D::Physics`)
   - 4D коллайдеры (сферы, боксы, плоскости)
   - 4D физические тела
   - Система частиц для 4D

4. **GUI Система** (`Engine4D::GUI`)
   - 2D графический интерфейс
   - Компоненты: кнопки, панели, поля ввода, слайдеры
   - Система событий и якорей
   - Адаптивное масштабирование
   - Полная поддержка русского языка и UTF-8

5. **Система ввода** (`Engine4D::Input`)
   - Навигация в 4D пространстве
   - Управление поворотами в 6 плоскостях
   - Специальные режимы (сечение, проекции)

6. **Система объектов** (`Engine4D::Core`)
   - Компонентная архитектура
   - 4D трансформации
   - Управление сценами

## 💡 Примеры использования

### Создание 4D объекта

```cpp
#include <Engine4D/Core/GameObject4D.h>

// Создание тессеракта
auto* tesseract = Engine4D::Core::GameObject4D::createPrimitive("Tesseract");
tesseract->transform->setPosition(Engine4D::Math::Vector4(0, 0, 0, 0));
tesseract->transform->setScale(Engine4D::Math::Vector4(2, 2, 2, 2));

// Настройка рендеринга
auto* renderer = tesseract->getComponent<Engine4D::Core::MeshRenderer4D>();
renderer->setColor(Engine4D::Math::Vector4(1, 0.5f, 0, 0.8f));
```

### Создание 4D камеры

```cpp
auto* cameraObj = Engine4D::Core::GameObject4D::create("Camera");
auto* camera = cameraObj->addComponent<Engine4D::Core::Camera4DComponent>();
camera->setMainCamera(true);
cameraObj->transform->setPosition(Engine4D::Math::Vector4(0, 0, -5, 0));
```

### Настройка 4D навигации

```cpp
#include <Engine4D/Input/Input4D.h>

auto& inputManager = Engine4D::Input::InputManager4D::getInstance();
Engine4D::Input::DefaultActions4D::setupDefaultActions(inputManager);

auto controller = std::make_unique<Engine4D::Input::Controller4D>();
controller->setMoveSpeed(10.0f);
controller->setWSpeed(5.0f);
controller->setWMovement(true);
```

### Создание GUI интерфейса

```cpp
#include <Engine4D/GUI/GUI.h>

// Инициализация GUI
GUI::InitializeGUI(1200, 800);

// Создание canvas
auto canvasObject = GameObject4D::create("UI Canvas");
auto canvas = canvasObject->addComponent<UICanvas>();
canvas->setReferenceResolution(1200, 800);

// Создание кнопки
auto button = std::make_shared<UIButton>();
button->setText("Привет!");
button->setRect(UIRect(100, 100, 200, 50));
button->setOnClick([]() {
    std::cout << "Кнопка нажата!" << std::endl;
});
canvas->addElement(button);

// Создание панели
auto panel = std::make_shared<UIPanel>();
panel->setRect(UIRect(50, 50, 300, 400));
panel->setBackgroundColor(Vector4(0.2f, 0.3f, 0.5f, 0.8f));
canvas->addElement(panel);
```

## 🎯 Проекции 4D→3D

Движок поддерживает три основных типа проекций:

1. **Ортогональная проекция**: Отбрасывает W-координату
2. **Перспективная проекция**: Проецирует 4D объекты на 3D с учетом расстояния
3. **Сечение**: Показывает 3D срез 4D объекта при заданном W

## 🔧 Форматы файлов

- **OFF (Object File Format)**: Расширен для 4D политопов
- **.4DO**: Собственный бинарный формат
- **Процедурная генерация**: Тессеракты, симплексы, гиперкубы

## 🚧 Ограничения

- **Производительность**: 4D рендеринг требует больше вычислений
- **Интуитивность**: Навигация в 4D может быть сложной для пользователей
- **Совместимость**: Некоторые 3D инструменты не поддерживают 4D

## 🔮 Планы развития

- [ ] VR поддержка
- [ ] 4D анимации
- [ ] 3D позиционный звук с 4D эффектами
- [ ] Многопользовательские 4D игры
- [ ] Визуальный редактор 4D сцен

## 📄 Лицензия

Проект распространяется под лицензией MIT. См. файл [LICENSE](LICENSE) для подробностей.

## 🤝 Вклад в проект

Мы приветствуем вклад в развитие проекта! Пожалуйста, ознакомьтесь с [CONTRIBUTING.md](CONTRIBUTING.md) для получения подробной информации.

## 📞 Контакты

- **Автор**: [Ваше имя]
- **Email**: [your.email@example.com]
- **GitHub**: [github.com/yourusername/4d-engine]

## 🙏 Благодарности

- [Alan Zucconi](https://www.alanzucconi.com/2023/07/06/understanding-the-fourth-dimension/) за вдохновляющие туториалы по 4D
- [Miegakure](https://miegakure.com/) за демонстрацию 4D игровых механик
- Сообщество разработчиков игр за идеи и поддержку

---

**Примечание**: Это экспериментальный проект, находящийся в активной разработке. API может изменяться между версиями.
