# 🎯 Следующие шаги для завершения установки HyperEngine

> **Статус:** SDK не установлены  
> **Дата:** 1 октября 2025

## 📊 Текущее состояние

### ✅ Установлено и работает:
- ✓ Vulkan SDK 1.3.280
- ✓ Системные библиотеки (glfw3, glm, vulkan)
- ✓ vcpkg и базовые зависимости
- ✓ CMake и инструменты сборки

### ❌ Отсутствует (требуется для полной функциональности):
- ✗ CUDA Toolkit (для GPU ускорения FlashGS)
- ✗ OptiX SDK (для ray tracing)
- ⚠ DLSS SDK (опционально для NVIDIA upscaling)
- ⚠ FSR SDK (опционально для AMD upscaling)

---

## 🚀 Вариант 1: Автоматическая установка (РЕКОМЕНДУЕТСЯ)

### Шаг 1: Запустите установщик

```bash
cd /home/tigron/Documents/GITHUB/HyperEngine
chmod +x scripts/install_nvidia_sdks.sh
./scripts/install_nvidia_sdks.sh
```

### Шаг 2: Выберите опцию в меню

```
======================================
Выберите компоненты для установки:
======================================
1) CUDA Toolkit (обязательно для CUDA/OptiX)
2) OptiX SDK (требует CUDA)
3) DLSS SDK / Streamline (опционально)
4) FidelityFX FSR (опционально)
5) Установить ВСЁ              <- ВЫБЕРИТЕ ЭТУ ОПЦИЮ
6) Проверка установленных SDK
0) Выход
```

**Выберите опцию 5** для автоматической установки всех компонентов.

### Шаг 3: Следуйте инструкциям скрипта

Скрипт:
- ✅ Автоматически установит CUDA Toolkit
- ⚠️ Попросит вас скачать OptiX SDK с сайта NVIDIA (требуется регистрация)
- ⚠️ Попросит вас скачать DLSS SDK с сайта NVIDIA (требуется регистрация)
- ✅ Автоматически клонирует FidelityFX FSR

### Шаг 4: Примените переменные окружения

```bash
source ~/.bashrc
```

### Шаг 5: Пересоберите проект

```bash
cd /home/tigron/Documents/GITHUB/HyperEngine

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

---

## 🔧 Вариант 2: Ручная установка

### 2.1. Установка CUDA Toolkit

```bash
# Скачивание CUDA keyring
wget https://developer.download.nvidia.com/compute/cuda/repos/ubuntu2204/x86_64/cuda-keyring_1.1-1_all.deb
sudo dpkg -i cuda-keyring_1.1-1_all.deb

# Установка CUDA
sudo apt-get update
sudo apt-get install -y cuda-toolkit-12-8

# Настройка переменных окружения
cat >> ~/.bashrc << 'EOF'

# CUDA Toolkit
export CUDA_PATH=/usr/local/cuda
export PATH=$CUDA_PATH/bin:$PATH
export LD_LIBRARY_PATH=$CUDA_PATH/lib64:$LD_LIBRARY_PATH
EOF

source ~/.bashrc
nvcc --version
```

### 2.2. Установка OptiX SDK

1. **Посетите:** https://developer.nvidia.com/optix/downloads
2. **Войдите** в NVIDIA Developer Account (бесплатная регистрация)
3. **Скачайте** OptiX SDK 7.7.0 для Linux
4. **Установите:**

```bash
cd ~/Downloads
chmod +x NVIDIA-OptiX-SDK-7.7.0-linux64-x86_64.sh
mkdir -p $HOME/nvidia-sdks
./NVIDIA-OptiX-SDK-7.7.0-linux64-x86_64.sh --skip-license --prefix=$HOME/nvidia-sdks/optix-7.7.0

cat >> ~/.bashrc << 'EOF'

# OptiX SDK
export OptiX_ROOT_DIR=$HOME/nvidia-sdks/optix-7.7.0
export OPTIX_ROOT=$OptiX_ROOT_DIR
EOF

source ~/.bashrc
```

### 2.3. Установка DLSS SDK (опционально)

1. **Посетите:** https://developer.nvidia.com/rtx/streamline
2. **Скачайте** Streamline SDK
3. **Извлеките** в `$HOME/nvidia-sdks/streamline`
4. **Настройте:**

```bash
cat >> ~/.bashrc << 'EOF'

# DLSS SDK (Streamline)
export STREAMLINE_ROOT=$HOME/nvidia-sdks/streamline
export DLSS_ROOT_DIR=$STREAMLINE_ROOT
EOF

source ~/.bashrc
```

### 2.4. Установка FSR SDK (опционально)

```bash
cd $HOME/nvidia-sdks
git clone https://github.com/GPUOpen-Effects/FidelityFX-FSR2.git

cat >> ~/.bashrc << 'EOF'

# FidelityFX FSR
export FIDELITYFX_ROOT=$HOME/nvidia-sdks/FidelityFX-FSR2
export FSR_ROOT_DIR=$FIDELITYFX_ROOT
EOF

