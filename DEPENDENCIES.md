# Зависимости SpectraForge

> **Версия:** 2.0  
> **Дата обновления:** 2025-10-01  
> **Оптимизировано для:** Claude 4.5 Sonnet

## 📋 Обзор

SpectraForge - это современный 3D/4D движок с поддержкой передовых технологий рендеринга:
- **Vulkan** - современный графический API
- **CUDA** - вычисления на GPU (NVIDIA)
- **OptiX** - трассировка лучей (NVIDIA)
- **DLSS** - AI апскейлинг (NVIDIA RTX)
- **FSR** - апскейлинг (AMD/универсальный)

## 🚀 Быстрая установка

### Автоматическая установка (Linux)

```bash
# Переход в директорию проекта
cd /home/tigron/Documents/GITHUB/SpectraForge

# Запуск скрипта установки
./scripts/install_dependencies.sh
```

Скрипт автоматически установит:
1. ✅ Системные зависимости
2. ✅ Vulkan SDK
3. ⚙️ CUDA Toolkit (опционально)
4. ⚙️ OptiX SDK (опционально)
5. ⚙️ DLSS/Streamline SDK (опционально)
6. ⚙️ FSR SDK (опционально)
7. ✅ vcpkg и зависимости проекта

### Ручная установка

Если автоматическая установка не подходит, следуйте инструкциям ниже.

## 📦 Обязательные зависимости

### 1. Системные инструменты

#### Ubuntu/Debian
```bash
sudo apt-get update
sudo apt-get install -y \
    build-essential \
    cmake \
    git \
    wget \
    curl \
    pkg-config \
    ninja-build \
    python3 \
    python3-pip
```

#### Fedora/RHEL
```bash
sudo dnf install -y \
    gcc \
    gcc-c++ \
    cmake \
    git \
    wget \
    curl \
    pkg-config \
    ninja-build \
    python3 \
    python3-pip
```

#### Arch Linux
```bash
sudo pacman -Syu --noconfirm \
    base-devel \
    cmake \
    git \
    wget \
    curl \
    pkg-config \
    ninja \
    python \
    python-pip
```

### 2. Vulkan SDK

**Требуемая версия:** ≥ 1.3.0

#### Ubuntu/Debian
```bash
# Добавление репозитория LunarG
wget -qO - https://packages.lunarg.com/lunarg-signing-key-pub.asc | sudo apt-key add -
sudo wget -qO /etc/apt/sources.list.d/lunarg-vulkan-1.3.280-jammy.list \
    https://packages.lunarg.com/vulkan/1.3.280/lunarg-vulkan-1.3.280-jammy.list

# Установка
sudo apt-get update
sudo apt-get install -y vulkan-sdk
```

#### Fedora
```bash
sudo dnf install -y vulkan-loader vulkan-tools vulkan-validation-layers-devel
```

#### Arch Linux
```bash
sudo pacman -S --noconfirm vulkan-devel vulkan-tools vulkan-validation-layers
```

#### Проверка установки
```bash
vulkaninfo --summary
```

### 3. vcpkg и зависимости проекта

```bash
# Установка vcpkg
cd ~
git clone https://github.com/Microsoft/vcpkg.git
cd vcpkg
./bootstrap-vcpkg.sh

# Добавление в переменные окружения
echo "export VCPKG_ROOT=$HOME/vcpkg" >> ~/.bashrc
echo 'export PATH=$VCPKG_ROOT:$PATH' >> ~/.bashrc
source ~/.bashrc

# Переход в директорию проекта
cd /home/tigron/Documents/GITHUB/SpectraForge

# Установка зависимостей проекта
$VCPKG_ROOT/vcpkg install \
    glfw3 \
    glew \
    vulkan \
    vulkan-memory-allocator \
    vulkan-hpp \
    spirv-cross \
    shaderc \
    imgui[vulkan-binding] \
    assimp \
    stb \
    glm \
    gtest
```

#### Список зависимостей vcpkg

