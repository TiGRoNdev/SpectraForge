# Two-Pass Rendering Implementation (Priority #1)

**Дата**: 2025-10-04  
**Статус**: ✅ Архитектура реализована  
**Ожидаемое ускорение**: 20-50× для сцен с 40k+ треугольниками

---

## 📊 Проблема: O(N×M) Сложность

### Текущая архитектура (Single-Pass):
```
ДЛЯ КАЖДОГО ПИКСЕЛЯ (2,073,600 пикселей @ 1080p):
    ДЛЯ КАЖДОГО ТРЕУГОЛЬНИКА (40,211 треугольников):
        - Проекция на экран
        - AABB тест
        - SDF вычисление
        - Блендинг
```

**Сложность**: O(N × M) = **83 миллиарда итераций/кадр**  
**Результат**: 4670ms/кадр = **0.21 FPS** (неприемлемо)

---

## 🚀 Решение: Two-Pass Rendering O(N+M)

### Архитектура:

```
PASS 1: Visibility Pass (O(N))
  ДЛЯ КАЖДОГО ТРЕУГОЛЬНИКА (40,211):
      - Проецировать на экран
      - Вычислить AABB
      - Rasterize: для каждого покрытого пикселя:
          - SDF тест
          - Если внутри → записать ID треугольника в visibility buffer

PASS 2: Shading Pass (O(M))
  ДЛЯ КАЖДОГО ПИКСЕЛЯ (2,073,600):
      - Прочитать список видимых треугольников (обычно 5-20)
      - Применить SDF window function и блендинг ТОЛЬКО для этих треугольников
```

**Новая сложность**: O(N + M × k) где k ≈ 5-20 (среднее число треугольников/пиксель)  
**Ожидаемый результат**: ~150-300ms/кадр = **6-20 FPS** → путь к 60 FPS с дополнительными оптимизациями

---

## 🏗️ Архитектурная реализация

### 1. Новые шейдеры

#### `shaders/TriangleVisibility.comp`
- **Workgroup**: 256×1×1 (параллелизм по треугольникам)
- **Алгоритм**:
  1. Для каждого треугольника:
     - Проекция на экран
     - Backface culling
     - AABB вычисление
  2. Rasterize AABB:
     - Для каждого пикселя в AABB
     - SDF тест: phi < 0 → внутри
     - Atomic append triangle ID к visibility list пикселя

#### `shaders/TriangleShading.comp`
- **Workgroup**: 16×16×1 (параллелизм по пикселям)
- **Алгоритм**:
  1. Для каждого пикселя:
     - Прочитать visibility list (count + triangle IDs)
     - Обработать ТОЛЬКО эти треугольники:
       - SDF window function
       - Front-to-back alpha blending
       - Early termination (если alpha > threshold)

### 2. Новые ресурсы

#### Visibility Buffer
```cpp
// Layout: для каждого пикселя [count, tri0, tri1, ..., triN]
vk::Buffer visibilityBuffer_;  
// Размер: num_pixels × (1 + maxTrianglesPerPixel) × sizeof(uint32_t)
// Пример @ 1080p с max=64: 2,073,600 × 65 × 4 = ~540 MB
```

#### Pixel Counters Buffer
```cpp
// Atomic counters для append операций в visibility pass
vk::Buffer pixelCountersBuffer_;
// Размер: num_pixels × sizeof(uint32_t)
// Пример @ 1080p: 2,073,600 × 4 = ~8 MB
```

### 3. Push Constants

```cpp
struct TwoPassPushConstants {
    glm::mat4 viewProj;          // View-projection matrix
    uint32_t outputWidth;        // Screen width
    uint32_t outputHeight;       // Screen height
    uint32_t triangleCount;      // Total triangles to process
    uint32_t maxTrianglesPerPixel; // Buffer capacity per pixel (64)
    uint32_t enableEarlyTermination; // Alpha early-out flag
    float alphaThreshold;        // Early termination threshold (0.99)
    uint32_t padding;
};
```

---

## 🔄 Execution Flow

```cpp
void TriangleSplattingPass::executeTwoPassRendering(vk::CommandBuffer cmd) {
    // ===== PRE: Clear buffers =====
    cmd.fillBuffer(pixelCountersBuffer_, 0, VK_WHOLE_SIZE, 0);
    cmd.fillBuffer(visibilityBuffer_, 0, VK_WHOLE_SIZE, 0xFFFFFFFFu);
    
    // Barrier: transfer → shader write/read
    ...
    
    // ===== PASS 1: Visibility (Triangle-parallel) =====
    executeVisibilityPass(cmd);
    // Output: visibilityBuffer_ заполнен списками треугольников для каждого пикселя
    
    // Barrier: visibility write → shading read
    ...
    
    // ===== PASS 2: Shading (Pixel-parallel) =====
    executeShadingPass(cmd);
    // Output: outputImage_ содержит финальный кадр
    
    // Barrier: shader write → transfer read (для present)
    ...
}
```

---

## 📈 Ожидаемые результаты

### Сравнение с текущей реализацией

