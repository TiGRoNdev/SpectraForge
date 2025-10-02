# SpectraForge Examples

Данная директория содержит демо-приложения и примеры использования SpectraForge.

## 🎨 FreqVox Renderer Demos

### FreqVox Sponza Demo ⭐ **РЕКОМЕНДУЕТСЯ**
**Файл:** `freqvox_sponza_demo.cpp`  
**Документация:** [FREQVOX_SPONZA_DEMO.md](FREQVOX_SPONZA_DEMO.md)  
**Запуск:** `./run_freqvox_sponza.sh` (из корня проекта)

Полнофункциональное демо FreqVox Renderer с:
- Загрузкой OBJ сцены (Sponza)
- Вокселизацией геометрии
- Фовеированной выборкой
- Frequency-Domain Shading (DCT/FFT)
- Temporal Reprojection
- Neural Upscaling
- Интерактивной камерой
- Детальной статистикой производительности

**Управление:**
- WASD - движение камеры
- Мышь - поворот камеры
- F/T/U - переключение оптимизаций
- ESC - выход

---

### FreqVox Demo (Базовый)
**Файл:** `freqvox_demo.cpp`  
**Запуск:** `./build/FreqVox_Demo`

Простое демо для тестирования FreqVox backend'ов:
- Проверка доступности FFT backend'ов (cuFFT, VkFFT, Simple)
- Тест прямого и обратного DCT преобразования
- Базовая инициализация компонентов FreqVox

---

## 🔧 3D Engine Demos

### 3D Demo
**Файл:** `3d_demo.cpp`  
**Запуск:** `./build/SpectraForge_3D_Demo`

Базовое демо 3D движка с примитивами и камерой.

### Optimal Renderer Demo
**Файл:** `optimal_renderer_demo.cpp`  
**Запуск:** `./build/OptimalRenderer_Demo`

Демонстрация оптимального рендерера с применением SOLID принципов.

### Renderer Adapter Demo
**Файл:** `renderer_adapter_demo.cpp`  
**Запуск:** `./build/RendererAdapter_Demo`

Демонстрация адаптера рендерера для переключения между различными реализациями.

### SOLID Principles Demo
**Файл:** `solid_principles_demo.cpp`  
**Запуск:** `./build/SolidPrinciples_Demo`

Примеры применения SOLID принципов в архитектуре движка.

---

## 🎥 Vulkan Renderer Demos

### Vulkan Demo
**Файл:** `vulkan_demo.cpp`  
**Запуск:** `./build/Vulkan_Demo`

Демонстрация Vulkan Hybrid Renderer с:
- Hardware Detection
- FlashGS (Gaussian Splatting)
- OptiX Ray Tracing
- AI Upscaling (DLSS/FSR)

### Vulkan Basic Demo
**Файл:** `vulkan_basic_demo.cpp`  
**Запуск:** `./build/VulkanBasic_Demo`

Базовое демо Vulkan рендерера.

---

## ⚡ CUDA Demos

### CUDA-Vulkan Interop Demo
**Файл:** `cuda_vulkan_interop_demo.cpp`  
**Запуск:** `./build/CudaVulkanInterop_Demo`  
**Требования:** CUDA Toolkit

Демонстрация interop между CUDA и Vulkan с external memory.

### FlashGS Demo
**Файл:** `flashgs_demo.cpp`  
**Запуск:** `./build/FlashGS_Demo`  
**Требования:** CUDA Toolkit

Демонстрация 3D Gaussian Splatting с CUDA ускорением.

### External Memory Test
**Файл:** `test_external_memory.cpp`  
**Запуск:** `./build/ExternalMemory_Test`  
**Требования:** CUDA Toolkit

Тест external memory handles между Vulkan и CUDA.

---

## 🔦 OptiX Demos

### OptiX Ray Tracer Demo
**Файл:** `optix_raytracer_demo.cpp`  
**Запуск:** `./build/OptiXRayTracer_Demo`  
**Требования:** OptiX SDK

Демонстрация OptiX ray tracing интеграции.

---

## 🖥️ Console Demos

### UTF-8 Console Demo
**Файл:** `utf8_console_demo.cpp`  
**Запуск:** `./build/UTF8Console_Demo`