| Пакет | Описание | Версия |
|-------|----------|--------|
| glfw3 | Библиотека окон и ввода | ≥ 3.3 |
| glew | OpenGL Extension Wrangler | ≥ 2.1 |
| vulkan | Vulkan SDK | ≥ 1.3 |
| vulkan-memory-allocator | VMA для управления памятью Vulkan | latest |
| vulkan-hpp | C++ биндинги для Vulkan | latest |
| spirv-cross | Инструменты компиляции шейдеров | latest |
| shaderc | Компилятор GLSL в SPIR-V | latest |
| imgui | Библиотека GUI | latest |
| assimp | Загрузка 3D моделей | ≥ 5.0 |
| stb | Загрузка изображений | latest |
| glm | Математическая библиотека | ≥ 0.9.9 |
| gtest | Google Test Framework | ≥ 1.12 |

## ⚙️ Опциональные зависимости

### 4. NVIDIA CUDA Toolkit

**Требуется для:** Ускорение вычислений на GPU, CUDA-Vulkan interop  
**Требуемая версия:** ≥ 11.8  
**Минимальная GPU:** NVIDIA с compute capability ≥ 5.0

#### Ubuntu
```bash
# Скачивание установочного пакета
wget https://developer.download.nvidia.com/compute/cuda/repos/ubuntu2204/x86_64/cuda-keyring_1.1-1_all.deb
sudo dpkg -i cuda-keyring_1.1-1_all.deb

# Установка CUDA Toolkit
sudo apt-get update
sudo apt-get install -y cuda-toolkit-12-3

# Добавление в PATH
echo 'export PATH=/usr/local/cuda/bin:$PATH' >> ~/.bashrc
echo 'export LD_LIBRARY_PATH=/usr/local/cuda/lib64:$LD_LIBRARY_PATH' >> ~/.bashrc
source ~/.bashrc
```

#### Проверка установки
```bash
nvcc --version
nvidia-smi
```

#### Альтернативные методы установки
- **Официальный сайт:** https://developer.nvidia.com/cuda-downloads
- **Документация:** https://docs.nvidia.com/cuda/cuda-installation-guide-linux/

### 5. NVIDIA OptiX SDK

**Требуется для:** Трассировка лучей в реальном времени  
**Требуемая версия:** ≥ 7.5  
**Зависимости:** CUDA Toolkit  
**Минимальная GPU:** NVIDIA RTX серии

#### Установка

1. **Регистрация в NVIDIA Developer Program:**
   - Посетите: https://developer.nvidia.com/optix
   - Войдите или создайте аккаунт NVIDIA Developer
   - Примите лицензионное соглашение

2. **Скачивание SDK:**
   - Выберите версию OptiX SDK 7.7.0 или новее
   - Скачайте установщик для Linux (.sh или .tar.gz)

3. **Установка:**
   ```bash
   # Для .sh установщика
   bash NVIDIA-OptiX-SDK-7.7.0-linux64-x86_64.sh --skip-license --prefix=$HOME/optix
   
   # Для .tar.gz архива
   mkdir -p $HOME/optix
   tar -xzf NVIDIA-OptiX-SDK-7.7.0-linux64-x86_64.tar.gz -C $HOME/optix --strip-components=1
   
   # Добавление в переменные окружения
   echo "export OPTIX_ROOT=$HOME/optix" >> ~/.bashrc
   source ~/.bashrc
   ```

4. **Проверка установки:**
   ```bash
   ls $OPTIX_ROOT/include/optix.h
   ```

### 6. NVIDIA DLSS SDK (Streamline)

**Требуется для:** AI апскейлинг с DLSS  
**Требуемая версия:** Streamline ≥ 2.0  
**Минимальная GPU:** NVIDIA RTX 20-series или новее

#### Установка

1. **Регистрация в NVIDIA Developer Program:**
   - Посетите: https://developer.nvidia.com/rtx/streamline
   - Войдите в аккаунт NVIDIA Developer
   - Примите EULA для Streamline SDK

