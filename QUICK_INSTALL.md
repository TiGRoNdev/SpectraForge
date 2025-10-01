# 🚀 Быстрая установка HyperEngine

> **Последнее обновление:** 2025-10-01  
> **Поддерживаемые ОС:** Ubuntu 20.04+, Fedora 35+, WSL2

## 📋 Предварительные требования

### Минимальные требования (обязательно)
- **CMake** 3.16+ 
- **C++ компилятор** с поддержкой C++17 (GCC 9+, Clang 10+, MSVC 2019+)
- **Vulkan SDK** 1.3+
- **Git**
- **vcpkg** (менеджер пакетов)

### Опциональные компоненты (для расширенных функций)
- **CUDA Toolkit** 11.8+ (для GPU ускорения с FlashGS)
- **OptiX SDK** 7.5+ (для ray tracing)
- **DLSS SDK** (Streamline) (для NVIDIA AI upscaling)
- **FidelityFX FSR** 2.0+ (для AMD upscaling)

---

## 🎯 Автоматическая установка (Рекомендуется)

### Вариант 1: Полная автоматическая установка

```bash
# Клонирование репозитория
git clone https://github.com/TiGRoNdev/HyperEngine.git
cd HyperEngine

# Запуск автоматической установки всех SDK
chmod +x scripts/install_nvidia_sdks.sh
./scripts/install_nvidia_sdks.sh

# Перезапуск терминала или применение переменных окружения
source ~/.bashrc

# Сборка со всеми функциями
cmake -B build -G Ninja \
  -DCMAKE_TOOLCHAIN_FILE=$HOME/vcpkg/scripts/buildsystems/vcpkg.cmake \
  -DCMAKE_EXPORT_COMPILE_COMMANDS=ON \
  -DBUILD_WITH_CUDA=ON \
  -DBUILD_WITH_OPTIX=ON \
  -DBUILD_WITH_DLSS=ON \
  -DBUILD_WITH_FSR=ON

cmake --build build --config Release
```

### Вариант 2: Установка зависимостей вручную

```bash
# 1. Установка базовых зависимостей
sudo apt-get update
sudo apt-get install -y build-essential cmake git ninja-build \
    libglfw3-dev libglm-dev libvulkan-dev libglew-dev libassimp-dev

# 2. Установка vcpkg
git clone https://github.com/Microsoft/vcpkg.git $HOME/vcpkg
$HOME/vcpkg/bootstrap-vcpkg.sh

# 3. Клонирование HyperEngine
git clone https://github.com/TiGRoNdev/HyperEngine.git
cd HyperEngine

# 4. Базовая сборка (без CUDA/OptiX/DLSS/FSR)
cmake -B build -G Ninja \
  -DCMAKE_TOOLCHAIN_FILE=$HOME/vcpkg/scripts/buildsystems/vcpkg.cmake \
  -DCMAKE_BUILD_TYPE=Release

cmake --build build
```

---

## 🛠️ Ручная установка NVIDIA SDK (для расширенных функций)

### 1. CUDA Toolkit (для GPU ускорения)

**Ubuntu/Debian:**
```bash
# Скачивание и установка CUDA keyring
wget https://developer.download.nvidia.com/compute/cuda/repos/ubuntu2204/x86_64/cuda-keyring_1.1-1_all.deb
sudo dpkg -i cuda-keyring_1.1-1_all.deb

# Установка CUDA Toolkit
sudo apt-get update
sudo apt-get install -y cuda-toolkit-12-8

# Настройка переменных окружения
echo 'export CUDA_PATH=/usr/local/cuda' >> ~/.bashrc
echo 'export PATH=$CUDA_PATH/bin:$PATH' >> ~/.bashrc
echo 'export LD_LIBRARY_PATH=$CUDA_PATH/lib64:$LD_LIBRARY_PATH' >> ~/.bashrc
source ~/.bashrc
```

**Fedora/RHEL:**
```bash
# Установка репозитория CUDA
sudo dnf config-manager --add-repo \
    https://developer.download.nvidia.com/compute/cuda/repos/rhel9/x86_64/cuda-rhel9.repo

# Установка CUDA Toolkit
sudo dnf clean all
sudo dnf -y module install nvidia-driver:latest-dkms
sudo dnf -y install cuda-toolkit-12-8

# Настройка переменных окружения
echo 'export CUDA_PATH=/usr/local/cuda' >> ~/.bashrc
echo 'export PATH=$CUDA_PATH/bin:$PATH' >> ~/.bashrc
echo 'export LD_LIBRARY_PATH=$CUDA_PATH/lib64:$LD_LIBRARY_PATH' >> ~/.bashrc
source ~/.bashrc
```

**Проверка:**
```bash
nvcc --version
```

### 2. OptiX SDK (для ray tracing)

1. Посетите: https://developer.nvidia.com/optix/downloads
2. Войдите в NVIDIA Developer Account (бесплатно)
3. Скачайте OptiX SDK 7.7.0 для Linux
4. Установите:

```bash
chmod +x NVIDIA-OptiX-SDK-7.7.0-linux64-x86_64.sh
./NVIDIA-OptiX-SDK-7.7.0-linux64-x86_64.sh --prefix=$HOME/nvidia-sdks/optix-7.7.0

# Настройка переменных окружения
echo 'export OptiX_ROOT_DIR=$HOME/nvidia-sdks/optix-7.7.0' >> ~/.bashrc
echo 'export OPTIX_ROOT=$OptiX_ROOT_DIR' >> ~/.bashrc
source ~/.bashrc
```

### 3. DLSS SDK (Streamline) - опционально