source ~/.bashrc
```

---

## 🎯 Вариант 3: Сборка без расширенных SDK

Если вам не нужны CUDA, OptiX, DLSS или FSR, соберите базовую версию:

```bash
cd /home/tigron/Documents/GITHUB/HyperEngine

cmake -B build -G Ninja \
  -DCMAKE_TOOLCHAIN_FILE=$HOME/vcpkg/scripts/buildsystems/vcpkg.cmake \
  -DCMAKE_BUILD_TYPE=Release \
  -DBUILD_WITH_CUDA=OFF \
  -DBUILD_WITH_OPTIX=OFF \
  -DBUILD_WITH_DLSS=OFF \
  -DBUILD_WITH_FSR=OFF

cmake --build build --config Release -j$(nproc)
```

**Эта конфигурация будет работать ПРЯМО СЕЙЧАС** с уже установленными компонентами.

---

## ✅ Проверка установки

После установки SDK запустите:

```bash
cd /home/tigron/Documents/GITHUB/HyperEngine
./scripts/setup_sdk.sh
```

**Ожидаемый результат после установки:**

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

## 📚 Дополнительная документация

Созданы следующие файлы с подробными инструкциями:

1. **[QUICK_INSTALL.md](QUICK_INSTALL.md)** - Быстрая установка (на английском)
2. **[INSTALL_GUIDE_RU.md](INSTALL_GUIDE_RU.md)** - Полное руководство (на русском)
3. **[SDK_INSTALLATION_ISSUES.md](SDK_INSTALLATION_ISSUES.md)** - Решение проблем с SDK
4. **[scripts/install_nvidia_sdks.sh](scripts/install_nvidia_sdks.sh)** - Автоматический установщик

---

## 🎓 Обучающие материалы

### Что делает каждый SDK:

| SDK | Назначение | Обязательность |
|-----|-----------|---------------|
| **CUDA** | GPU ускорение для FlashGS Gaussian Splatting (до 4x быстрее) | Опционально |
| **OptiX** | Ray tracing для вторичных эффектов (отражения, преломления) | Опционально |
| **DLSS** | AI upscaling от NVIDIA (до 8x FPS на RTX картах) | Опционально |
| **FSR** | Upscaling от AMD (до 2x FPS на всех картах) | Опционально |
| **Vulkan** | Базовый рендеринг | **Обязательно** |

### Рекомендации по конфигурации:

**Для разработки (максимум функций):**
```bash
-DBUILD_WITH_CUDA=ON -DBUILD_WITH_OPTIX=ON -DBUILD_WITH_DLSS=ON -DBUILD_WITH_FSR=ON
```

**Для продакшена (только CUDA):**
```bash
-DBUILD_WITH_CUDA=ON -DBUILD_WITH_OPTIX=OFF -DBUILD_WITH_DLSS=OFF -DBUILD_WITH_FSR=OFF
```

**Для быстрого тестирования (только Vulkan):**
```bash
-DBUILD_WITH_CUDA=OFF -DBUILD_WITH_OPTIX=OFF -DBUILD_WITH_DLSS=OFF -DBUILD_WITH_FSR=OFF
```

---

## 🆘 Помощь и поддержка

### Если возникли проблемы:

1. **Проверьте логи:**
   ```bash
   cmake -B build ... 2>&1 | tee cmake_log.txt
   cat cmake_log.txt | grep -i "warning\|error"
   ```

2. **Проверьте переменные окружения:**
   ```bash
   env | grep -E "CUDA|OPTIX|DLSS|FSR|VULKAN"
   ```

3. **Создайте issue на GitHub:**
   - URL: https://github.com/TiGRoNdev/HyperEngine/issues
   - Приложите вывод команд выше

### Контакты:

- **GitHub Issues:** https://github.com/TiGRoNdev/HyperEngine/issues
- **GitHub Discussions:** https://github.com/TiGRoNdev/HyperEngine/discussions
- **Email:** tigron.dev@gmail.com

---

## 📝 Краткое резюме команд

### Для автоматической установки:
```bash
cd /home/tigron/Documents/GITHUB/HyperEngine
./scripts/install_nvidia_sdks.sh  # Выбрать опцию 5
source ~/.bashrc
cmake -B build -G Ninja -DCMAKE_TOOLCHAIN_FILE=$HOME/vcpkg/scripts/buildsystems/vcpkg.cmake -DBUILD_WITH_CUDA=ON -DBUILD_WITH_OPTIX=ON -DBUILD_WITH_DLSS=ON -DBUILD_WITH_FSR=ON
cmake --build build --config Release -j$(nproc)
```

### Для быстрой сборки (без SDK):
```bash
cd /home/tigron/Documents/GITHUB/HyperEngine
cmake -B build -G Ninja -DCMAKE_TOOLCHAIN_FILE=$HOME/vcpkg/scripts/buildsystems/vcpkg.cmake -DBUILD_WITH_CUDA=OFF -DBUILD_WITH_OPTIX=OFF -DBUILD_WITH_DLSS=OFF -DBUILD_WITH_FSR=OFF
cmake --build build --config Release -j$(nproc)
```

---

**Выберите вариант и начните установку! 🚀**

*Если нужна помощь - обращайтесь в GitHub Issues*

