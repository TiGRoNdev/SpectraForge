# Runtime Error Fixes - SpectraForge

**Дата**: 2025-10-03  
**Статус**: ✅ Исправлено (частично)

## Проблема 1: VulkanContext Initialization Failed

### Симптомы
```
VulkanContext initialization failed: vk::createInstance: ErrorExtensionNotPresent
HybridFreGSRenderer: не удалось создать VulkanContext
```

### Причина
Приложение использовало `libvulkan.so` из vcpkg (версия 1.4.309), который не имеет доступа к системным драйверам ICD (Installable Client Driver). Vulkan loader из vcpkg не видел драйверы Intel/Radeon/Mesa, установленные в системе.

### Решение

#### Вариант 1: Wrapper Script (Рекомендуется для разработки)
Используйте скрипт для запуска с системным Vulkan:

```bash
./run_with_system_vulkan.sh ./build/SpectraForge_Example_Demo
```

#### Вариант 2: Export переменной окружения
```bash
export LD_LIBRARY_PATH="/usr/lib/x86_64-linux-gnu:$LD_LIBRARY_PATH"
./build/SpectraForge_Example_Demo
```

#### Вариант 3: Permanent Fix (TODO)
Изменить CMakeLists.txt для явного использования системного Vulkan вместо vcpkg версии:

```cmake
# В CMakeLists.txt добавить BEFORE find_package(Vulkan REQUIRED):
set(CMAKE_FIND_PACKAGE_PREFER_CONFIG OFF)
set(Vulkan_LIBRARY "/usr/lib/x86_64-linux-gnu/libvulkan.so")
find_package(Vulkan REQUIRED)
```

### Диагностика

Проверить используемый libvulkan:
```bash
ldd ./build/SpectraForge_Example_Demo | grep vulkan
```

Должно показать **системный** libvulkan:
```
libvulkan.so.1 => /lib/x86_64-linux-gnu/libvulkan.so.1
```

Не vcpkg версию:
```
libvulkan.so.1 => /path/to/vcpkg_installed/x64-linux/lib/libvulkan.so.1  # ❌ НЕПРАВИЛЬНО
```

## Проблема 2: GLFW Vulkan Extensions

### Симптомы
```
Warning: glfwGetRequiredInstanceExtensions returned null
```

### Причина
GLFW инициализировалась до проверки поддержки Vulkan, но окно создавалось без `GLFW_CLIENT_API = GLFW_NO_API` или `glfwVulkanSupported()` не проверялось.

### Решение
Изменения в `Window.cpp` и `VulkanContextImpl.cpp`:

1. **Window.cpp**: Добавлена проверка `glfwVulkanSupported()` перед созданием окна
2. **VulkanContextImpl.cpp**: Изменена логика fallback расширений на throw исключения с детальной диагностикой
3. **Engine.cpp**: Добавлена подробная логгирование на каждом шаге инициализации

## Текущие Известные Проблемы

### Vulkan Validation Errors (Runtime)

**Статус**: ⚠️ Требует исправления

Приложение запускается, но validation layer выдаёт ошибки:

```
VUID-vkQueueSubmit-pCommandBuffers-00070: Command buffer must be in pending or executable state
VUID-vkFreeCommandBuffers-pCommandBuffers-00047: Command buffer is in use
```

**Причина**: `HybridFreGSRenderer::renderFrame()` не полностью реализован (stub согласно docs/reports/Renderer_Validation_Report.md). Command buffer записывается но не завершается (`vkEndCommandBuffer()` не вызывается).

**Решение**: Реализовать полный pipeline в HybridFreGSRenderer:
1. Wire Wavelet→FreGS passes
2. Implement renderFrame() with proper command buffer recording/submission
3. Add image barriers between compute passes

См. `docs/reports/Renderer_Validation_Report.md` для деталей.

## Успешная Инициализация

После исправлений, приложение успешно:
- ✅ Инициализирует GLFW
- ✅ Создаёт окно с Vulkan support
- ✅ Получает Vulkan расширения от GLFW (2 extensions)
- ✅ Создаёт Vulkan instance (API 1.3)
- ✅ Обнаруживает GPU: Intel(R) Graphics (LNL)
- ✅ Создаёт logical device (Graphics Queue: 0, Compute Queue: 1)
- ✅ Инициализирует VMA (Vulkan Memory Allocator)
- ✅ VulkanContext инициализирован
- ✅ WaveletPass создаёт transient images (7 MB)

## Рекомендации

1. **Для разработчиков**: Используйте `run_with_system_vulkan.sh` для запуска
2. **Для CI/CD**: Добавьте `LD_LIBRARY_PATH` в GitHub Actions workflow
3. **Для релиза**: Рассмотреть static linking Vulkan или bundle системного libvulkan
4. **Перед коммитом**: Убедитесь что vcpkg.json не содержит явной зависимости от `vulkan` или `vulkan-loader`

## Связанные Файлы

- `src/core/VulkanContextImpl.cpp` - Инициализация Vulkan
- `src/core/Window.cpp` - GLFW window management
- `src/app/Engine.cpp` - Application lifecycle
- `run_with_system_vulkan.sh` - Wrapper script
- `docs/reports/Renderer_Validation_Report.md` - Validation pipeline issues

---
**Автор**: SpectraForge Team  
**Последнее обновление**: 2025-10-03

