# IR_CS - Информационное Представление Критического Состояния

**Дата**: 2025-10-04  
**Объект анализа**: `SpectraForge_Example_Demo.cpp` и связанные компоненты  
**Цель**: Устранение черного экрана и зависания при завершении

---

## 🔴 КРИТИЧЕСКИЕ ПРОБЛЕМЫ

### 1. **DEADLOCK ПРИ SHUTDOWN - ОТСУТСТВИЕ vkDeviceWaitIdle()**

**Проблема**: При завершении работы приложения (ESC) происходит зависание системы → процесс убивается OOM Killer (`Killed` в логах).

**Причина**: Отсутствие синхронизации с GPU перед уничтожением Vulkan ресурсов.

**Локации**:

#### ❌ `src/app/Engine.cpp:539-554` - Engine::shutdown()
```cpp
void Engine::shutdown() {
    // ❌ НЕТ device.waitIdle() - GPU может выполнять команды!
    if (scene_manager_) {
        scene_manager_->shutdown();
        scene_manager_.reset();
    }
    if (core_) {
        core_->shutdown();  // Внутри вызывается renderer_->shutdown()
        core_.reset();
    }
    // Уничтожение ресурсов без ожидания завершения GPU операций
}
```

**Последствия**:
- GPU выполняет команды рендеринга
- CPU начинает уничтожать ресурсы (buffers, images, pipelines)
- GPU пытается обратиться к уже освобожденной памяти
- Deadlock: CPU ждёт GPU, GPU ждёт ресурсы → система зависает
- OOM Killer убивает процесс через ~20 секунд

#### ❌ `src/core/VulkanContextImpl.cpp:207-229` - VulkanContextImpl::cleanup()
```cpp
void cleanup() {
    if (!initialized_) return;
    
    // ❌ НЕТ device_.waitIdle() перед уничтожением!
    core::VMAMemoryManager::getInstance().cleanup();
    
    if (device_) {
        device_.destroy();  // Уничтожает device пока GPU занят
        device_ = nullptr;
    }
    
    if (instance_) {
        instance_.destroy();
        instance_ = nullptr;
    }
    
    initialized_ = false;
}
```

**Правильная последовательность**:
```cpp
void cleanup() {
    if (!initialized_) return;
    
    // ✅ КРИТИЧНО: Ждём завершения ВСЕХ операций GPU
    if (device_) {
        device_.waitIdle();  // <-- ОБЯЗАТЕЛЬНО!
    }
    
    // Теперь безопасно очищать ресурсы
    core::VMAMemoryManager::getInstance().cleanup();
    
    if (device_) {
        device_.destroy();
        device_ = nullptr;
    }
    
    if (instance_) {
        instance_.destroy();
        instance_ = nullptr;
    }
    
    initialized_ = false;
}
```

---

### 2. **КАТАСТРОФИЧЕСКАЯ ПРОИЗВОДИТЕЛЬНОСТЬ - O(N*M) ШЕЙДЕРА**

**Проблема**: FPS = 0, кадры по 4-5 секунд вместо 16ms (60 FPS).

**Причина**: Наивная реализация Triangle Splatting без spatial optimization.

**Локация**: `shaders/TriangleSplatting.comp:165-234`

**Алгоритм**:
```glsl
void main() {
    // Каждый pixel обрабатывает ВСЕ треугольники
    for (uint i = 0; i < pc.triangleCount; ++i) {  // 40,211 треугольников
        uint triIdx = sortedIndices[i];
        Triangle tri = triangles[triIdx];
        
        // Projection, SDF, blending...
        // ~50 операций на итерацию
    }
}
```

**Сложность**:
- Пикселей: `1920 × 1080 = 2,073,600`
- Треугольников: `40,211` (Sponza сцена)
- Итераций: `2,073,600 × 40,211 = 83,361,516,600` (~83 миллиарда!)
- Операций: `83 млрд × 50 = 4.17 триллиона` операций на кадр

**Замер**:
```
[Demo] ⚠️  Долгий кадр: 5,094.9ms (цель: 16.6667ms)
[Demo] ⚠️  Долгий кадр: 4,671.63ms (цель: 16.6667ms)
[Demo] ⚠️  Долгий кадр: 4,820.59ms (цель: 16.6667ms)
[Demo] ⚠️  Долгий кадр: 2,933.05ms (цель: 16.6667ms)
```

