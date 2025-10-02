# 🐧 Сборка SpectraForge на Linux

> **Дата:** 1 октября 2025  
> **Версия:** 1.0

## 📊 Краткий обзор

SpectraForge поддерживает сборку на Linux с некоторыми ограничениями, связанными с платформенной поддержкой SDK от производителей.

## ⚠️ Важные ограничения платформы

| SDK | Linux Support | Windows Support | Примечания |
|-----|---------------|-----------------|------------|
| **Vulkan** | ✅ Полная | ✅ Полная | Основной рендерер |
| **CUDA** | ✅ Полная | ✅ Полная | Требует NVIDIA GPU |
| **OptiX** | ✅ Полная | ✅ Полная | Требует NVIDIA GPU |
| **DLSS (Streamline)** | ❌ Не поддерживается | ✅ Полная | **Только Windows!** |
| **FSR** | ✅ Полная | ✅ Полная | Кросс-платформенная альтернатива DLSS |

### 🔴 DLSS на Linux

**ВАЖНО:** NVIDIA Streamline SDK (DLSS) **официально поддерживает только Windows**. 

Если вам нужен AI-апскейлинг на Linux, используйте **FSR (FidelityFX Super Resolution)** от AMD, который полностью кросс-платформенный и работает на любом GPU.

## 🚀 Быстрый старт

### Вариант 1: Автоматическая сборка (рекомендуется)

```bash
cd /home/tigron/Documents/GITHUB/SpectraForge

# Запустить скрипт автоматической сборки
./build_linux.sh Release

# Или для Debug сборки
./build_linux.sh Debug
```

Скрипт автоматически:
- ✅ Проверит наличие vcpkg
- ✅ Определит доступные SDK (CUDA, OptiX, FSR)
- ✅ Отключит DLSS (не поддерживается на Linux)
- ✅ Настроит оптимальную конфигурацию
- ✅ Выполнит сборку

### Вариант 2: Ручная сборка

#### Рекомендуемая конфигурация для Linux

```bash
# С CUDA и OptiX (NVIDIA GPU)
cmake -B build -G Ninja \
  -DCMAKE_TOOLCHAIN_FILE=$HOME/vcpkg/scripts/buildsystems/vcpkg.cmake \
  -DCMAKE_EXPORT_COMPILE_COMMANDS=ON \
  -DCMAKE_BUILD_TYPE=Release \
  -DBUILD_WITH_CUDA=ON \
  -DBUILD_WITH_OPTIX=ON \
  -DBUILD_WITH_DLSS=OFF \
  -DBUILD_WITH_FSR=ON

cmake --build build --config Release -j$(nproc)
```

#### Базовая конфигурация (без NVIDIA SDK)

```bash
# Только Vulkan + FSR
cmake -B build -G Ninja \
  -DCMAKE_TOOLCHAIN_FILE=$HOME/vcpkg/scripts/buildsystems/vcpkg.cmake \
  -DCMAKE_EXPORT_COMPILE_COMMANDS=ON \
  -DCMAKE_BUILD_TYPE=Release \
  -DBUILD_WITH_CUDA=OFF \
  -DBUILD_WITH_OPTIX=OFF \
  -DBUILD_WITH_DLSS=OFF \
  -DBUILD_WITH_FSR=ON

cmake --build build --config Release -j$(nproc)
```

## 📦 Установка зависимостей

### Системные пакеты (Ubuntu/Debian)

```bash
sudo apt-get update
sudo apt-get install -y \
    build-essential \
    cmake \
    ninja-build \
    git \
    libvulkan-dev \
    vulkan-tools \
    libglfw3-dev \
    libglm-dev
```

### vcpkg

```bash
# Установка vcpkg
cd ~
git clone https://github.com/Microsoft/vcpkg.git
cd vcpkg
./bootstrap-vcpkg.sh

# Установка зависимостей проекта
./vcpkg install glfw3 glew glm --triplet x64-linux
```

### CUDA Toolkit (опционально, для NVIDIA GPU)

```bash
# Ubuntu 22.04
wget https://developer.download.nvidia.com/compute/cuda/repos/ubuntu2204/x86_64/cuda-keyring_1.1-1_all.deb
sudo dpkg -i cuda-keyring_1.1-1_all.deb
sudo apt-get update
sudo apt-get install -y cuda-toolkit-12-8

# Настройка переменных окружения
echo 'export CUDA_PATH=/usr/local/cuda' >> ~/.bashrc
echo 'export PATH=$CUDA_PATH/bin:$PATH' >> ~/.bashrc
echo 'export LD_LIBRARY_PATH=$CUDA_PATH/lib64:$LD_LIBRARY_PATH' >> ~/.bashrc
source ~/.bashrc

# Проверка
nvcc --version
```

### OptiX SDK (опционально, для NVIDIA GPU)

1. Посетите: https://developer.nvidia.com/optix/downloads
2. Войдите в NVIDIA Developer Account
3. Скачайте OptiX SDK 7.7.0 для Linux
4. Установите:

