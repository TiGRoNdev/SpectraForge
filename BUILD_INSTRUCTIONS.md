# Инструкции по сборке - 4D Game Engine

## Требования

### Системные требования
- **Операционная система**: Windows 10/11, Linux (Ubuntu 20.04+), macOS 10.15+
- **Процессор**: x64 архитектура
- **Память**: Минимум 4 GB RAM (рекомендуется 8 GB+)
- **Видеокарта**: Поддержка OpenGL 3.3+ (рекомендуется OpenGL 4.0+)

### Зависимости
- **CMake**: версия 3.16 или выше
- **C++ компилятор**: поддерживающий C++17
  - Windows: Visual Studio 2019/2022 или MinGW-w64
  - Linux: GCC 8+ или Clang 8+
  - macOS: Xcode 12+ или Clang 8+
- **OpenGL**: версия 3.3 или выше
- **GLFW**: версия 3.3 или выше
- **GLEW**: версия 2.1 или выше

## Установка зависимостей

### Windows

#### Вариант 1: Visual Studio + vcpkg
```bash
# Установка vcpkg
git clone https://github.com/Microsoft/vcpkg.git
cd vcpkg
.\bootstrap-vcpkg.bat

# Установка зависимостей
.\vcpkg install glfw3:x64-windows
.\vcpkg install glew:x64-windows
```

#### Вариант 2: MinGW-w64 + MSYS2
```bash
# Установка через pacman
pacman -S mingw-w64-x86_64-cmake
pacman -S mingw-w64-x86_64-glfw
pacman -S mingw-w64-x86_64-glew
```

### Linux (Ubuntu/Debian)

```bash
# Обновление пакетов
sudo apt update

# Установка зависимостей
sudo apt install build-essential cmake
sudo apt install libgl1-mesa-dev
sudo apt install libglfw3-dev
sudo apt install libglew-dev
```

### Linux (Fedora/CentOS)

```bash
# Установка зависимостей
sudo dnf install gcc-c++ cmake
sudo dnf install mesa-libGL-devel
sudo dnf install glfw-devel
sudo dnf install glew-devel
```

### macOS

```bash
# Установка через Homebrew
brew install cmake
brew install glfw
brew install glew
```

## Сборка проекта

### 1. Клонирование репозитория

```bash
git clone <repository-url>
cd 4DEngine
```

### 2. Создание директории сборки

```bash
mkdir build
cd build
```

### 3. Конфигурация CMake

#### Windows (Visual Studio)
```bash
cmake .. -DCMAKE_TOOLCHAIN_FILE=<path-to-vcpkg>/scripts/buildsystems/vcpkg.cmake
```

#### Windows (MinGW)
```bash
cmake .. -G "MinGW Makefiles"
```

#### Linux/macOS
```bash
cmake ..
```

### 4. Сборка

#### Windows (Visual Studio)
```bash
cmake --build . --config Release
```

#### Linux/macOS/MinGW
```bash
make -j4
```

### 5. Запуск демо

#### Windows
```bash
# Debug
.\Debug\Engine4D_Demo.exe

# Release
.\Release\Engine4D_Demo.exe
```

#### Linux/macOS
```bash
./Engine4D_Demo
```

## Структура проекта

```
4DEngine/
├── CMakeLists.txt              # Основной файл CMake
├── BUILD_INSTRUCTIONS.md       # Этот файл
├── README.md                   # Основная документация
├── include/                    # Заголовочные файлы
│   └── Engine4D/
│       ├── Math/               # Математическая библиотека
│       ├── Rendering/          # Система рендеринга
│       ├── Physics/            # Физическая система
│       ├── Input/              # Система ввода
│       └── Core/               # Основные компоненты
├── src/                        # Исходный код
│   ├── Math/
│   ├── Rendering/
│   ├── Physics/
│   ├── Input/
│   └── Core/
├── examples/                   # Примеры использования
│   └── demo/
│       └── main.cpp
├── shaders/                    # Шейдеры
│   ├── vertex_4d.glsl
│   └── fragment_4d.glsl
├── docs/                       # Документация
│   ├── README.md
│   ├── API_Reference.md
│   └── Examples.md
└── build/                      # Директория сборки (создается)
```

## Настройка IDE

### Visual Studio Code

1. Установите расширения:
   - C/C++ (Microsoft)
   - CMake Tools
   - GLSL Language Support

2. Откройте папку проекта в VS Code

3. Нажмите Ctrl+Shift+P и выберите "CMake: Configure"

4. Выберите компилятор и нажмите "CMake: Build"

### Visual Studio

1. Откройте папку проекта в Visual Studio

2. Visual Studio автоматически обнаружит CMakeLists.txt

3. Выберите конфигурацию (Debug/Release) и соберите проект

### CLion

1. Откройте папку проекта в CLion

2. CLion автоматически обнаружит CMakeLists.txt

3. Настройте CMake в Settings → Build, Execution, Deployment → CMake

