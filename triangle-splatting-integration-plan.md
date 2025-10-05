# Подробный план реализации Triangle Splatting Integration в SpectraForge

## Краткий обзор

**Статус проекта**: Triangle Splatting на 70% реализован в SpectraForge  
**Общая оценка времени**: 64 часа (8 рабочих дней)  
**Критические задачи**: 13 часов (1-2 дня)  

Анализ кодовой базы показал, что основная инфраструктура Triangle Splatting уже создана:
- ✅ Vulkan compute pipeline реализован 
- ✅ VMA память и буферы настроены
- ✅ Базовые шейдеры созданы (TriangleSplatting.comp, TriangleVisibility.comp, TriangleShading.comp)
- ✅ Two-Pass Rendering архитектура готова
- ⚠️ Математические алгоритмы SDF требуют доработки
- ⚠️ Интеграция с основным рендерером неполная

## Детальный план по фазам

### 📋 ФАЗА 1: Завершение шейдерных алгоритмов ⭐ КРИТИЧНО
**Приоритет**: КРИТИЧНО | **Время**: 13 часов

#### 1.1. Исправить математическую реализацию SDF (4 часа)
**Файл**: `shaders/TriangleSplatting.comp`

**Проблема**: Текущая функция `halfPlaneDistance()` содержит ошибки для screen space координат с Y-down ориентацией.