Демонстрация работы с UTF-8 в консоли на различных платформах.

### Safe Console Demo
**Файл:** `safe_console_demo.cpp`  
**Запуск:** `./build/SafeConsole_Demo`

Демонстрация безопасного вывода в консоль с использованием SAFE_TO_STRING макросов.

---

## 📁 Сцены

### Sponza Scene
**Путь:** `scenes/sponza/`

Знаменитая тестовая сцена Crytek Sponza для демонстрации производительности рендеринга.

Содержит:
- `sponza.obj` - геометрия (~262 тыс. треугольников)
- `sponza.mtl` - материалы
- `*.JPG` / `*.jpg` - текстуры

**Лицензия:** Public Domain

---

## 🛠️ Сборка всех примеров

```bash
# Из корня репозитория
mkdir build && cd build

# Конфигурация с примерами
cmake .. \
    -DBUILD_EXAMPLES=ON \
    -DENABLE_FREQVOX=ON \
    -DBUILD_WITH_CUDA=ON \
    -DBUILD_WITH_OPTIX=ON

# Сборка всех примеров
cmake --build . --config Release

# Или сборка конкретного примера
cmake --build . --config Release --target FreqVox_Sponza_Demo
```

---

## 📊 Сравнение демо

| Демо | Сложность | Интерактивность | GPU | Требования | Рекомендуется для |
|------|-----------|-----------------|-----|------------|-------------------|
| **FreqVox Sponza** | ⭐⭐⭐ | ✅ Полная | ✅ | GLFW, GLM | Демонстрации FreqVox |
| FreqVox Basic | ⭐ | ❌ | Частично | - | Тестирования backend'ов |
| Vulkan Demo | ⭐⭐⭐ | ✅ Полная | ✅ | Vulkan, CUDA* | Демонстрации Vulkan |
| FlashGS Demo | ⭐⭐ | Частично | ✅ | CUDA | Тестирования GS |
| 3D Demo | ⭐ | Частично | ✅ | OpenGL | Начинающих |
| SOLID Demo | ⭐ | ❌ | ❌ | - | Изучения архитектуры |

\* опционально

---

## 🎯 Быстрый старт

### Для демонстрации FreqVox:
```bash
./run_freqvox_sponza.sh
```

### Для тестирования базового FreqVox:
```bash
./build/FreqVox_Demo
```

### Для демонстрации Vulkan:
```bash
./build/Vulkan_Demo
```

---

## 📚 Документация

- [FreqVox Sponza Demo Guide](FREQVOX_SPONZA_DEMO.md)
- [FreqVox Integration](../docs/FreqVox_Integration.md)
- [Architecture](../docs/architecture/ARCHITECTURE.md)
- [Build Instructions](../BUILD_INSTRUCTIONS.md)
- [Developer Guide](../docs/DEVELOPER_GUIDE.md)

---

## 🐛 Troubleshooting

### Демо не находит сцену
```bash
# Убедитесь что запускаете из корня проекта
cd /path/to/SpectraForge
./run_freqvox_sponza.sh
```

### Ошибка линковки GLFW
```bash
# Ubuntu/Debian
sudo apt install libglfw3-dev

# Arch Linux
sudo pacman -S glfw-x11

# Fedora
sudo dnf install glfw-devel
```

### Ошибка линковки GLM
```bash
# Ubuntu/Debian
sudo apt install libglm-dev

# Arch Linux
sudo pacman -S glm

# Fedora
sudo dnf install glm-devel
```

### CUDA backend недоступен
```bash
# Установите CUDA Toolkit
# https://developer.nvidia.com/cuda-downloads

# Пересоберите с CUDA
cmake .. -DBUILD_WITH_CUDA=ON
cmake --build .
```

---

## 🤝 Вклад

При добавлении новых примеров:

1. Создайте `.cpp` файл в `examples/`
2. Добавьте цель в корневой `CMakeLists.txt`
3. Обновите этот README
4. Добавьте документацию, если демо сложное
5. Следуйте [Coding Standards](../docs/CODING_STANDARDS.md)

---

**Версия:** 1.0.0  
**Обновлено:** 2025-10-02

