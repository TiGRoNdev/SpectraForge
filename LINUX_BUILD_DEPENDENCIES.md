# Инструкция по установке зависимостей для Linux

## 1. Установка системных пакетов

Выполните следующую команду для установки всех необходимых зависимостей:

```bash
sudo apt update && sudo apt install -y \
    libglfw3-dev \
    libvulkan-dev \
    vulkan-tools \
    glslang-tools \
    spirv-tools \
    build-essential \
    cmake \
    git \
    pkg-config \
    libx11-dev
```

## 2. VulkanMemoryAllocator (VMA)

✅ **Уже установлен!** VMA был скачан в папку `external/vk_mem_alloc.h`

## 3. Проверка установки

Проверьте, что все зависимости установлены:

```bash
# Проверка GLFW
pkg-config --modversion glfw3

# Проверка Vulkan
pkg-config --modversion vulkan

# Проверка glslc (shader compiler)
which glslc
```

## 4. Пересборка проекта

После установки зависимостей:

```bash
cd /home/tigron/Documents/SpectraForge

# Удаляем старую сборку
rm -rf build_linux

# Запускаем сборку
./build_linux.sh
```

## Изменения в проекте

- ✅ Добавлен `external/vk_mem_alloc.h` для VulkanMemoryAllocator
- ✅ Обновлен `CMakeLists.txt` с поиском GLFW
- ✅ Добавлены пути включения для VMA
- ✅ Добавлена линковка GLFW в ядро проекта

## Устранение проблем

### Ошибка: vk_mem_alloc.h not found
- Проверьте наличие файла: `ls external/vk_mem_alloc.h`
- Если отсутствует, скачайте снова:
```bash
mkdir -p external
cd external
wget https://raw.githubusercontent.com/GPUOpen-LibrariesAndSDKs/VulkanMemoryAllocator/master/include/vk_mem_alloc.h
```

### Ошибка: GLFW/glfw3.h not found
```bash
sudo apt install libglfw3-dev
```

### Ошибка: glslc not found
```bash
sudo apt install glslang-tools
```