1. Посетите: https://developer.nvidia.com/rtx/streamline
2. Скачайте Streamline SDK
3. Извлеките в `$HOME/nvidia-sdks/streamline`
4. Настройте переменные:

```bash
echo 'export STREAMLINE_ROOT=$HOME/nvidia-sdks/streamline' >> ~/.bashrc
echo 'export DLSS_ROOT_DIR=$STREAMLINE_ROOT' >> ~/.bashrc
source ~/.bashrc
```

### 4. FidelityFX FSR - опционально

```bash
cd $HOME/nvidia-sdks
git clone https://github.com/GPUOpen-Effects/FidelityFX-FSR2.git

echo 'export FIDELITYFX_ROOT=$HOME/nvidia-sdks/FidelityFX-FSR2' >> ~/.bashrc
echo 'export FSR_ROOT_DIR=$FIDELITYFX_ROOT' >> ~/.bashrc
source ~/.bashrc
```

---

## 🔍 Проверка установки SDK

```bash
# Запуск скрипта проверки
./scripts/setup_sdk.sh

# Или проверка вручную:
nvcc --version                    # CUDA
ls $OptiX_ROOT_DIR/include       # OptiX
ls $STREAMLINE_ROOT/include      # DLSS
ls $FIDELITYFX_ROOT/sdk          # FSR
```

---

## 🏗️ Варианты сборки

### Минимальная конфигурация (только Vulkan)

```bash
cmake -B build -G Ninja \
  -DCMAKE_TOOLCHAIN_FILE=$HOME/vcpkg/scripts/buildsystems/vcpkg.cmake \
  -DCMAKE_BUILD_TYPE=Release \
  -DBUILD_WITH_CUDA=OFF \
  -DBUILD_WITH_OPTIX=OFF \
  -DBUILD_WITH_DLSS=OFF \
  -DBUILD_WITH_FSR=OFF

cmake --build build
```

### Полная конфигурация (все функции)

```bash
cmake -B build -G Ninja \
  -DCMAKE_TOOLCHAIN_FILE=$HOME/vcpkg/scripts/buildsystems/vcpkg.cmake \
  -DCMAKE_EXPORT_COMPILE_COMMANDS=ON \
  -DCMAKE_BUILD_TYPE=Release \
  -DBUILD_WITH_CUDA=ON \
  -DBUILD_WITH_OPTIX=ON \
  -DBUILD_WITH_DLSS=ON \
  -DBUILD_WITH_FSR=ON

cmake --build build --config Release -j$(nproc)
```

### Debug конфигурация (для разработки)

```bash
cmake -B build-debug -G Ninja \
  -DCMAKE_TOOLCHAIN_FILE=$HOME/vcpkg/scripts/buildsystems/vcpkg.cmake \
  -DCMAKE_BUILD_TYPE=Debug \
  -DBUILD_TESTS=ON \
  -DENABLE_COVERAGE=ON

cmake --build build-debug
```

---

## 🐳 Docker (альтернативный вариант)

Если не хотите устанавливать SDK локально, используйте Docker:

```bash
# Сборка Docker образа
docker-compose build

# Запуск сборки в контейнере
docker-compose run --rm ci-runner

# Или вручную
docker build -t hyperengine .
docker run --rm -v $(pwd):/workspace hyperengine
```

---

## ❓ Решение проблем

### CUDA не найдена

```bash
# Проверка установки драйвера NVIDIA
nvidia-smi

# Проверка nvcc
which nvcc

# Если не найден, добавьте в PATH:
export PATH=/usr/local/cuda/bin:$PATH
export LD_LIBRARY_PATH=/usr/local/cuda/lib64:$LD_LIBRARY_PATH
```

### OptiX/DLSS/FSR не найдены

```bash
# Проверьте переменные окружения
echo $OptiX_ROOT_DIR
echo $STREAMLINE_ROOT
echo $FIDELITYFX_ROOT

# Установите вручную, если не установлены
export OptiX_ROOT_DIR=$HOME/nvidia-sdks/optix-7.7.0
export STREAMLINE_ROOT=$HOME/nvidia-sdks/streamline
export FIDELITYFX_ROOT=$HOME/nvidia-sdks/FidelityFX-FSR2
```

### Ошибки линковки Vulkan

```bash
# Установка Vulkan SDK
wget -qO- https://packages.lunarg.com/lunarg-signing-key-pub.asc | sudo tee /etc/apt/trusted.gpg.d/lunarg.asc
sudo wget -qO /etc/apt/sources.list.d/lunarg-vulkan-jammy.list \
    http://packages.lunarg.com/vulkan/lunarg-vulkan-jammy.list

sudo apt update
sudo apt install vulkan-sdk
```

---

## 📚 Дополнительная документация

- [BUILD_INSTRUCTIONS.md](BUILD_INSTRUCTIONS.md) - Полные инструкции по сборке
- [DEPENDENCIES.md](DEPENDENCIES.md) - Список всех зависимостей
- [INSTALL_GUIDE_RU.md](INSTALL_GUIDE_RU.md) - Подробное руководство на русском
- [docs/guides/](docs/guides/) - Дополнительные руководства

---

## 🆘 Поддержка

- **Issues:** https://github.com/TiGRoNdev/HyperEngine/issues
- **Discussions:** https://github.com/TiGRoNdev/HyperEngine/discussions
- **Email:** tigron.dev@gmail.com

---

## 🎉 Быстрый тест

После сборки проверьте примеры:

```bash
# Запуск базового Vulkan demo
./build/VulkanBasic_Demo

# Запуск демо с FlashGS (если CUDA установлена)
./build/flashgs_demo

# Запуск всех тестов
cd build
ctest --output-on-failure
```

---

**Успешной сборки! 🚀**
