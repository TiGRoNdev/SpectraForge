# 4D Game Engine

Экспериментальный игровой движок для создания игр в четырехмерном пространстве. Движок расширяет концепции 3D игровых движков, добавляя поддержку четвертого пространственного измерения (W-координата) для создания уникальных игровых механик и визуальных эффектов.

## 🚀 Особенности

- **4D Математика**: Полная поддержка 4D векторов, матриц и кватернионов
- **4D Рендеринг**: Проекции 4D→3D→2D с поддержкой сечений и перспективы
- **4D Физика**: Коллизии, физические тела и система частиц в 4D пространстве
- **4D Навигация**: Интуитивное управление в гиперпространстве
- **Компонентная архитектура**: Гибкая система объектов и компонентов
- **Кроссплатформенность**: Windows, Linux, macOS

## 📋 Требования

- **C++17** компилятор
- **CMake 3.16+**
- **OpenGL 3.3+**
- **GLFW 3.3+**
- **GLEW 2.1+**

## 🛠️ Быстрый старт

### Установка зависимостей

#### Windows
```bash
# Через vcpkg
git clone https://github.com/Microsoft/vcpkg.git
cd vcpkg
.\bootstrap-vcpkg.bat
.\vcpkg install glfw3:x64-windows glew:x64-windows
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

```bash
git clone <repository-url>
cd 4DEngine
mkdir build && cd build
cmake ..
make -j4
```

### Запуск демо

```bash
./Engine4D_Demo
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

4. **Система ввода** (`Engine4D::Input`)
   - Навигация в 4D пространстве
   - Управление поворотами в 6 плоскостях
   - Специальные режимы (сечение, проекции)

5. **Система объектов** (`Engine4D::Core`)
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