**Математическая основа**: Согласно [trianglesplatting.github.io](https://trianglesplatting.github.io), SDF должна вычисляться как:
```glsl
φ(p) = max(L₁(p), L₂(p), L₃(p))
```
где `Lᵢ(p) = nᵢ·p + dᵢ` - расстояние до полуплоскости.

**Решение**:
```glsl
float halfPlaneDistance(vec2 p, vec2 v0, vec2 v1) {
    vec2 edge = v1 - v0;
    // ИСПРАВЛЕНИЕ: Правильные нормали для Y-down screen space
    vec2 normal = vec2(edge.y, -edge.x); // Right perpendicular
    float edgeLength = length(edge);
    if (edgeLength < 0.0001) return 10000.0;
    normal = normal / edgeLength; // Unit normal
    return dot(normal, p - v0);
}
```

#### 1.2. Реализовать smooth window function I(p) (3 часа)
**Файл**: `shaders/TriangleSplatting.comp`

**Математическая основа**: 
```glsl
I(p) = ReLU(φ(p)/φ(s))^σ = max(0, φ(p)/φ(s))^σ
```
где `s` - incenter треугольника, `φ(s) < 0` - минимальное SDF значение.

**Текущий статус**: Функция `smoothWindowFunction()` частично реализована.

**Решение**: Обеспечить корректный расчет:
- `φ(s) < 0` для incenter 
- `φ(p) ≤ 0` внутри треугольника
- `I(p) = 0` на границе и снаружи

#### 1.3. Завершить Two-Pass Rendering шейдеры (6 часов)
**Файлы**: `shaders/TriangleVisibility.comp`, `shaders/TriangleShading.comp`

**Проблема**: Шейдеры созданы но требуют отладки AABB clipping и SDF computation.

**Решение**:
- Исправить conservative rasterization в Visibility pass
- Оптимизировать per-pixel triangle lists в Shading pass
- Добавить bounds checking для visibility buffer overflow

### 📋 ФАЗА 2: Интеграция с существующим рендером
**Приоритет**: ВЫСОКИЙ | **Время**: 9 часов

#### 2.1. Подключить Triangle Splatting к HybridFreGSRenderer (4 часа)
**Файл**: `src/rendering/HybridFreGSRenderer.cpp`

**Решение**: Добавить conditional branch в `renderFrame()`:
```cpp
if (renderMode == RenderMode::TriangleSplatting) {
    triangleSplattingPass_->execute(cmd, frameIndex);
} else {
    fregsPass_->execute(cmd, frameIndex);
}
```

#### 2.2. Реализовать Triangle data upload (3 часа)
**Файлы**: `src/rendering/TriangleSplattingPass.cpp`, `include/SpectraForge/rendering/TriangleSplattingPass.h`

**Решение**: Добавить utility функции для конвертации mesh data:
```cpp
std::vector<Triangle> convertMeshToTriangles(const Mesh& mesh, float sigma);
void uploadTriangles(const std::vector<Triangle>& triangles);
```

#### 2.3. Настроить descriptor bindings (2 часа)
**Файл**: `src/rendering/TriangleSplattingPass.cpp`

**Решение**: Расширить binding layout:
- Binding 4: MaterialTextures buffer
- Binding 5: TextureData buffer

### 📋 ФАЗА 3: Оптимизация и производительность  
**Приоритет**: СРЕДНИЙ | **Время**: 18 часов

#### 3.1. Реализовать Tile-based culling (8 часов)
**Файл**: `shaders/TileCulling.comp`

**Решение**: Conservative rasterization треугольников в 16×16 тайлы для пространственной оптимизации.

#### 3.2. Оптимизировать Bitonic Sort (6 часов)
**Файлы**: `shaders/BitonicSort.comp`, `src/rendering/TriangleSplattingPass.cpp`

**Решение**: Исправить depth sorting алгоритм и power-of-2 padding.

#### 3.3. Реализовать Frustum Culling (4 часа)
**Файл**: `shaders/FrustumCulling.comp`

**Решение**: GPU-based view-frustum intersection tests.

### 📋 ФАЗА 4: Тестирование и валидация
**Приоритет**: ВЫСОКИЙ | **Время**: 10 часов

#### 4.1. Создать unit тесты (4 часа)
**Файл**: `tests/triangle_splatting_test.cpp`

**Решение**: Тестировать корректность φ(p) и I(p) функций.

#### 4.2. Визуальный debug mode (3 часа)
**Файл**: `shaders/TriangleSplatting.comp`

**Решение**: Debug push constants для визуализации SDF values.

#### 4.3. Performance benchmarking (3 часа)
**Файл**: `src/performance/TriangleSplattingBenchmark.cpp`

**Решение**: Сравнение производительности с FreGS режимом.

### 📋 ФАЗА 5: Материалы и текстуры
**Приоритет**: СРЕДНИЙ | **Время**: 14 часов

#### 5.1. Реализовать texture sampling (6 часов)
**Файл**: `shaders/TriangleSplatting.comp`

**Решение**: UV mapping и texture atlas lookup вместо fallback цветов.

#### 5.2. Добавить lighting model (8 часов) 
**Файл**: `shaders/TriangleShading.comp`

**Решение**: Phong/PBR shading с normal vectors.

## Математические основы для реализации

### SDF Computation
Согласно Triangle Splatting paper:
```glsl
φ(p) = max(L₁(p), L₂(p), L₃(p))
```
где `Lᵢ(p)` - расстояние до i-й полуплоскости треугольника.

### Smooth Window Function  
```glsl
I(p) = ReLU(φ(p)/φ(s))^σ
```
- `φ(s)` - SDF в incenter (минимальное значение)
- `σ` - smoothness parameter
- `ReLU(x) = max(0, x)`

### Two-Pass Rendering
1. **Visibility Pass O(N)**: Определить видимые треугольники для каждого пикселя
2. **Shading Pass O(M)**: Применить SDF и blending только к видимым

Ожидаемое ускорение: **20-50× для сцен с высокой triangle density**.

## Критический путь (1-2 дня)

Для получения работающего Triangle Splatting в минимальные сроки:

1. **День 1**: Фаза 1 (13 часов) - исправить математику SDF
2. **День 2**: Фаза 2.1-2.2 (7 часов) - интегрировать с рендерером

После этого базовый Triangle Splatting будет функционировать, остальные фазы можно реализовывать итеративно.

## Заключение

Triangle Splatting integration в SpectraForge близка к завершению. Основная работа сосредоточена на исправлении математических алгоритмов в шейдерах и интеграции с существующим render pipeline. При фокусе на критических задачах работающая реализация может быть получена за 1-2 дня.