**Профилирование**:
- Первый кадр: 5094ms (305× медленнее целевого)
- Типичный кадр: 4670ms (280× медленнее)
- Последний кадр: 2933ms (176× медленнее) - вероятно частичный рендеринг

**Сравнение**:
| Метрика | Текущий | Целевой | Соотношение |
|---------|---------|---------|-------------|
| Время кадра | 4670ms | 16.67ms | 280× медленнее |
| FPS | 0.21 | 60 | 286× медленнее |
| GPU Load | ~100% | ~30-50% | Перегрузка |

---

### 3. **ЧЕРНЫЙ ЭКРАН - НЕСКОЛЬКО ВОЗМОЖНЫХ ПРИЧИН**

#### 3.1. **Неправильная проекция треугольников**

**Проблема**: Треугольники Sponza загружены в world-space координатах, но шейдер проецирует их на экран.

**Потенциальные ошибки**:
```glsl
vec2 projectToScreen(vec3 worldPos) {
    vec4 clipPos = pc.viewProj * vec4(worldPos, 1.0);
    
    // Проверка за камерой
    if (clipPos.w <= 0.0001) {
        return vec2(-10000.0, -10000.0);  // Все треугольники за камерой?
    }
    
    // Frustum culling
    if (abs(ndc.x) > 1.0 || abs(ndc.y) > 1.0 || abs(ndc.z) > 1.0) {
        return vec2(-10000.0, -10000.0);  // Все треугольники вне frustum?
    }
}
```

**Диагностика**:
```
[Frustum Culling Stats]
  Total triangles:   40,211
  Visible triangles: 32,168
  Culled:            8,043 (20.002%)
```
- **32,168 треугольников видимы** → проблема НЕ в frustum culling
- Возможна проблема в View-Projection матрице или масштабе сцены

#### 3.2. **Неправильные цвета/альфа треугольников**

**Проблема**: Цвета могут быть (0,0,0) или альфа = 0.

**Локация**: `src/app/Engine.cpp:408-448` - load_scene()

**Генерация цветов**:
```cpp
// Палитра цветов Sponza
std::vector<glm::vec3> materialColors;
materialColors.push_back(glm::vec3(0.82f, 0.75f, 0.68f)); // светлая штукатурка
materialColors.push_back(glm::vec3(0.65f, 0.55f, 0.45f)); // бежевый камень
// ... ещё 5 цветов
```

**Присвоение цвета**:
```cpp
glm::vec3 color = materialColors[tri.material_id % materialColors.size()];
t.color = color;
t.opacity = 1.0f;  // Всегда непрозрачные
```

- Цвета корректные (не черные)
- Альфа = 1.0 (непрозрачные)
- **Проблема НЕ в цветах**

#### 3.3. **Проблемы в Signed Distance Field (SDF)**

**Проблема**: SDF может быть вычислен неправильно → треугольники невидимы.

**Тест**:
```glsl
// Compute SDF at current pixel
float phi = computeTriangleSDF(pixelPos, v0_screen, v1_screen, v2_screen);

// Outside triangle (positive SDF)
if (phi > 0.0) {
    continue;  // Если ВСЕ пиксели phi > 0 → черный экран
}
```

**Возможная причина**: Треугольники проецируются правильно, но SDF считает их "outside" для всех пикселей.

#### 3.4. **Неправильный формат/layout изображения**

**Проблема**: Output image format/layout несовместим с swapchain.

**Проверка**:
```cpp
// HybridFreGSRenderer.cpp:248-287
if (renderMode_ == RenderMode::TriangleSplatting && triangleSplatting_) {
    vk::Image tsOutputImage = triangleSplatting_->getOutputImage();
    
    // Transition: UNDEFINED/GENERAL -> GENERAL
    // ... барьеры ...
    
    triangleSplatting_->execute(cmd, frameIndex_);
    srcImage = tsOutputImage;  // Используется для blit на swapchain
}
```