2. **Скачивание SDK:**
   - Скачайте последнюю версию Streamline SDK
   - Распакуйте архив

3. **Установка:**
   ```bash
   # Распаковка
   mkdir -p $HOME/streamline
   unzip Streamline_v2.x.x.zip -d $HOME/streamline
   
   # Добавление в переменные окружения
   echo "export STREAMLINE_ROOT=$HOME/streamline" >> ~/.bashrc
   echo "export DLSS_ROOT_DIR=$HOME/streamline" >> ~/.bashrc
   source ~/.bashrc
   ```

4. **Проверка установки:**
   ```bash
   ls $STREAMLINE_ROOT/include/sl.h
   ```

**Примечание:** DLSS работает только на NVIDIA RTX GPU и требует драйвер ≥ 520.xx

### 7. AMD FidelityFX Super Resolution (FSR)

**Требуется для:** Универсальный апскейлинг  
**Требуемая версия:** FSR 2.x или FSR 3.x  
**Поддерживаемые GPU:** Любые (AMD, NVIDIA, Intel)

#### Установка

```bash
# Клонирование репозитория
cd ~
git clone --recursive https://github.com/GPUOpen-Effects/FidelityFX-FSR2.git

# Добавление в переменные окружения
echo "export FSR_ROOT_DIR=$HOME/FidelityFX-FSR2" >> ~/.bashrc
echo "export FIDELITYFX_ROOT=$HOME/FidelityFX-FSR2" >> ~/.bashrc
source ~/.bashrc
```

#### Альтернативно - FSR 3.0
```bash
git clone --recursive https://github.com/GPUOpen-Effects/FidelityFX-FSR3.git
echo "export FSR_ROOT_DIR=$HOME/FidelityFX-FSR3" >> ~/.bashrc
```

## 🔧 Сборка проекта

### С использованием vcpkg (рекомендуется)

```bash
# Создание директории сборки
mkdir -p build && cd build

# Конфигурация CMake с vcpkg
cmake .. \
    -DCMAKE_TOOLCHAIN_FILE=$VCPKG_ROOT/scripts/buildsystems/vcpkg.cmake \
    -DCMAKE_BUILD_TYPE=Release \
    -GNinja

# Сборка
cmake --build . --config Release -j$(nproc)
```

### С опциональными компонентами

```bash
cmake .. \
    -DCMAKE_TOOLCHAIN_FILE=$VCPKG_ROOT/scripts/buildsystems/vcpkg.cmake \
    -DCMAKE_BUILD_TYPE=Release \
    -DBUILD_WITH_CUDA=ON \
    -DBUILD_WITH_OPTIX=ON \
    -DBUILD_WITH_DLSS=ON \
    -DBUILD_WITH_FSR=ON \
    -DBUILD_EXAMPLES=ON \
    -DBUILD_TESTS=ON \
    -GNinja

cmake --build . --config Release -j$(nproc)
```

### Опции сборки

| Опция | Описание | По умолчанию |
|-------|----------|--------------|
| BUILD_ENGINE_3D | Сборка 3D движка | ON |
| BUILD_ENGINE_4D | Сборка 4D движка | OFF |
| BUILD_VULKAN_RENDERER | Сборка Vulkan рендерера | ON |
| BUILD_WITH_CUDA | Сборка с поддержкой CUDA | OFF |
| BUILD_WITH_OPTIX | Сборка с поддержкой OptiX | ON |
| BUILD_WITH_DLSS | Сборка с поддержкой DLSS | ON |
| BUILD_WITH_FSR | Сборка с поддержкой FSR | ON |
| BUILD_EXAMPLES | Сборка примеров | ON |
| BUILD_TESTS | Сборка тестов | ON |
| ENABLE_CODE_COVERAGE | Включить coverage для тестов | ON |

## 🐛 Устранение проблем

### Vulkan не найден

**Проблема:** CMake не может найти Vulkan SDK

**Решение:**
```bash
# Установите переменную окружения
export VULKAN_SDK=/usr/share/vulkan

# Или укажите путь явно при конфигурации
cmake .. -DVulkan_INCLUDE_DIR=/usr/include/vulkan
```