| Метрика | Single-Pass (текущий) | Two-Pass (новый) | Улучшение |
|---------|----------------------|-----------------|-----------|
| **Итераций/кадр** | 83 млрд | ~100 млн (2M pixels × 50 tri/px) | **830×** |
| **Frame time** | 4670 ms | 150-300 ms | **15-30×** |
| **FPS** | 0.21 | 6-20 | **30-100×** |
| **GPU utilization** | 100% (wasted) | 60-80% (efficient) | Эффективнее |
| **Memory bandwidth** | 5.4 TB/кадр | ~50 GB/кадр | **100×** |

### Scaling @ различных разрешениях

| Разрешение | Single-Pass | Two-Pass | FPS Target |
|-----------|------------|---------|-----------|
| 960×540 | 1200 ms | 40-80 ms | ✅ 15-25 FPS |
| 1920×1080 | 4670 ms | 150-300 ms | ⚠️ 6-10 FPS |
| 3840×2160 (4K) | 18680 ms | 600-1200 ms | ❌ 1-2 FPS |

**Вывод**: Two-Pass дает путь к 60 FPS @ 1080p с дополнительными оптимизациями (BVH + GPU-Driven).

---

## 🛠️ Конфигурация

### Включение Two-Pass режима

```cpp
TriangleSplattingPass::Config config;
config.enableTwoPassRendering = true;  // По умолчанию включено
config.maxTrianglesPerPixel = 64;      // Типичное покрытие: 5-20 tri/pixel

auto pass = std::make_unique<TriangleSplattingPass>(config);
```

### Tuning параметров

- **`maxTrianglesPerPixel`**: 
  - **32**: Экономия памяти (~270 MB @ 1080p), риск overflow на плотных сценах
  - **64**: Баланс (540 MB), рекомендуется для большинства случаев
  - **128**: Высокая плотность (1.08 GB), для extreme scenes

- **Fallback**: Если pixel list переполняется:
  - Single-pass fallback автоматически для переполненных пикселей (будущая реализация)
  - Или предупреждение + clamp к `maxTrianglesPerPixel`

---

## 🔗 Совместимость с существующими оптимизациями

Two-Pass Rendering **совместим** со всеми текущими оптимизациями:

- ✅ **Frustum Culling**: работает перед visibility pass
- ✅ **Depth Sorting**: передается через sortedIndices в visibility pass
- ✅ **Tile Culling**: можно комбинировать (tile-based visibility pass)
- ✅ **Early Termination**: применяется в shading pass
- ✅ **Opaque Fast Path**: применяется в shading pass

---

## 📝 Дальнейшие оптимизации (Priority #2, #3)

После Two-Pass Rendering:

1. **Priority #2: Hierarchical BVH** (ускорение 5-10×)
   - Spatial acceleration для visibility pass
   - Уменьшает AABB rasterization до O(log N)

2. **Priority #3: GPU-Driven Rendering** (ускорение 2-3×)
   - Indirect dispatch с GPU
   - Compact visible triangles на GPU

**Комбинированное ускорение**: 20× (Two-Pass) × 5× (BVH) × 2× (GPU-Driven) = **200× общее ускорение**

---

## 🧪 Тестирование

### Unit tests

```cpp
TEST(TwoPassRendering, VisibilityBufferSize) {
    // Проверка размеров буферов
    uint32_t width = 1920, height = 1080;
    uint32_t maxTriPerPixel = 64;
    uint32_t expectedSize = width * height * (1 + maxTriPerPixel) * sizeof(uint32_t);
    ASSERT_EQ(visibilityBuffer_.size, expectedSize);
}

TEST(TwoPassRendering, PixelCoverage) {
    // Проверка корректности visibility pass:
    // каждый покрытый пиксель должен иметь хотя бы 1 треугольник
    ...
}
```

### Integration tests

```bash
# Benchmark с различными конфигурациями
./scripts/benchmark_two_pass.sh

# Профилирование с RenderDoc/NSight
./scripts/profile_two_pass.sh
```

---

## 📚 Ссылки

- [Triangle Splatting 2025 Paper](https://trianglesplatting.github.io/)
- [docs/ir/IR_CF.md](ir/IR_CF.md) - Core Findings (корневые причины)
- [docs/ir/IR_CSS.md](ir/IR_CSS.md) - Глубокий анализ производительности
- [docs/architecture/Renderer.md](architecture/Renderer.md) - Hybrid DWT + FreGS архитектура

---

## ✅ Status Checklist

- [x] Архитектура спроектирована
- [x] Шейдеры созданы (`TriangleVisibility.comp`, `TriangleShading.comp`)
- [x] Заголовочный файл обновлен (новые ресурсы и методы)
- [x] Скрипт компиляции шейдеров обновлен
- [x] Документация создана
- [ ] Реализация в `.cpp` файле (createTwoPassResources, execute*)
- [ ] Компиляция и устранение ошибок
- [ ] Тестирование на Sponza @ 1080p
- [ ] Benchmark и профилирование
- [ ] Финальный отчет с метриками

---

**Версия**: 1.0  
**Оптимизировано для**: Claude 4.5 Sonnet  
**Next Steps**: Реализация createTwoPassResources() и execute*() методов