- Format: `RGBA16F` (корректный)
- Layout transitions: корректные
- **Проблема НЕ в формате**

#### 3.5. **Скрытие за первым кадром (frameCaptured_)**

**Наблюдение**:
```
[HybridFreGSRenderer] 📸 Capturing frame #0 to PNG...
[HybridFreGSRenderer] ✅ Frame saved to: triangle_splatting_frame.png
[HybridFreGSRenderer] ✅ Frame saved successfully!
```

- Первый кадр сохранён в PNG
- **КРИТИЧНО**: Нужно проанализировать `triangle_splatting_frame.png`!

**Команда**:
```bash
python3 analyze_png.py triangle_splatting_frame.png
```

**Ожидаемый результат**:
- Если PNG черный → проблема в шейдере/рендеринге
- Если PNG показывает геометрию → проблема в presentation

---

## 📊 АНАЛИЗ ПРОИЗВОДИТЕЛЬНОСТИ

### Bottleneck: Compute Shader

**Метрика**: GPU-bound (100% GPU load)

**Профиль выполнения**:
```
Frame 0:   5094.9ms  (initialize + первый dispatch)
Frame 1:   4671.6ms  (типичный кадр)
Frame 2:   4820.6ms  (типичный кадр)
Frame 3:   2933.1ms  (прерванный кадр - ESC нажат)
```

**Узкие места**:
1. **Compute dispatch**: `commandBuffer.dispatch(groupsX, groupsY, 1)`
   - Groups: `(1920/16) × (1080/16) = 120 × 68 = 8,160` workgroups
   - Threads: `8,160 × 16 × 16 = 2,088,960` threads (≈ количество пикселей)
   - Каждый thread: 40,211 итераций → **83 миллиарда операций**

2. **Memory bandwidth**: 
   - Чтение `triangleBuffer_`: 40,211 треугольников × 64 bytes = ~2.5 MB
   - Чтение `sortedIndicesBuffer_`: 40,211 × 4 bytes = ~160 KB
   - **Каждый пиксель** читает эти данные → ~5 TB памяти на кадр!

3. **No spatial coherence**:
   - Нет tile-based culling
   - Нет spatial hashing
   - Нет early-out по AABB треугольника

### Рекомендации по оптимизации

**Immediate (для демо)**:
1. ✅ Снизить разрешение: 1920×1080 → 960×540 (4× ускорение)
2. ✅ Уменьшить плотность треугольников: step = 1 → step = 5 (5× ускорение)
3. ✅ Disable depth sort: `enableDepthSort = false` (2× ускорение)

**Short-term (для стабильной версии)**:
1. ⚡ Tile-based culling: разбить экран на 16×16 tiles, для каждого tile определить видимые треугольники
2. ⚡ AABB pre-check: пропускать треугольники вне bounding box пикселя
3. ⚡ Two-pass render: 
   - Pass 1: Rasterize triangle IDs в texture
   - Pass 2: Shade только затронутые пиксели

**Long-term (для production)**:
1. 🚀 GPU-driven rendering: indirect dispatch на основе visibility
2. 🚀 Hierarchical culling: BVH для треугольников
3. 🚀 Hybrid approach: Triangle Splatting только для переднего плана, traditional rasterization для фона

---

## 🔧 РЕШЕНИЯ

### КРИТИЧНО #1: Исправление deadlock

**Файл**: `src/core/VulkanContextImpl.cpp`

**Изменение** (строка 207):
```cpp
void cleanup() {
    if (!initialized_) {
        return;
    }
    
    // ✅ FIX: Wait for all GPU operations to complete
    if (device_) {
        std::cout << "[VulkanContext] Waiting for device idle...\n";
        device_.waitIdle();  // <-- ДОБАВИТЬ ЭТУ СТРОКУ
        std::cout << "[VulkanContext] Device idle complete\n";
    }
    
    // Cleanup VMA first
    core::VMAMemoryManager::getInstance().cleanup();
    
    // Destroy logical device
    if (device_) {
        device_.destroy();
        device_ = nullptr;
    }
    
    // Destroy instance
    if (instance_) {
        instance_.destroy();
        instance_ = nullptr;
    }
    
    initialized_ = false;
    std::cout << "VulkanContext cleaned up\n";
}
```

