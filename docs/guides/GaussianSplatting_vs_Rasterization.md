# Gaussian Splatting vs Traditional Rasterization

**Дата**: 2025-10-03  
**Цель**: Объяснить разницу между Gaussian Splatting и традиционным растеризатором для понимания выбора технологии

---

## 🎯 Основные отличия

| Аспект | Gaussian Splatting | Traditional Rasterization |
|--------|-------------------|--------------------------|
| **Входные данные** | Point clouds, 3D гауссианы | Треугольники (полигоны) |
| **Представление** | Центр + размер (σ) + цвет | Вершины + текстурные координаты |
| **Пайплайн** | Compute shader (projection + blend) | Vertex → Rasterizer → Fragment |
| **Текстуры** | Сложно (нужно запекать в цвет) | Естественно (UV mapping) |
| **Освещение** | Пре-рендер или аппроксимация | Real-time (Phong, PBR) |
| **Подходит для** | NeRF, фотограмметрия, particle effects | Игры, CAD, традиционные 3D сцены |
| **Производительность** | O(N×pixels), зависит от количества гауссианов | O(triangles), GPU-оптимизировано |

---

## 🔬 Gaussian Splatting: Как работает

### Математическая модель
Каждый гауссиан представлен как **3D Gaussian distribution**:

```glsl
G(x, y, z) = weight × exp(-0.5 × ((x-μx)² + (y-μy)² + (z-μz)²) / σ²)
```

**Параметры**:
- `μ = (μx, μy, μz)` — центр гауссиана (3D позиция)
- `σ` — размер (standard deviation)
- `weight` — вклад в финальный цвет
- `color` — RGB цвет

### Рендеринг пайплайн
1. **Projection**: Трансформация 3D позиций в screen-space через VP матрицу
2. **Per-pixel evaluation**: Для каждого пикселя вычисляем вклад всех видимых гауссианов
3. **Blending**: Суммируем взвешенные цвета (альфа-блендинг или add-blend)

### Проблемы с полигональными сценами
- **Один гауссиан ≠ один треугольник**: Треугольники имеют рёбра, гауссианы — сферические
- **Нет sharp edges**: Гауссианы "размазаны" по краям
- **Сложное текстурирование**: UV-координаты не применимы напрямую

---

## 🎮 Traditional Rasterization: Как работает

### Pipeline (Vulkan)
1. **Vertex Shader**: Трансформация вершин через MVP матрицу
2. **Primitive Assembly**: Сборка треугольников из вершин
3. **Rasterization**: Преобразование треугольников в фрагменты (пиксели)
4. **Fragment Shader**: Вычисление цвета каждого фрагмента (текстуры + освещение)
5. **Depth Test**: Z-buffer для правильного порядка рендеринга
6. **Blending**: Альфа-блендинг для прозрачности

### Преимущества для Sponza:
✅ **Текстуры**: Sponza имеет **28 текстур** (стены, колонны, растения) — применяются напрямую  
✅ **Нормали**: Для **per-pixel lighting** (Phong, normal mapping)  
✅ **Эффективность**: GPU аппаратно оптимизирован для растеризации треугольников  
✅ **Sharp edges**: Чёткие рёбра колонн, арок, растений  

---

## 📊 Сравнение для Sponza (262K треугольников)

### Gaussian Splatting (текущая реализация)
- **Гауссианов**: 36,475 (subdivision × 4.5)
- **Покрытие**: ~14% от треугольников → **грубая аппроксимация**
- **Качество**: Размытые края, нет текстур, упрощённые цвета
- **Производительность**: ~60 FPS (Intel iGPU)

### Традиционный растеризатор (гипотетический)
- **Треугольников**: 262,000 (все)
- **Покрытие**: 100% геометрии
- **Качество**: Точная геометрия + текстуры + освещение
- **Производительность**: ~90-120 FPS (Intel iGPU с оптимизацией)

---

## 🎯 Когда использовать Gaussian Splatting

### ✅ Идеально подходит для:

1. **NeRF-подобные сцены**:
   - Фотограмметрия (сканирование реальных объектов)
   - Реконструкция из фото (SfM → Gaussian Splats)
   - Представление "мягких" объектов (облака, туман)

2. **Particle effects**:
   - Дым, огонь, взрывы
   - Волюметрический свет (god rays)
   - Plasma, энергетические эффекты