```bash
chmod +x NVIDIA-OptiX-SDK-7.7.0-linux64-x86_64.sh
mkdir -p $HOME/nvidia-sdks
./NVIDIA-OptiX-SDK-7.7.0-linux64-x86_64.sh --prefix=$HOME/nvidia-sdks/optix-7.7.0

# Настройка переменных
echo 'export OptiX_ROOT_DIR=$HOME/nvidia-sdks/optix-7.7.0' >> ~/.bashrc
echo 'export OPTIX_ROOT=$OptiX_ROOT_DIR' >> ~/.bashrc
source ~/.bashrc
```

### FSR (FidelityFX Super Resolution)

```bash
mkdir -p $HOME/nvidia-sdks
cd $HOME/nvidia-sdks
git clone https://github.com/GPUOpen-Effects/FidelityFX-FSR2.git

# Настройка переменных
echo 'export FIDELITYFX_ROOT=$HOME/nvidia-sdks/FidelityFX-FSR2' >> ~/.bashrc
echo 'export FSR_ROOT_DIR=$FIDELITYFX_ROOT' >> ~/.bashrc
source ~/.bashrc
```

## 🔍 Проверка установки

```bash
# Проверка переменных окружения
echo "CUDA_PATH: $CUDA_PATH"
echo "OptiX_ROOT_DIR: $OptiX_ROOT_DIR"
echo "FSR_ROOT_DIR: $FSR_ROOT_DIR"

# Проверка CUDA
nvcc --version 2>/dev/null || echo "CUDA не установлен"

# Проверка OptiX
ls -l $OptiX_ROOT_DIR/include/optix.h 2>/dev/null || echo "OptiX не установлен"

# Проверка FSR
ls -l $FSR_ROOT_DIR/sdk/include/FidelityFX/host/ffx_fsr2.h 2>/dev/null || echo "FSR не установлен"

# Проверка Vulkan
vulkaninfo | head -20
```

## 🛠️ Решение проблем

### Проблема: DLSS не найден

**Решение:** Это нормально для Linux. DLSS работает только на Windows.

Используйте FSR вместо DLSS:
```bash
cmake ... -DBUILD_WITH_DLSS=OFF -DBUILD_WITH_FSR=ON
```

### Проблема: CUDA не найден

**Проверка:**
```bash
which nvcc
nvcc --version
echo $CUDA_PATH
```

**Решение:** Установите CUDA Toolkit и настройте переменные окружения (см. выше).

### Проблема: OptiX не найден

**Проверка:**
```bash
ls -l $OptiX_ROOT_DIR/include/optix.h
echo $OptiX_ROOT_DIR
```

**Решение:** Скачайте и установите OptiX SDK с сайта NVIDIA.

### Проблема: Vulkan не найден

**Решение:**
```bash
sudo apt-get install -y libvulkan-dev vulkan-tools
vulkaninfo
```

## 📊 Сравнение DLSS vs FSR

| Параметр | DLSS | FSR |
|----------|------|-----|
| Платформы | Windows only | Windows, Linux, macOS |
| GPU поддержка | NVIDIA RTX only | Все GPU (NVIDIA, AMD, Intel) |
| Качество | Отлично (AI) | Хорошо (не AI) |
| Производительность | До 8x FPS | До 2x FPS |
| Интеграция | Сложная | Простая |

**Рекомендация для Linux:** Используйте FSR как универсальное решение для AI-апскейлинга.

## 🎯 Примеры конфигураций

### NVIDIA GPU с CUDA + OptiX + FSR

```bash
./build_linux.sh Release
# Или
cmake -B build -G Ninja \
  -DCMAKE_TOOLCHAIN_FILE=$HOME/vcpkg/scripts/buildsystems/vcpkg.cmake \
  -DBUILD_WITH_CUDA=ON \
  -DBUILD_WITH_OPTIX=ON \
  -DBUILD_WITH_FSR=ON \
  -DBUILD_WITH_DLSS=OFF
```

### AMD/Intel GPU с FSR только

```bash
cmake -B build -G Ninja \
  -DCMAKE_TOOLCHAIN_FILE=$HOME/vcpkg/scripts/buildsystems/vcpkg.cmake \
  -DBUILD_WITH_CUDA=OFF \
  -DBUILD_WITH_OPTIX=OFF \
  -DBUILD_WITH_FSR=ON \
  -DBUILD_WITH_DLSS=OFF
```

### Минимальная сборка (только Vulkan)

```bash
cmake -B build -G Ninja \
  -DCMAKE_TOOLCHAIN_FILE=$HOME/vcpkg/scripts/buildsystems/vcpkg.cmake \
  -DBUILD_WITH_CUDA=OFF \
  -DBUILD_WITH_OPTIX=OFF \
  -DBUILD_WITH_FSR=OFF \
  -DBUILD_WITH_DLSS=OFF
```

## 📚 Дополнительные ресурсы

- [BUILD_INSTRUCTIONS.md](BUILD_INSTRUCTIONS.md) - Общие инструкции по сборке
- [DEPENDENCIES.md](DEPENDENCIES.md) - Полный список зависимостей
- [SDK_INSTALLATION_ISSUES.md](SDK_INSTALLATION_ISSUES.md) - Решение проблем с SDK

---

**Удачной сборки на Linux! 🐧🚀**