4. Соберите проект через Build → Build Project

## Отладка

### Настройка отладчика

#### Visual Studio Code
Создайте файл `.vscode/launch.json`:

```json
{
    "version": "0.2.0",
    "configurations": [
        {
            "name": "Debug Engine4D Demo",
            "type": "cppdbg",
            "request": "launch",
            "program": "${workspaceFolder}/build/Engine4D_Demo",
            "args": [],
            "stopAtEntry": false,
            "cwd": "${workspaceFolder}",
            "environment": [],
            "externalConsole": false,
            "MIMode": "gdb",
            "setupCommands": [
                {
                    "description": "Enable pretty-printing for gdb",
                    "text": "-enable-pretty-printing",
                    "ignoreFailures": true
                }
            ],
            "preLaunchTask": "build",
            "miDebuggerPath": "/usr/bin/gdb"
        }
    ]
}
```

#### Visual Studio
1. Установите точку останова в коде
2. Нажмите F5 для запуска с отладкой
3. Используйте окна "Watch", "Locals", "Call Stack" для отладки

## Тестирование

### Запуск тестов

```bash
cd build
ctest --verbose
```

### Создание тестов

Создайте файл `tests/test_math.cpp`:

```cpp
#include <gtest/gtest.h>
#include <Engine4D/Math/Vector4.h>

TEST(Vector4Test, Addition) {
    Engine4D::Math::Vector4 a(1, 2, 3, 4);
    Engine4D::Math::Vector4 b(5, 6, 7, 8);
    Engine4D::Math::Vector4 result = a + b;
    
    EXPECT_FLOAT_EQ(result.x, 6.0f);
    EXPECT_FLOAT_EQ(result.y, 8.0f);
    EXPECT_FLOAT_EQ(result.z, 10.0f);
    EXPECT_FLOAT_EQ(result.w, 12.0f);
}
```

## Устранение неполадок

### Ошибки компиляции

#### "OpenGL not found"
```bash
# Linux
sudo apt install libgl1-mesa-dev

# macOS
brew install glew
```

#### "GLFW not found"
```bash
# Linux
sudo apt install libglfw3-dev

# macOS
brew install glfw
```

#### "GLEW not found"
```bash
# Linux
sudo apt install libglew-dev

# macOS
brew install glew
```

### Ошибки линковки

#### "Undefined reference to glfwInit"
Убедитесь, что GLFW правильно установлен и CMake его находит:

```bash
cmake .. -DCMAKE_PREFIX_PATH=/path/to/glfw
```

#### "OpenGL functions not found"
Убедитесь, что GLEW правильно инициализирован:

```cpp
#include <GL/glew.h>
// ...
if (glewInit() != GLEW_OK) {
    // Ошибка инициализации
}
```

### Ошибки выполнения

#### "Failed to create window"
Проверьте поддержку OpenGL:

```bash
# Linux
glxinfo | grep "OpenGL version"

# Windows
# Используйте GPU-Z или аналогичную утилиту
```

#### "Shader compilation failed"
Проверьте файлы шейдеров в папке `shaders/`:

```bash
# Убедитесь, что файлы существуют
ls shaders/
# vertex_4d.glsl
# fragment_4d.glsl
```

## Производительность

### Оптимизация сборки

#### Release режим
```bash
cmake .. -DCMAKE_BUILD_TYPE=Release
make -j4
```

#### Включение оптимизаций
```bash
cmake .. -DCMAKE_CXX_FLAGS="-O3 -march=native"
```

#### Отключение отладочной информации
```bash
cmake .. -DCMAKE_BUILD_TYPE=Release -DCMAKE_CXX_FLAGS="-DNDEBUG"
```

### Профилирование

#### Linux (perf)
```bash
perf record ./Engine4D_Demo
perf report
```

#### Windows (Visual Studio)
1. Build → Performance Profiler
2. Выберите "CPU Usage"
3. Запустите профилирование

## Дополнительные настройки

### Настройка CMake

Создайте файл `CMakeUserPresets.json`:

```json
{
    "version": 3,
    "configurePresets": [
        {
            "name": "default",
            "displayName": "Default Config",
            "generator": "Unix Makefiles",
            "binaryDir": "${sourceDir}/build",
            "cacheVariables": {
                "CMAKE_BUILD_TYPE": "Release",
                "CMAKE_CXX_STANDARD": "17"
            }
        }
    ]
}
```

### Настройка компилятора

#### GCC
```bash
export CXX=g++
export CC=gcc
```

#### Clang
```bash
export CXX=clang++
export CC=clang
```

## Поддержка

Если у вас возникли проблемы:

1. Проверьте раздел "Устранение неполадок"
2. Убедитесь, что все зависимости установлены
3. Проверьте версии компилятора и CMake
4. Создайте issue в репозитории с подробным описанием проблемы

## Лицензия

Проект распространяется под лицензией MIT. См. файл LICENSE для подробностей.