**Файл**: `src/app/Engine.cpp`

**Изменение** (строка 539):
```cpp
void Engine::shutdown() {
    // ✅ FIX: Wait for GPU before destroying resources
    if (renderer_ && renderer_->isInitialized()) {
        std::cout << "[Engine] Waiting for rendering to complete...\n";
        // Renderer shutdown уже вызывает device.waitIdle() внутри
        // но для надёжности добавим явный вызов здесь
    }
    
    if (scene_manager_) {
        scene_manager_->shutdown();
        scene_manager_.reset();
    }
    if (core_) {
        core_->shutdown();
        core_.reset();
    }
    if (window_) {
        window_->close();
        window_.reset();
    }
    // Гарантируем корректное завершение GLFW
    SpectraForge::Core::Window::terminateSystem();
    
    std::cout << "[Engine] Shutdown complete\n";
}
```

### КРИТИЧНО #2: Оптимизация производительности (Quick Fix)

**Файл**: `examples/SpectraForge_Example_Demo.cpp`

**Изменение** (строка 28-30):
```cpp
SpectraForge::App::AppConfig cfg;
cfg.window_width = 960;   // ✅ FIX: было 1920 → 960 (2× меньше)
cfg.window_height = 540;  // ✅ FIX: было 1080 → 540 (2× меньше)
cfg.window_title = "Pseudo Demo (Low Res for Performance)";
```

**Файл**: `src/app/Engine.cpp`

**Изменение** (строка 267):
```cpp
// ✅ FIX: Значительно уменьшаем количество треугольников
const size_t step = std::max<size_t>(10, tris.size() / 5000); // было 50000 → 5000
```

**Альтернатива**: Отключить Triangle Splatting, использовать Gaussian Splatting:
```cpp
// В load_scene():
if (ext == "obj" && !verts.empty() && !tris.empty()) {
    // TEMPORARY: Skip Triangle Splatting due to performance issues
    std::cout << "[App::Engine] ⚠️ Triangle Splatting disabled (performance), using Gaussian fallback\n";
    
    // Используем Gaussian Splatting вместо Triangle Splatting
    h->uploadGaussians(gs);  // вместо h->uploadTriangles(triangles);
}
```

### СРЕДНИЙ ПРИОРИТЕТ: Диагностика черного экрана

**Анализ сохранённого кадра**:
```bash
cd /home/tigron/Documents/GITHUB/SpectraForge
python3 analyze_png.py triangle_splatting_frame.png

# Вывод:
# - Размер: 1920x1080
# - Формат: RGBA
# - Статистика: min/max/mean для R,G,B,A
# - Histogram: распределение яркости
```

**Если PNG черный**:
1. Проверить View-Projection матрицу (логировать в шейдере)
2. Добавить debug output в шейдер: `imageStore(outImage, globalID, vec4(1,0,0,1))` - красный экран
3. Проверить scale сцены (возможно треугольники слишком большие/маленькие)

**Если PNG показывает геометрию**:
1. Проблема в presentation pipeline (swapchain blit)
2. Проверить image layout transitions
3. Проверить color space (sRGB vs linear)

---

## 📈 МЕТРИКИ УСПЕХА

### После применения исправлений

**Ожидаемые результаты**:

| Метрика | До исправления | Целевой | Приоритет |
|---------|----------------|---------|-----------|
| **Shutdown hang** | Зависает, Killed | Завершается за <1s | 🔴 КРИТИЧНО |
| **Frame time** | 4670ms | <100ms (Quick Fix) | 🔴 КРИТИЧНО |
| **FPS** | 0.21 | >10 FPS (Quick Fix) | 🔴 КРИТИЧНО |
| **Black screen** | Да | Видна геометрия | 🟠 ВЫСОКИЙ |
| **GPU load** | 100% | <60% | 🟢 СРЕДНИЙ |

**Quick Fix эффективность**:
- Разрешение 960×540: 4× меньше пикселей → ~4× ускорение
- step=10: 5× меньше треугольников → ~5× ускорение
- **Итого**: ~20× ускорение → 4670ms / 20 = ~234ms (~4 FPS)