3. **Специальные задачи**:
   - Сжатие 3D сцен (NeRF → compacted Gaussians)
   - Рендеринг больших point clouds (лидар данные)
   - Аппроксимация сложных поверхностей

### ❌ НЕ подходит для:

1. **Традиционные 3D сцены**:
   - Игровые уровни (Sponza, Cornell Box)
   - CAD модели (архитектура, механика)
   - Анимированные персонажи (с рёбрами)

2. **Сцены с текстурами**:
   - Требуется UV mapping
   - Нормали для освещения
   - Материалы с roughness/metallic

---

## 💡 Hybrid подход (лучшее из обоих миров)

### Идея:
**Gaussian Splatting для эффектов + Traditional Rasterization для геометрии**

```cpp
// Псевдокод пайплайна
void renderFrame() {
    // 1) Традиционный растеризатор для Sponza
    rasterizer.render(sponzaMesh, textures);
    
    // 2) Gaussian Splatting для спец. эффектов
    gaussianPass.render(fireParticles);    // огонь факелов
    gaussianPass.render(dustParticles);    // пыль в воздухе
    gaussianPass.render(volumetricLight);  // лучи света
    
    // 3) Post-processing
    postProcess.compose(rasterizedImage, gaussianEffects);
}
```

### Преимущества:
✅ **Точная геометрия** от растеризатора  
✅ **Красивые эффекты** от Gaussian Splatting  
✅ **Оптимальная производительность** (каждая технология в своей области)  

---

## 🔧 Как перейти к Traditional Rasterization в SpectraForge

### Шаг 1: Создать Vulkan Rasterizer
```cpp
// include/SpectraForge/Rendering/VulkanRasterizer.h
class VulkanRasterizer : public IRenderer {
    vk::Pipeline graphicsPipeline_;
    vk::RenderPass renderPass_;
    vk::DescriptorSet textureDescriptorSet_;
    
    void createGraphicsPipeline();  // vertex + fragment shaders
    void loadMeshes(const OBJ& obj);
    void loadTextures(const std::vector<Texture>& textures);
};
```

### Шаг 2: Vertex + Fragment шейдеры
```glsl
// vertex_shader.vert
#version 450
layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inNormal;
layout(location = 2) in vec2 inTexCoord;

layout(push_constant) uniform MVP {
    mat4 model;
    mat4 view;
    mat4 proj;
} mvp;

layout(location = 0) out vec3 fragNormal;
layout(location = 1) out vec2 fragTexCoord;

void main() {
    gl_Position = mvp.proj * mvp.view * mvp.model * vec4(inPosition, 1.0);
    fragNormal = mat3(mvp.model) * inNormal;
    fragTexCoord = inTexCoord;
}
```

```glsl
// fragment_shader.frag
#version 450
layout(location = 0) in vec3 fragNormal;
layout(location = 1) in vec2 fragTexCoord;

layout(binding = 0) uniform sampler2D texSampler;

layout(location = 0) out vec4 outColor;

void main() {
    vec3 lightDir = normalize(vec3(0, 1, 0));
    float diffuse = max(dot(normalize(fragNormal), lightDir), 0.2); // ambient 0.2
    vec3 texColor = texture(texSampler, fragTexCoord).rgb;
    outColor = vec4(texColor * diffuse, 1.0);
}
```

### Шаг 3: Интеграция
```cpp
// src/app/Engine.cpp
auto rasterizer = std::make_shared<VulkanRasterizer>();
rasterizer->loadScene("examples/scenes/sponza/sponza.obj");
renderer_ = rasterizer; // заменить HybridFreGSRenderer
```

---

## 📚 Дополнительные ресурсы

### Gaussian Splatting:
- [3D Gaussian Splatting Paper](https://repo-sam.inria.fr/fungraph/3d-gaussian-splatting/)
- [Instant-NGP (NVIDIA)](https://github.com/NVlabs/instant-ngp)

### Vulkan Rasterization:
- [Vulkan Tutorial](https://vulkan-tutorial.com/)
- [Sponza Scene](https://www.crytek.com/cryengine/cryengine3/downloads)

---

## ✅ Заключение

**Для Sponza**: Традиционный растеризатор — **правильный выбор**  
**Для спец. эффектов**: Gaussian Splatting — **отличное дополнение**  
**Текущая реализация**: Хороша для **изучения Gaussian Splatting**, но **не для production-качества полигональных сцен**

---

**Version**: 1.0  
**Статус**: Рекомендации для архитектуры

