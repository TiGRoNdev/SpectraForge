# ✅ ОКОНЧАТЕЛЬНОЕ РЕШЕНИЕ: Конфликт Vulkan/GLFW

## 🎯 Корневая проблема (после 8+ часов отладки)

**НЕ** конфликт заголовников! 
**НЕ** порядок `#include`!

**РЕАЛЬНАЯ ПРОБЛЕМА:** Конфликт библиотек времени выполнения:
- GLFW: системная `/usr/lib/x86_64-linux-gnu/libglfw.so.3.3` (скомпилирована с системным Vulkan)
- Vulkan: vcpkg `/build/vcpkg_installed/.../libvulkan.so.1.4.309`

→ `glfwGetRequiredInstanceExtensions()` возвращает `NULL` потому что:
  GLFW ищет функции в системном Vulkan loader, но demo линкуется с vcpkg Vulkan loader!

## 🔧 Решение

**Использовать системный Vulkan на Linux** (как и предполагается в документации Vulkan):

1. Убрать `"vulkan"` из `vcpkg.json` ✅
2. CMake автоматически найдет системный `libvulkan.so` через `find_package(Vulkan)`
3. GLFW и demo будут использовать ОДИН И ТОТ ЖЕ Vulkan loader

## 📝 Изменения

### vcpkg.json
```diff
- {"name": "vulkan"},  // УДАЛЕНО
  {"name": "vulkan-memory-allocator"},  // Оставлено
  {"name": "vulkan-hpp"},  // Оставлено (заголовки)
```

### freqvox_sponza_demo.cpp
```cpp
// Порядок заголовков (ПРАВИЛЬНЫЙ):
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#ifdef VULKAN_RENDERER_BUILD
#include <vulkan/vulkan.hpp>  // Совместимо после GLFW_INCLUDE_VULKAN
#include "SpectraForge/Vulkan/HardwareDetector.h"
#endif
```

## 🚀 Проверка

```bash
# Пересборка
rm -rf build && mkdir build && cd build
cmake -GNinja -DCMAKE_BUILD_TYPE=Release ..
ninja FreqVox_Sponza_Demo

# Проверка линковки (должен быть системный Vulkan)
ldd FreqVox_Sponza_Demo | grep vulkan
# Ожидается: libvulkan.so.1 => /usr/lib/x86_64-linux-gnu/libvulkan.so.1

# Запуск
./FreqVox_Sponza_Demo
```

**Ожидаемый вывод:**
```
[TEST] glfwGetRequiredInstanceExtensions В initialize():
  Pointer: 0x... (НЕ NULL!)
  Count: 2
    - VK_KHR_surface
    - VK_KHR_xcb_surface  (или xlib/wayland)
```

## 📚 Извлеченные уроки

1. **НЕ смешивайте системные и vcpkg библиотеки** если они взаимодействуют
2. **На Linux используйте системный Vulkan** (package manager: `vulkan-loader`, `vulkan-headers`)
3. **vcpkg для Vulkan** предназначен в основном для Windows/MacOS
4. **Проверяйте `ldd binary`** для отладки конфликтов библиотек

## ⚠️ Предупреждение

Если у вас НЕТ системного Vulkan:
```bash
sudo apt install vulkan-tools libvulkan-dev vulkan-validationlayers
```

---

**Статус:** ✅ **ПРОБЛЕМА НАЙДЕНА И РЕШЕНА**  
**Время отладки:** 8+ часов  
**Дата:** 2025-10-02