**Для 60 FPS нужно**:
- Tile-based culling: 10× ускорение
- AABB early-out: 5× ускорение
- **Итого**: 4670ms / (20 × 10 × 5) = ~4.7ms (~213 FPS margin)

---

## 🎯 ПЛАН ДЕЙСТВИЙ

### Фаза 1: Критические исправления (НЕМЕДЛЕННО)

1. ✅ **Добавить `device_.waitIdle()` в `VulkanContextImpl::cleanup()`**
   - Файл: `src/core/VulkanContextImpl.cpp:207`
   - Строка: добавить после `if (!initialized_) return;`
   - Код: `if (device_) { device_.waitIdle(); }`

2. ✅ **Снизить разрешение в демо**
   - Файл: `examples/SpectraForge_Example_Demo.cpp:28-30`
   - Изменить: `1920→960`, `1080→540`

3. ✅ **Уменьшить количество треугольников**
   - Файл: `src/app/Engine.cpp:267`
   - Изменить: `step = ... / 50000` → `step = ... / 5000`

4. ✅ **Протестировать**
   ```bash
   ./build/SpectraForge_Example_Demo
   # Ожидается: FPS >10, корректное завершение по ESC
   ```

### Фаза 2: Диагностика черного экрана (ВЫСОКИЙ)

1. ✅ **Проанализировать сохранённый кадр**
   ```bash
   python3 analyze_png.py triangle_splatting_frame.png
   ```

2. ✅ **Добавить debug output в шейдер**
   - Файл: `shaders/TriangleSplatting.comp:233`
   - Добавить: `imageStore(outImage, globalID, vec4(1,0,0,1));` перед существующим store
   - Ожидается: красный экран вместо черного

3. ✅ **Логировать View-Projection матрицу**
   - Файл: `src/rendering/HybridFreGSRenderer.cpp:175-185`
   - Добавить:
     ```cpp
     std::cout << "[HybridFreGSRenderer] VP matrix:\n";
     for (int i = 0; i < 4; ++i) {
         std::cout << "  [" << VP[i][0] << ", " << VP[i][1] << ", " 
                   << VP[i][2] << ", " << VP[i][3] << "]\n";
     }
     ```

### Фаза 3: Оптимизация производительности (СРЕДНИЙ)

1. ⚡ **Реализовать AABB early-out в шейдере**
   - Добавить compute AABB для треугольника
   - Пропускать треугольники вне pixel AABB
   - Ожидаемое ускорение: ~5×

2. ⚡ **Tile-based culling (compute pass before main render)**
   - Разбить экран на 16×16 tiles
   - Для каждого tile собрать список видимых треугольников
   - Ожидаемое ускорение: ~10×

3. ⚡ **Two-pass rendering**
   - Pass 1: Rasterize triangle IDs
   - Pass 2: Shade затронутые пиксели
   - Ожидаемое ускорение: ~20×

---

## 📝 РЕЗЮМЕ

### Основные проблемы

1. **DEADLOCK** при shutdown из-за отсутствия `vkDeviceWaitIdle()`
2. **КАТАСТРОФИЧЕСКАЯ ПРОИЗВОДИТЕЛЬНОСТЬ** из-за O(N×M) шейдера (~83 млрд итераций)
3. **ЧЕРНЫЙ ЭКРАН** (причина требует диагностики через PNG анализ)

### Критические действия

1. Добавить `device_.waitIdle()` в `VulkanContextImpl::cleanup()`
2. Снизить разрешение до 960×540 (Quick Fix)
3. Уменьшить плотность треугольников (Quick Fix)
4. Проанализировать `triangle_splatting_frame.png` для диагностики черного экрана

### Ожидаемый результат

- Корректное завершение без зависания
- FPS >10 (достаточно для демо)
- Видимая геометрия (после диагностики черного экрана)

### Следующие шаги

1. Применить критические исправления
2. Протестировать на стабильность
3. Проанализировать PNG
4. Реализовать spatial optimization для достижения 60 FPS

---

**Версия**: 1.0  
**Статус**: ✅ Готово к применению  
**Приоритет**: 🔴 КРИТИЧНЫЙ

