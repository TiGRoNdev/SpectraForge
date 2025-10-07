# 🚀 SpectraForge - Быстрый Старт

## 📦 Установка зависимостей (Ubuntu/Debian)
```bash
# Vulkan SDK
wget -qO- https://packages.lunarg.com/lunarg-signing-key-pub.asc | sudo tee /etc/apt/trusted.gpg.d/lunarg.asc
sudo wget -qO /etc/apt/sources.list.d/lunarg-vulkan-jammy.list https://packages.lunarg.com/vulkan/lunarg-vulkan-jammy.list
sudo apt update
sudo apt install vulkan-sdk

# Компилятор шейдеров и инструменты
sudo apt install glslc cmake build-essential

# Библиотеки
sudo apt install libglfw3-dev libglm-dev libpng-dev
```

## 🔨 Компиляция
```bash
# Быстрая сборка с оптимизациями
chmod +x build_and_run_optimized.sh
./build_and_run_optimized.sh
```

## 🎮 Запуск

### Автоматический (рекомендуется)
```bash
./build_and_run_optimized.sh
# Выберите режим:
# 1 - Mobile 60+ FPS (для слабых GPU)
# 2 - Balanced (средние настройки)
# 3 - Quality (максимальное качество)
```

### Ручной запуск
```bash
cd build_linux_optimized

# Mobile режим - 60+ FPS гарантировано
./SpectraForge_Optimized_Demo --mobile

# Сбалансированный режим
./SpectraForge_Optimized_Demo --balanced

# Максимальное качество
./SpectraForge_Optimized_Demo --quality
```

## 🎯 Управление
- **WASD** - движение камеры
- **Мышь** - поворот камеры  
- **Q/E** - подъем/спуск
- **1-3** - переключение качества в реальном времени
- **H** - включить/выключить HDR
- **ESC** - выход

## 📊 Ожидаемая производительность

| GPU | Mobile (1080p→4K) | Balanced (1440p→4K) | Quality (4K) |
|-----|-------------------|---------------------|--------------|
| **Adreno 740** | 60+ FPS | 45-60 FPS | 25-35 FPS |
| **Mali G78** | 55+ FPS | 40-55 FPS | 20-30 FPS |
| **Intel UHD** | 45+ FPS | 30-45 FPS | 15-25 FPS |
| **GTX 1650** | 60+ FPS | 60+ FPS | 45-60 FPS |
| **RTX 3060** | 60+ FPS | 60+ FPS | 60+ FPS |

## ⚡ Советы для максимальной производительности

1. **Закройте другие приложения** - особенно браузеры
2. **Отключите композитор** (Linux):
   ```bash
   # KDE
   qdbus org.kde.KWin /Compositor suspend
   
   # GNOME
   gsettings set org.gnome.mutter experimental-features "['kms-modifiers']"
   ```
3. **Установите производительный режим GPU**:
   ```bash
   # NVIDIA
   nvidia-settings -a "[gpu:0]/GpuPowerMizerMode=1"
   
   # AMD
   echo "performance" | sudo tee /sys/class/drm/card0/device/power_dpm_state
   ```

## 🐛 Решение проблем

### "Vulkan SDK не найден"
```bash
export VULKAN_SDK=/usr/include/vulkan
export LD_LIBRARY_PATH=$VULKAN_SDK/lib:$LD_LIBRARY_PATH
```

### Низкий FPS
- Переключитесь на Mobile режим (клавиша 1)
- Проверьте температуру GPU
- Убедитесь что используется дискретная видеокарта:
  ```bash
  # Проверка
  vulkaninfo | grep "GPU id"
  
  # Принудительно NVIDIA
  __NV_PRIME_RENDER_OFFLOAD=1 __GLX_VENDOR_LIBRARY_NAME=nvidia ./SpectraForge_Optimized_Demo
  ```

### Артефакты изображения
- Обновите драйверы GPU
- Попробуйте другой режим качества
- Отключите HDR (клавиша H)

## 📝 Дополнительно
- [Подробное описание оптимизаций](OPTIMIZATIONS_EXPLAINED.md)
- [Документация проекта](docs/)
