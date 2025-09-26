# Зависимости для 4D Engine

## Требуемые библиотеки

Для компиляции 4D Engine необходимы следующие библиотеки:

### 1. OpenGL
- **Windows**: Обычно входит в состав драйверов видеокарты
- **Linux**: `sudo apt-get install libgl1-mesa-dev` (Ubuntu/Debian)
- **macOS**: Входит в состав Xcode

### 2. GLFW3
- **Windows**: 
  - Скачать с https://www.glfw.org/download.html
  - Или использовать vcpkg: `vcpkg install glfw3`
- **Linux**: `sudo apt-get install libglfw3-dev`
- **macOS**: `brew install glfw`

### 3. GLEW
- **Windows**:
  - Скачать с http://glew.sourceforge.net/
  - Или использовать vcpkg: `vcpkg install glew`
- **Linux**: `sudo apt-get install libglew-dev`
- **macOS**: `brew install glew`

## Установка через vcpkg (рекомендуется)

1. Установите vcpkg:
```bash
git clone https://github.com/Microsoft/vcpkg.git
cd vcpkg
./bootstrap-vcpkg.bat  # Windows
# или
./bootstrap-vcpkg.sh   # Linux/macOS
```

2. Установите зависимости:
```bash
vcpkg install glfw3 glew
```

3. Настройте CMake для использования vcpkg:
```bash
cmake .. -DCMAKE_TOOLCHAIN_FILE=[path to vcpkg]/scripts/buildsystems/vcpkg.cmake
```

## Альтернативная установка

### Windows (Visual Studio)
1. Скачайте GLFW3 и GLEW с официальных сайтов
2. Распакуйте в папку `dependencies/`
3. Обновите CMakeLists.txt для указания путей к библиотекам

### Linux (Ubuntu/Debian)
```bash
sudo apt-get update
sudo apt-get install libgl1-mesa-dev libglfw3-dev libglew-dev
```

### macOS
```bash
brew install glfw glew
```

## Проверка установки

После установки зависимостей попробуйте скомпилировать проект:

```bash
mkdir build
cd build
cmake ..
cmake --build . --config Release
```

## Упрощенная версия

Если у вас нет доступа к OpenGL библиотекам, вы можете скомпилировать упрощенную версию:

```bash
mkdir build_simple
cd build_simple
cmake .. -f ../CMakeLists_simple.txt
cmake --build . --config Release
```

Эта версия не требует OpenGL зависимостей и предназначена для тестирования основных компонентов движка.
