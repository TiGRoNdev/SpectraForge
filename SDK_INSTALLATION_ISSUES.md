# 🔧 Решение проблем установки SDK для HyperEngine

> **Дата:** 1 октября 2025  
> **Версия:** 1.0

## 📊 Краткое резюме проблем

При попытке сборки HyperEngine со всеми включенными параметрами (`BUILD_WITH_CUDA=ON`, `BUILD_WITH_OPTIX=ON`, `BUILD_WITH_DLSS=ON`, `BUILD_WITH_FSR=ON`) возникают следующие ошибки:

```
CMake Warning: CUDA compiler not found. CUDA support will be disabled.
Could NOT find OptiX (missing: OptiX_ROOT_DIR OptiX_INCLUDE_DIR)
Could NOT find DLSS (missing: DLSS_ROOT_DIR DLSS_INCLUDE_DIR DLSS_LIBRARY)
Could NOT find FSR (missing: FSR_ROOT_DIR FSR_INCLUDE_DIR FSR_API_LIBRARY)
```

**Результат:** Все расширенные функции отключены, сборка идет только с базовым Vulkan.

---

## 🎯 Причины проблем

| SDK | Проблема | Причина |
|-----|----------|---------|
| **CUDA** | Компилятор nvcc не найден | CUDA Toolkit не установлен или не в PATH |
| **OptiX** | Переменная OptiX_ROOT_DIR не установлена | OptiX SDK не установлен (требует ручной загрузки) |
| **DLSS** | Переменные DLSS_ROOT_DIR не установлены | Streamline SDK не установлен (требует ручной загрузки) |
| **FSR** | Переменные FSR_ROOT_DIR не установлены | FidelityFX FSR не клонирован |

---

## ✅ Решения

### 🚀 Вариант 1: Автоматическая установка (РЕКОМЕНДУЕТСЯ)

```bash
# Запустите скрипт автоматической установки
cd /home/tigron/Documents/GITHUB/HyperEngine
chmod +x scripts/install_nvidia_sdks.sh
./scripts/install_nvidia_sdks.sh

# Выберите в меню:
# 5) Установить ВСЁ

# Следуйте инструкциям скрипта для:
# - CUDA Toolkit (автоматическая установка)
# - OptiX SDK (требуется ручная загрузка с сайта NVIDIA)
# - DLSS SDK (требуется ручная загрузка с сайта NVIDIA)
# - FSR SDK (автоматическое клонирование)

# Перезапустите терминал или примените переменные
source ~/.bashrc

# Пересоберите проект
cmake -B build -G Ninja \
  -DCMAKE_TOOLCHAIN_FILE=$HOME/vcpkg/scripts/buildsystems/vcpkg.cmake \
  -DCMAKE_EXPORT_COMPILE_COMMANDS=ON \
  -DBUILD_WITH_CUDA=ON \
  -DBUILD_WITH_OPTIX=ON \
  -DBUILD_WITH_DLSS=ON \
  -DBUILD_WITH_FSR=ON

cmake --build build --config Release -j$(nproc)
```

### 🔧 Вариант 2: Ручная установка

#### 2.1. Установка CUDA Toolkit

```bash
# Ubuntu 22.04
wget https://developer.download.nvidia.com/compute/cuda/repos/ubuntu2204/x86_64/cuda-keyring_1.1-1_all.deb
sudo dpkg -i cuda-keyring_1.1-1_all.deb
sudo apt-get update
sudo apt-get install -y cuda-toolkit-12-8

# Настройка переменных окружения
cat >> ~/.bashrc << 'EOF'
export CUDA_PATH=/usr/local/cuda
export PATH=$CUDA_PATH/bin:$PATH
export LD_LIBRARY_PATH=$CUDA_PATH/lib64:$LD_LIBRARY_PATH
EOF

source ~/.bashrc
nvcc --version
```

#### 2.2. Установка OptiX SDK

**Шаги:**

1. Посетите: https://developer.nvidia.com/optix/downloads
2. Войдите в NVIDIA Developer Account (бесплатная регистрация)
3. Скачайте OptiX SDK 7.7.0 для Linux
4. Установите:

```bash
chmod +x NVIDIA-OptiX-SDK-7.7.0-linux64-x86_64.sh
mkdir -p $HOME/nvidia-sdks
./NVIDIA-OptiX-SDK-7.7.0-linux64-x86_64.sh --prefix=$HOME/nvidia-sdks/optix-7.7.0

cat >> ~/.bashrc << 'EOF'
export OptiX_ROOT_DIR=$HOME/nvidia-sdks/optix-7.7.0
export OPTIX_ROOT=$OptiX_ROOT_DIR
EOF

source ~/.bashrc
```

#### 2.3. Установка DLSS SDK (Streamline)

**Шаги:**

1. Посетите: https://developer.nvidia.com/rtx/streamline
2. Скачайте Streamline SDK
3. Извлеките в `$HOME/nvidia-sdks/streamline`
4. Настройте:

```bash
cat >> ~/.bashrc << 'EOF'
export STREAMLINE_ROOT=$HOME/nvidia-sdks/streamline
export DLSS_ROOT_DIR=$STREAMLINE_ROOT
EOF

source ~/.bashrc
```

#### 2.4. Установка FidelityFX FSR

```bash
cd $HOME/nvidia-sdks
git clone https://github.com/GPUOpen-Effects/FidelityFX-FSR2.git

cat >> ~/.bashrc << 'EOF'
export FIDELITYFX_ROOT=$HOME/nvidia-sdks/FidelityFX-FSR2
export FSR_ROOT_DIR=$FIDELITYFX_ROOT
EOF

source ~/.bashrc
```

---

## 🔍 Проверка установки

### Шаг 1: Проверка переменных окружения

```bash
echo "CUDA_PATH: $CUDA_PATH"
echo "OptiX_ROOT_DIR: $OptiX_ROOT_DIR"
echo "STREAMLINE_ROOT: $STREAMLINE_ROOT"
echo "FIDELITYFX_ROOT: $FIDELITYFX_ROOT"
```

### Шаг 2: Проверка файлов

```bash
# CUDA
nvcc --version
which nvcc

# OptiX
ls $OptiX_ROOT_DIR/include/optix.h

# DLSS (опционально)
ls $STREAMLINE_ROOT/include/sl.h

# FSR
ls $FIDELITYFX_ROOT/sdk/include/FidelityFX/host/ffx_fsr2.h
```

### Шаг 3: Запуск скрипта проверки

```bash
cd /home/tigron/Documents/GITHUB/HyperEngine
./scripts/setup_sdk.sh
```

**Ожидаемый вывод:**
```
[1/4] Проверка CUDA Toolkit...
✓ CUDA Toolkit найден
Cuda compilation tools, release 12.8, V12.8.0

[2/4] Проверка OptiX SDK...
✓ OptiX SDK найден: /home/tigron/nvidia-sdks/optix-7.7.0

[3/4] Проверка DLSS SDK (опционально)...
✓ DLSS SDK найден: /home/tigron/nvidia-sdks/streamline

[4/4] Проверка FidelityFX SDK...
✓ FidelityFX SDK найден: /home/tigron/nvidia-sdks/FidelityFX-FSR2
```

---

## 🏗️ Пересборка проекта

После установки всех SDK пересоберите проект:

```bash
cd /home/tigron/Documents/GITHUB/HyperEngine

# Очистка старой конфигурации (опционально)
rm -rf build

# Конфигурация со всеми SDK
cmake -B build -G Ninja \
  -DCMAKE_TOOLCHAIN_FILE=$HOME/vcpkg/scripts/buildsystems/vcpkg.cmake \
  -DCMAKE_EXPORT_COMPILE_COMMANDS=ON \
  -DCMAKE_BUILD_TYPE=Release \
  -DBUILD_WITH_CUDA=ON \
  -DBUILD_WITH_OPTIX=ON \
  -DBUILD_WITH_DLSS=ON \
  -DBUILD_WITH_FSR=ON

# Проверка конфигурации CMake
# Ожидаемый вывод:
# -- Build with CUDA: ON
# -- Build with OptiX: ON
# -- Build with DLSS: ON
# -- Build with FSR: ON

# Сборка
cmake --build build --config Release -j$(nproc)

# Проверка сборки
ls -lh build/*.a build/*_Demo
```

---

## 📝 Ожидаемая конфигурация CMake

После правильной установки всех SDK, вывод CMake должен выглядеть так:

```
-- Running vcpkg install - done
-- OpenGL backend enabled
-- Vulkan set as default backend
-- GLM found via CMake config
-- CUDA Toolkit found: /usr/local/cuda (version 12.8.0)
-- CUDA Advanced configuration:
--   Version: 12.8.0
--   Compute Capabilities: 75;80;86;89;90
--   Vulkan Interop: TRUE
--   Memory Pools: TRUE
--   Stream Ordered Alloc: TRUE
-- Found OptiX: /home/tigron/nvidia-sdks/optix-7.7.0 (version 7.7.0)
-- Found DLSS: /home/tigron/nvidia-sdks/streamline (version 2.5.1)
-- Found FSR: /home/tigron/nvidia-sdks/FidelityFX-FSR2 (version 2.2.1)
-- Building HyperEngine with modular architecture...
-- Building Vulkan Hybrid Renderer...
-- VulkanRenderer: CUDA поддержка включена
-- CUDA demo включены
-- 
-- === HyperEngine Build Configuration ===
-- Build 3D Engine: ON
-- Build 4D Engine: OFF
-- Build Vulkan Renderer: ON
-- Build with CUDA: ON
-- Build with OptiX: ON
-- Build with DLSS: ON
-- Build with FSR: ON
-- Build Examples: ON
-- Build Tests: ON
-- C++ Standard: 17
-- Compiler: GNU
-- ========================================
```

---

## 🎯 Альтернативные варианты

### Вариант A: Сборка без расширенных SDK

Если не нужны CUDA/OptiX/DLSS/FSR, соберите базовую версию:

```bash
cmake -B build -G Ninja \
  -DCMAKE_TOOLCHAIN_FILE=$HOME/vcpkg/scripts/buildsystems/vcpkg.cmake \
  -DCMAKE_BUILD_TYPE=Release \
  -DBUILD_WITH_CUDA=OFF \
  -DBUILD_WITH_OPTIX=OFF \
  -DBUILD_WITH_DLSS=OFF \
  -DBUILD_WITH_FSR=OFF

cmake --build build --config Release
```

### Вариант B: Частичная сборка (только CUDA)

```bash
cmake -B build -G Ninja \
  -DCMAKE_TOOLCHAIN_FILE=$HOME/vcpkg/scripts/buildsystems/vcpkg.cmake \
  -DCMAKE_BUILD_TYPE=Release \
  -DBUILD_WITH_CUDA=ON \
  -DBUILD_WITH_OPTIX=OFF \
  -DBUILD_WITH_DLSS=OFF \
  -DBUILD_WITH_FSR=OFF

cmake --build build --config Release
```

### Вариант C: Docker (если локальная установка проблематична)

```bash
cd /home/tigron/Documents/GITHUB/HyperEngine

# Сборка Docker образа (все SDK включены)
docker-compose build

# Запуск сборки в контейнере
docker-compose run --rm ci-runner

# Результаты сборки будут в ./build/
```

---

## 📚 Дополнительные ресурсы

- [QUICK_INSTALL.md](QUICK_INSTALL.md) - Быстрая установка
- [INSTALL_GUIDE_RU.md](INSTALL_GUIDE_RU.md) - Подробное руководство на русском
- [BUILD_INSTRUCTIONS.md](BUILD_INSTRUCTIONS.md) - Инструкции по сборке
- [DEPENDENCIES.md](DEPENDENCIES.md) - Список зависимостей

### Официальная документация SDK

- **CUDA:** https://docs.nvidia.com/cuda/
- **OptiX:** https://raytracing-docs.nvidia.com/optix7/
- **Streamline (DLSS):** https://developer.nvidia.com/rtx/streamline
- **FidelityFX FSR:** https://gpuopen.com/fidelityfx-superresolution/

---

## 🆘 Поддержка

Если после выполнения всех шагов проблемы сохраняются:

1. Проверьте журналы сборки: `cmake -B build ... 2>&1 | tee cmake_log.txt`
2. Создайте issue на GitHub: https://github.com/TiGRoNdev/HyperEngine/issues
3. Укажите в issue:
   - Версию ОС
   - Вывод `nvcc --version` и `nvidia-smi`
   - Полный лог CMake
   - Установленные переменные окружения (`env | grep -E "CUDA|OPTIX|DLSS|FSR"`)

---

**Удачной сборки! 🚀**

