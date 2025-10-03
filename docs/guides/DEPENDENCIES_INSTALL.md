# Инструкции по установке зависимостей

## Linux (Ubuntu/Debian)

### Основные зависимости для сборки

```bash
# Обновить список пакетов
sudo apt-get update

# Установить базовые инструменты сборки
sudo apt-get install -y build-essential cmake git

# Установить библиотеки для OpenGL рендеринга
sudo apt-get install -y libglfw3-dev libglew-dev

# Установить Google Test для тестирования
sudo apt-get install -y libgtest-dev

# Установить Doxygen для генерации документации
sudo apt-get install -y doxygen graphviz

# Установить инструменты для анализа кода
sudo apt-get install -y clang-format clang-tidy cppcheck
```

### Опциональные зависимости для Vulkan

```bash
# Установить Vulkan SDK
sudo apt-get install -y vulkan-tools libvulkan-dev vulkan-validationlayers-dev spirv-tools

# Установить Vulkan Memory Allocator (через заголовочный файл)
wget https://raw.githubusercontent.com/GPUOpen-LibrariesAndSDKs/VulkanMemoryAllocator/master/include/vk_mem_alloc.h \
  -P /usr/local/include/
```

### Опциональные зависимости для CUDA

```bash
# Загрузить и установить CUDA Toolkit с сайта NVIDIA
# https://developer.nvidia.com/cuda-downloads

# Или через репозиторий (пример для Ubuntu 22.04)
wget https://developer.download.nvidia.com/compute/cuda/repos/ubuntu2204/x86_64/cuda-keyring_1.0-1_all.deb
sudo dpkg -i cuda-keyring_1.0-1_all.deb
sudo apt-get update
sudo apt-get install -y cuda
```

### Опциональные зависимости для OptiX

```bash
# Скачать OptiX SDK с сайта NVIDIA
# https://developer.nvidia.com/designworks/optix/download

# Установить (пример)
sudo sh NVIDIA-OptiX-SDK-*.sh --skip-license --prefix=/opt/optix
```

## Проверка установленных зависимостей

```bash
# Проверить CMake
cmake --version

# Проверить компилятор
g++ --version

# Проверить clang-format
clang-format --version

# Проверить установленные библиотеки
pkg-config --list-all | grep -E "glfw|glew"
```

## Сборка проекта после установки зависимостей

### Минимальная сборка (только OpenGL)

```bash
# Конфигурация
cmake --preset default

# Или вручную
cmake -B build \
  -DCMAKE_BUILD_TYPE=Release \
  -DBUILD_VULKAN_RENDERER=OFF \
  -DBUILD_WITH_OPTIX=OFF \
  -DBUILD_WITH_DLSS=OFF \
  -DBUILD_WITH_FSR=OFF

# Сборка
cmake --build build --parallel $(nproc)
```

### Полная сборка (с Vulkan, CUDA, OptiX)

```bash
cmake -B build \
  -DCMAKE_BUILD_TYPE=Release \
  -DBUILD_VULKAN_RENDERER=ON \
  -DBUILD_WITH_CUDA=ON \
  -DBUILD_WITH_OPTIX=ON \
  -DBUILD_WITH_DLSS=ON \
  -DBUILD_WITH_FSR=ON

cmake --build build --parallel $(nproc)
```

## Альтернатива: Docker

Если не хотите устанавливать зависимости локально, используйте Docker:

```bash
# Собрать Docker образ
docker build -t hyperengine:latest .

# Запустить контейнер
docker run -it --rm -v $(pwd):/workspace hyperengine:latest bash

# Внутри контейнера выполнить сборку
cd /workspace
cmake --preset default
cmake --build build
```

## Альтернатива: vcpkg (Windows/Linux)

```bash
# Клонировать vcpkg
git clone https://github.com/Microsoft/vcpkg.git
cd vcpkg

# Linux/macOS
./bootstrap-vcpkg.sh
./vcpkg install glfw3 glew gtest

# Windows
.\bootstrap-vcpkg.bat
.\vcpkg.exe install glfw3:x64-windows glew:x64-windows gtest:x64-windows

# Вернуться в проект и собрать
cd ..
cmake -B build -DCMAKE_TOOLCHAIN_FILE=vcpkg/scripts/buildsystems/vcpkg.cmake
cmake --build build
```

## Устранение проблем

### Ошибка: "Unable to locate vk_mem_alloc.h"

Установите Vulkan Memory Allocator или отключите Vulkan рендеринг:

```bash
cmake -DBUILD_VULKAN_RENDERER=OFF ...
```

### Ошибка: "Could not find a package configuration file provided by glfw3"

Установите GLFW3:

```bash
sudo apt-get install libglfw3-dev
```

### Ошибка: "CUDA compiler not found"

Установите CUDA Toolkit или отключите CUDA:

```bash
cmake -DBUILD_WITH_CUDA=OFF ...
```

## Дополнительные ресурсы

- [CMake Documentation](https://cmake.org/documentation/)
- [Vulkan SDK Downloads](https://vulkan.lunarg.com/sdk/home)
- [NVIDIA CUDA Toolkit](https://developer.nvidia.com/cuda-toolkit)
- [NVIDIA OptiX SDK](https://developer.nvidia.com/optix)
- [vcpkg Documentation](https://vcpkg.io/en/getting-started.html)
