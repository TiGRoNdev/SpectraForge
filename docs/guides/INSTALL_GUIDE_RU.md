# 🇷🇺 Полное руководство по установке SpectraForge

> **Дата обновления:** 1 октября 2025 г.  
> **Версия:** 2.0  
> **Автор:** TiGRoN Development Team

## 📖 Содержание

1. [Введение](#введение)
2. [Системные требования](#системные-требования)
3. [Быстрая установка](#быстрая-установка)
4. [Подробная установка](#подробная-установка)
5. [Установка NVIDIA SDK](#установка-nvidia-sdk)
6. [Сборка проекта](#сборка-проекта)
7. [Решение проблем](#решение-проблем)
8. [FAQ](#faq)

---

## 🎯 Введение

SpectraForge - это современный гибридный 3D/4D движок рендеринга на основе Vulkan с поддержкой:
- **Vulkan** - базовый рендеринг
- **CUDA** - GPU ускорение для Gaussian Splatting (FlashGS)
- **OptiX** - ray tracing для вторичных эффектов
- **DLSS** - AI upscaling от NVIDIA
- **FSR** - upscaling от AMD

---

## 💻 Системные требования

### Минимальные (только Vulkan)

| Компонент | Требование |
|-----------|------------|
| **ОС** | Ubuntu 20.04+, Fedora 35+, Windows 10/11, WSL2 |
| **CPU** | x86_64 с поддержкой SSE4.2 |
| **RAM** | 8 GB |
| **GPU** | Vulkan 1.3 совместимая |
| **Компилятор** | GCC 9+, Clang 10+, MSVC 2019+ |
| **CMake** | 3.16+ |

### Рекомендуемые (с CUDA/OptiX)

| Компонент | Требование |
|-----------|------------|
| **GPU** | NVIDIA RTX 3060+ (Ampere или новее) |
| **VRAM** | 8 GB+ |
| **CUDA** | 11.8+ |
| **OptiX** | 7.5+ |
| **Драйвер NVIDIA** | 525.60+ |

---

## ⚡ Быстрая установка

### Автоматическая установка (Рекомендуется)

```bash
# 1. Клонирование репозитория
git clone https://github.com/TiGRoNdev/SpectraForge.git
cd SpectraForge

# 2. Запуск автоматического установщика
chmod +x scripts/install_nvidia_sdks.sh
./scripts/install_nvidia_sdks.sh

# Выберите опцию "5) Установить ВСЁ" в меню

# 3. Применение переменных окружения
source ~/.bashrc

# 4. Сборка со всеми функциями
cmake -B build -G Ninja \
  -DCMAKE_TOOLCHAIN_FILE=$HOME/vcpkg/scripts/buildsystems/vcpkg.cmake \
  -DCMAKE_EXPORT_COMPILE_COMMANDS=ON \
  -DBUILD_WITH_CUDA=ON \
  -DBUILD_WITH_OPTIX=ON \
  -DBUILD_WITH_DLSS=ON \
  -DBUILD_WITH_FSR=ON

cmake --build build --config Release -j$(nproc)

# 5. Запуск тестов
cd build
ctest --output-on-failure
```

Готово! 🎉 Переходите к [Проверке установки](#проверка-установки).

---

## 📦 Подробная установка

### Шаг 1: Установка базовых зависимостей

#### Ubuntu/Debian

```bash
# Обновление системы
sudo apt-get update
sudo apt-get upgrade -y

# Установка инструментов сборки
sudo apt-get install -y \
    build-essential \
    cmake \
    git \
    ninja-build \
    pkg-config \
    wget \
    curl

# Установка библиотек для Vulkan
sudo apt-get install -y \
    libglfw3-dev \
    libglm-dev \
    libvulkan-dev \
    libglew-dev \
    libassimp-dev \
    libstb-dev

# Установка Google Test
sudo apt-get install -y \
    libgtest-dev \
    libgmock-dev
```

#### Fedora/RHEL

```bash
# Обновление системы
sudo dnf update -y

# Установка инструментов сборки
sudo dnf install -y \
    gcc-c++ \
    cmake \
    git \
    ninja-build \
    pkg-config \
    wget \
    curl

# Установка библиотек
sudo dnf install -y \
    glfw-devel \
    glm-devel \
    vulkan-devel \
    glew-devel \
    assimp-devel

# Установка Google Test
sudo dnf install -y \
    gtest-devel \
    gmock-devel
```

### Шаг 2: Установка Vulkan SDK

#### Опция A: Через пакетный менеджер (Ubuntu)

```bash
# Добавление репозитория LunarG
wget -qO- https://packages.lunarg.com/lunarg-signing-key-pub.asc | \
    sudo tee /etc/apt/trusted.gpg.d/lunarg.asc

sudo wget -qO /etc/apt/sources.list.d/lunarg-vulkan-jammy.list \
    http://packages.lunarg.com/vulkan/lunarg-vulkan-jammy.list

# Установка Vulkan SDK
sudo apt update
sudo apt install -y vulkan-sdk

# Проверка
vulkaninfo --summary
```

#### Опция B: Ручная установка

```bash
# Скачивание Vulkan SDK
wget https://sdk.lunarg.com/sdk/download/latest/linux/vulkan-sdk.tar.gz

# Установка
tar -xzf vulkan-sdk.tar.gz
cd VulkanSDK/*
./vulkan --install

# Настройка переменных окружения
echo 'source $HOME/VulkanSDK/*/setup-env.sh' >> ~/.bashrc
source ~/.bashrc
```

### Шаг 3: Установка vcpkg

```bash
# Клонирование vcpkg
git clone https://github.com/Microsoft/vcpkg.git $HOME/vcpkg

# Сборка vcpkg
cd $HOME/vcpkg
./bootstrap-vcpkg.sh

# Добавление в PATH (опционально)
echo 'export PATH=$HOME/vcpkg:$PATH' >> ~/.bashrc
source ~/.bashrc
```

### Шаг 4: Клонирование SpectraForge

```bash
# Клонирование репозитория
git clone https://github.com/TiGRoNdev/SpectraForge.git
cd SpectraForge

# Проверка версии
git log --oneline -1
```

---

## 🚀 Установка NVIDIA SDK

### Опция 1: Автоматическая установка (Рекомендуется)

```bash
cd SpectraForge
chmod +x scripts/install_nvidia_sdks.sh
./scripts/install_nvidia_sdks.sh
```

**Меню скрипта:**
```
1) CUDA Toolkit (обязательно для CUDA/OptiX)
2) OptiX SDK (требует CUDA)
3) DLSS SDK / Streamline (опционально)
4) FidelityFX FSR (опционально)
5) Установить ВСЁ
6) Проверка установленных SDK
0) Выход
```

Выберите опцию **5** для полной установки.

### Опция 2: Ручная установка

#### 2.1. CUDA Toolkit

**Ubuntu 22.04:**

```bash
# Скачивание CUDA keyring
wget https://developer.download.nvidia.com/compute/cuda/repos/ubuntu2204/x86_64/cuda-keyring_1.1-1_all.deb

# Установка keyring
sudo dpkg -i cuda-keyring_1.1-1_all.deb
rm cuda-keyring_1.1-1_all.deb

# Обновление и установка CUDA
sudo apt-get update
sudo apt-get install -y cuda-toolkit-12-8

# Настройка переменных окружения
cat >> ~/.bashrc << 'EOF'

# CUDA Toolkit
export CUDA_PATH=/usr/local/cuda
export PATH=$CUDA_PATH/bin:$PATH
export LD_LIBRARY_PATH=$CUDA_PATH/lib64:$LD_LIBRARY_PATH
export CUDA_HOME=$CUDA_PATH
EOF

# Применение изменений
source ~/.bashrc

# Проверка
nvcc --version
nvidia-smi
```

**Fedora/RHEL:**

```bash
# Добавление репозитория CUDA
sudo dnf config-manager --add-repo \
    https://developer.download.nvidia.com/compute/cuda/repos/rhel9/x86_64/cuda-rhel9.repo

# Установка драйвера и CUDA
sudo dnf clean all
sudo dnf -y module install nvidia-driver:latest-dkms
sudo dnf -y install cuda-toolkit-12-8

# Настройка переменных окружения
cat >> ~/.bashrc << 'EOF'

# CUDA Toolkit
export CUDA_PATH=/usr/local/cuda
export PATH=$CUDA_PATH/bin:$PATH
export LD_LIBRARY_PATH=$CUDA_PATH/lib64:$LD_LIBRARY_PATH
EOF

source ~/.bashrc
```

#### 2.2. OptiX SDK

OptiX SDK требует ручной загрузки с сайта NVIDIA (требуется бесплатная регистрация).

**Инструкция:**

1. Посетите: https://developer.nvidia.com/optix/downloads
2. Войдите в NVIDIA Developer Account (регистрация бесплатна)
3. Скачайте **OptiX SDK 7.7.0** для Linux (файл: `NVIDIA-OptiX-SDK-7.7.0-linux64-x86_64.sh`)
4. Сохраните в директорию загрузок

**Установка:**

```bash
# Переход в директорию загрузок
cd ~/Downloads

# Установка прав на выполнение
chmod +x NVIDIA-OptiX-SDK-7.7.0-linux64-x86_64.sh

# Установка OptiX
mkdir -p $HOME/nvidia-sdks
./NVIDIA-OptiX-SDK-7.7.0-linux64-x86_64.sh --skip-license --prefix=$HOME/nvidia-sdks/optix-7.7.0

# Настройка переменных окружения
cat >> ~/.bashrc << 'EOF'

# OptiX SDK
export OptiX_ROOT_DIR=$HOME/nvidia-sdks/optix-7.7.0
export OPTIX_ROOT=$OptiX_ROOT_DIR
EOF

source ~/.bashrc

# Проверка
ls $OptiX_ROOT_DIR/include/optix.h
```

#### 2.3. DLSS SDK (Streamline) - Опционально

**Инструкция:**

1. Посетите: https://developer.nvidia.com/rtx/streamline
2. Войдите в NVIDIA Developer Account
3. Скачайте последнюю версию Streamline SDK
4. Извлеките архив

**Установка:**

```bash
# Извлечение в директорию SDK
mkdir -p $HOME/nvidia-sdks/streamline
cd ~/Downloads
unzip Streamline_SDK_*.zip -d $HOME/nvidia-sdks/streamline

# Настройка переменных окружения
cat >> ~/.bashrc << 'EOF'

# DLSS SDK (Streamline)
export STREAMLINE_ROOT=$HOME/nvidia-sdks/streamline
export DLSS_ROOT_DIR=$STREAMLINE_ROOT
EOF

source ~/.bashrc

# Проверка
ls $STREAMLINE_ROOT/include/sl.h
```

#### 2.4. FidelityFX FSR - Опционально

```bash
# Клонирование репозитория FidelityFX FSR2
cd $HOME/nvidia-sdks
git clone https://github.com/GPUOpen-Effects/FidelityFX-FSR2.git

# Настройка переменных окружения
cat >> ~/.bashrc << 'EOF'

# FidelityFX FSR
export FIDELITYFX_ROOT=$HOME/nvidia-sdks/FidelityFX-FSR2
export FSR_ROOT_DIR=$FIDELITYFX_ROOT
EOF

source ~/.bashrc

# Проверка
ls $FIDELITYFX_ROOT/sdk/include/FidelityFX/host/ffx_fsr2.h
```

---

## 🔨 Сборка проекта

### Конфигурация 1: Минимальная (только Vulkan)

```bash
cd SpectraForge

cmake -B build -G Ninja \
  -DCMAKE_TOOLCHAIN_FILE=$HOME/vcpkg/scripts/buildsystems/vcpkg.cmake \
  -DCMAKE_BUILD_TYPE=Release \
  -DBUILD_WITH_CUDA=OFF \
  -DBUILD_WITH_OPTIX=OFF \
  -DBUILD_WITH_DLSS=OFF \
  -DBUILD_WITH_FSR=OFF

cmake --build build --config Release -j$(nproc)
```

### Конфигурация 2: С CUDA (без OptiX/DLSS/FSR)

```bash
cmake -B build -G Ninja \
  -DCMAKE_TOOLCHAIN_FILE=$HOME/vcpkg/scripts/buildsystems/vcpkg.cmake \
  -DCMAKE_EXPORT_COMPILE_COMMANDS=ON \
  -DCMAKE_BUILD_TYPE=Release \
  -DBUILD_WITH_CUDA=ON \
  -DBUILD_WITH_OPTIX=OFF \
  -DBUILD_WITH_DLSS=OFF \
  -DBUILD_WITH_FSR=OFF

cmake --build build --config Release -j$(nproc)
```

### Конфигурация 3: Полная (все функции)

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

### Конфигурация 4: Debug (для разработки)

```bash
cmake -B build-debug -G Ninja \
  -DCMAKE_TOOLCHAIN_FILE=$HOME/vcpkg/scripts/buildsystems/vcpkg.cmake \
  -DCMAKE_BUILD_TYPE=Debug \
  -DCMAKE_EXPORT_COMPILE_COMMANDS=ON \
  -DBUILD_TESTS=ON \
  -DENABLE_COVERAGE=ON \
  -DBUILD_WITH_CUDA=ON \
  -DBUILD_WITH_OPTIX=ON

cmake --build build-debug -j$(nproc)
```

---

## ✅ Проверка установки

### 1. Проверка SDK

```bash
# Запуск скрипта проверки
cd SpectraForge
./scripts/setup_sdk.sh
```

Ожидаемый вывод:
```
[1/4] Проверка CUDA Toolkit...
✓ CUDA Toolkit найден
Cuda compilation tools, release 12.8, V12.8.0

[2/4] Проверка OptiX SDK...
✓ OptiX SDK найден: /home/user/nvidia-sdks/optix-7.7.0

[3/4] Проверка DLSS SDK (опционально)...
✓ DLSS SDK найден: /home/user/nvidia-sdks/streamline

[4/4] Проверка FidelityFX SDK...
✓ FidelityFX SDK найден: /home/user/nvidia-sdks/FidelityFX-FSR2
```

### 2. Проверка сборки

```bash
# Запуск тестов
cd build
ctest --output-on-failure

# Ожидаемый результат:
# 100% tests passed
```

### 3. Запуск примеров

```bash
# Базовое Vulkan demo
./VulkanBasic_Demo

# FlashGS demo (если CUDA установлена)
./flashgs_demo

# OptiX ray tracing demo (если OptiX установлен)
./optix_raytracer_demo

# Адаптер рендерера demo
./RendererAdapter_Demo
```

---

## 🛠️ Решение проблем

### Проблема 1: CUDA компилятор не найден

**Симптомы:**
```
CMake Warning at CMakeLists.txt:29 (message):
  CUDA compiler not found. CUDA support will be disabled.
```

**Решение:**

```bash
# Проверка наличия nvcc
which nvcc

# Если не найден:
export PATH=/usr/local/cuda/bin:$PATH
export LD_LIBRARY_PATH=/usr/local/cuda/lib64:$LD_LIBRARY_PATH

# Добавьте в ~/.bashrc для постоянного применения
echo 'export PATH=/usr/local/cuda/bin:$PATH' >> ~/.bashrc
echo 'export LD_LIBRARY_PATH=/usr/local/cuda/lib64:$LD_LIBRARY_PATH' >> ~/.bashrc
source ~/.bashrc

# Перезапустите CMake
cmake -B build -G Ninja ... (повторите команду конфигурации)
```

### Проблема 2: OptiX SDK не найден

**Симптомы:**
```
Could NOT find OptiX (missing: OptiX_ROOT_DIR OptiX_INCLUDE_DIR)
CMake Warning: OptiX SDK not found. Ray tracing features will be disabled.
```

**Решение:**

```bash
# Проверьте переменную окружения
echo $OptiX_ROOT_DIR

# Если пусто, установите:
export OptiX_ROOT_DIR=$HOME/nvidia-sdks/optix-7.7.0
export OPTIX_ROOT=$OptiX_ROOT_DIR

# Проверьте наличие файла optix.h
ls $OptiX_ROOT_DIR/include/optix.h

# Если файла нет, переустановите OptiX (см. Шаг 2.2)

# Добавьте в ~/.bashrc
echo 'export OptiX_ROOT_DIR=$HOME/nvidia-sdks/optix-7.7.0' >> ~/.bashrc
echo 'export OPTIX_ROOT=$OptiX_ROOT_DIR' >> ~/.bashrc
source ~/.bashrc
```

### Проблема 3: DLSS SDK не найден

**Симптомы:**
```
Could NOT find DLSS (missing: DLSS_ROOT_DIR DLSS_INCLUDE_DIR DLSS_LIBRARY)
```

**Решение:**

```bash
# Если DLSS не требуется, соберите без него:
cmake -B build -G Ninja ... -DBUILD_WITH_DLSS=OFF

# Если DLSS требуется:
export STREAMLINE_ROOT=$HOME/nvidia-sdks/streamline
export DLSS_ROOT_DIR=$STREAMLINE_ROOT

# Проверьте файл sl.h
ls $STREAMLINE_ROOT/include/sl.h
```

### Проблема 4: FSR SDK не найден

**Симптомы:**
```
Could NOT find FSR (missing: FSR_ROOT_DIR FSR_INCLUDE_DIR FSR_API_LIBRARY)
```

**Решение:**

```bash
# Если FSR не требуется:
cmake -B build -G Ninja ... -DBUILD_WITH_FSR=OFF

# Если FSR требуется:
export FIDELITYFX_ROOT=$HOME/nvidia-sdks/FidelityFX-FSR2
export FSR_ROOT_DIR=$FIDELITYFX_ROOT

# Клонирование FSR если отсутствует
cd $HOME/nvidia-sdks
git clone https://github.com/GPUOpen-Effects/FidelityFX-FSR2.git
```

### Проблема 5: Ошибки линковки Vulkan

**Симптомы:**
```
error: undefined reference to 'vkCreateInstance'
```

**Решение:**

```bash
# Установите Vulkan SDK
sudo apt install -y vulkan-sdk

# Или установите вручную
wget https://sdk.lunarg.com/sdk/download/latest/linux/vulkan-sdk.tar.gz
tar -xzf vulkan-sdk.tar.gz
```

### Проблема 6: vcpkg не установлен

**Симптомы:**
```
CMake Error: Could not find CMAKE_TOOLCHAIN_FILE
```

**Решение:**

```bash
# Установка vcpkg
git clone https://github.com/Microsoft/vcpkg.git $HOME/vcpkg
$HOME/vcpkg/bootstrap-vcpkg.sh

# Используйте полный путь в CMake
cmake -B build -G Ninja \
  -DCMAKE_TOOLCHAIN_FILE=$HOME/vcpkg/scripts/buildsystems/vcpkg.cmake \
  ...
```

---

## ❓ FAQ

### В1: Можно ли собрать SpectraForge без NVIDIA GPU?

**Ответ:** Да! Базовая версия с Vulkan работает на любой Vulkan-совместимой видеокарте (NVIDIA, AMD, Intel). Функции CUDA, OptiX и DLSS требуют GPU от NVIDIA.

Соберите с флагами:
```bash
-DBUILD_WITH_CUDA=OFF -DBUILD_WITH_OPTIX=OFF -DBUILD_WITH_DLSS=OFF
```

### В2: Какая минимальная версия CUDA требуется?

**Ответ:** CUDA 11.8 или новее. Рекомендуется CUDA 12.x для лучшей производительности.

### В3: Нужно ли устанавливать все SDK?

**Ответ:** Нет. Обязателен только Vulkan SDK. CUDA, OptiX, DLSS и FSR - опциональные компоненты для расширенных функций.

### В4: Работает ли SpectraForge на WSL2?

**Ответ:** Да, с WSLg (Windows Subsystem for Linux GUI). Требуется Windows 11 или Windows 10 Build 19044+.

### В5: Сколько места на диске требуется?

**Ответ:**
- Базовая сборка: ~2 GB
- С CUDA: ~5 GB
- С OptiX: ~6 GB
- Полная сборка со всеми SDK: ~10 GB

### В6: Как обновить SDK?

**Ответ:**

```bash
# CUDA Toolkit
sudo apt-get update
sudo apt-get upgrade cuda-toolkit-12-8

# OptiX - скачайте новую версию с сайта NVIDIA

# FSR - обновите git репозиторий
cd $FIDELITYFX_ROOT
git pull
```

---

## 📞 Поддержка

### Официальные каналы

- **GitHub Issues:** https://github.com/TiGRoNdev/SpectraForge/issues
- **GitHub Discussions:** https://github.com/TiGRoNdev/SpectraForge/discussions
- **Email:** tigron.dev@gmail.com

### Полезные ссылки

- **NVIDIA Developer:** https://developer.nvidia.com/
- **Vulkan Tutorial:** https://vulkan-tutorial.com/
- **CUDA Documentation:** https://docs.nvidia.com/cuda/
- **OptiX Documentation:** https://raytracing-docs.nvidia.com/optix7/

---

## 🎓 Дополнительные ресурсы

- [BUILD_INSTRUCTIONS.md](BUILD_INSTRUCTIONS.md) - Инструкции по сборке
- [DEPENDENCIES.md](DEPENDENCIES.md) - Список зависимостей
- [CONTRIBUTING.md](CONTRIBUTING.md) - Руководство по участию
- [docs/guides/](docs/guides/) - Дополнительные руководства

---

## 📝 Лицензия

SpectraForge распространяется под лицензией [MIT License](LICENSE).

---

**Успешной работы с SpectraForge! 🚀**

*Последнее обновление: 1 октября 2025*