### CUDA не найден

**Проблема:** CMake не может найти CUDA Toolkit

**Решение:**
```bash
# Проверьте наличие CUDA
which nvcc
nvcc --version

# Установите CUDA_HOME
export CUDA_HOME=/usr/local/cuda
export PATH=$CUDA_HOME/bin:$PATH
export LD_LIBRARY_PATH=$CUDA_HOME/lib64:$LD_LIBRARY_PATH
```

### OptiX не найден

**Проблема:** CMake не может найти OptiX SDK

**Решение:**
```bash
# Установите OPTIX_ROOT
export OPTIX_ROOT=/path/to/optix

# Или передайте при конфигурации
cmake .. -DOptiX_ROOT_DIR=/path/to/optix
```

### vcpkg пакеты не найдены

**Проблема:** CMake не может найти пакеты vcpkg

**Решение:**
```bash
# Убедитесь, что используется toolchain файл vcpkg
cmake .. -DCMAKE_TOOLCHAIN_FILE=$VCPKG_ROOT/scripts/buildsystems/vcpkg.cmake

# Переустановите пакеты
$VCPKG_ROOT/vcpkg remove --recurse $(vcpkg list)
$VCPKG_ROOT/vcpkg install glfw3 glew vulkan ...
```

### Ошибки компиляции CUDA

**Проблема:** Ошибки при компиляции CUDA кода

**Решение:**
```bash
# Отключите CUDA если не нужен
cmake .. -DBUILD_WITH_CUDA=OFF

# Или укажите compute capability вручную
cmake .. -DCMAKE_CUDA_ARCHITECTURES="75;86;89"
```

## 📚 Дополнительные ресурсы

### Официальная документация

- **Vulkan:** https://vulkan.lunarg.com/doc/sdk
- **CUDA:** https://docs.nvidia.com/cuda/
- **OptiX:** https://raytracing-docs.nvidia.com/optix7/
- **DLSS/Streamline:** https://developer.nvidia.com/rtx/streamline
- **FSR:** https://gpuopen.com/fidelityfx-superresolution/
- **vcpkg:** https://vcpkg.io/

### Системные требования

#### Минимальные
- **OS:** Linux (Ubuntu 20.04+, Fedora 35+, Arch Linux)
- **CPU:** x86_64 с поддержкой SSE4.2
- **RAM:** 8 GB
- **GPU:** Vulkan 1.3 совместимая
- **Место на диске:** 10 GB

#### Рекомендуемые
- **OS:** Ubuntu 22.04 LTS
- **CPU:** Intel Core i7/AMD Ryzen 7 или лучше
- **RAM:** 16 GB или больше
- **GPU:** NVIDIA RTX 3060 или AMD RX 6700 XT
- **Место на диске:** 20 GB

#### Для разработки с CUDA/OptiX/DLSS
- **GPU:** NVIDIA RTX 3060 Ti или выше
- **VRAM:** 8 GB или больше
- **Драйвер:** NVIDIA ≥ 520.xx

## 🔄 Обновление зависимостей

```bash
# Обновление системных пакетов
sudo apt-get update && sudo apt-get upgrade  # Ubuntu/Debian
sudo dnf upgrade  # Fedora

# Обновление vcpkg и пакетов
cd $VCPKG_ROOT
git pull
./bootstrap-vcpkg.sh
./vcpkg upgrade --no-dry-run

# Обновление FSR SDK
cd $FSR_ROOT_DIR
git pull --recurse-submodules
```

## 📞 Поддержка

При возникновении проблем:
1. Проверьте раздел [Устранение проблем](#-устранение-проблем)
2. Создайте issue на GitHub: https://github.com/TiGRoNdev/SpectraForge/issues
3. Приложите логи сборки и вывод `cmake ..`

---

**Версия:** 2.0  
**Последнее обновление:** 2025-10-01  
**Автор:** TiGRoNdev
