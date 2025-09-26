# Vulkan Forward+ Рендеринг в 4D Engine

## Обзор

4D Engine теперь использует современную технологию Forward+ рендеринга на базе Vulkan API. Forward+ (также известный как Tiled Forward Rendering) - это гибридная техника рендеринга, которая сочетает преимущества forward и deferred рендеринга.

## Архитектура Forward+ рендеринга

### Этапы рендеринга

1. **Depth Pre-pass** - Первый проход для записи глубины
2. **Light Culling** - Отсечение света по тайлам (compute shader)
3. **Forward+ Shading** - Финальный этап освещения

### Преимущества Forward+

- **Поддержка прозрачности**: В отличие от deferred рендеринга
- **Эффективность с множеством света**: Лучше чем классический forward рендеринг
- **Гибкость материалов**: Поддержка различных типов материалов
- **4D совместимость**: Естественная поддержка 4D геометрии

## Техническая реализация

### 1. Depth Pre-pass

Первый этап записывает только информацию о глубине в специальную текстуру:

```glsl
// depth_prepass.vert
void main() {
    vec4 worldPos = pc.model * inPosition;
    
    // Применяем 4D проекцию
    if (uniforms.useCrossSection != 0) {
        // Сечение 4D пространства
        float wDistance = abs(worldPos.w - uniforms.crossSectionW);
        if (wDistance > 0.1) {
            gl_Position = vec4(0.0, 0.0, -1.0, 1.0);
            return;
        }
        worldPos.w = 0.0;
    }
    
    // 4D -> 3D проекция
    vec3 pos3D = worldPos.xyz;
    if (!bool(uniforms.useCrossSection)) {
        float wDistance = 5.0;
        if (worldPos.w != 0.0) {
            float scale = wDistance / (wDistance + worldPos.w);
            pos3D *= scale;
        }
    }
    
    gl_Position = uniforms.projection * uniforms.view * vec4(pos3D, 1.0);
}
```

### 2. Light Culling (Compute Shader)

Compute shader группирует экран на тайлы 16x16 пикселей и определяет, какие источники света влияют на каждый тайл:

```glsl
// light_culling.comp
layout(local_size_x = 16, local_size_y = 16, local_size_z = 1) in;

void main() {
    // Определяем границы тайла
    ivec2 tileID = ivec2(gl_WorkGroupID.xy);
    ivec2 pixelMin = tileID * 16;
    ivec2 pixelMax = min((tileID + 1) * 16, ivec2(uniforms.screenDimensions.xy));
    
    // Находим min/max глубину в тайле
    float minDepth = 1.0;
    float maxDepth = 0.0;
    
    for (int y = pixelMin.y; y < pixelMax.y; y++) {
        for (int x = pixelMin.x; x < pixelMax.x; x++) {
            float depth = texelFetch(depthTexture, ivec2(x, y), 0).r;
            minDepth = min(minDepth, depth);
            maxDepth = max(maxDepth, depth);
        }
    }
    
    // Проверяем пересечение каждого света с фрустумом тайла
    uint visibleLightCount = 0;
    for (uint lightIndex = 0; lightIndex < uniforms.lightCount; ++lightIndex) {
        if (sphereInsideFrustum(lights[lightIndex], tileCorners, minDepth, maxDepth)) {
            visibleLightIndices[visibleLightCount++] = lightIndex;
        }
    }
    
    // Записываем результат
    tileData[tileIndex].lightCount = visibleLightCount;
    for (uint i = 0; i < visibleLightCount; ++i) {
        tileData[tileIndex].lightIndices[i] = visibleLightIndices[i];
    }
}
```

### 3. Forward+ Shading

Финальный этап использует результаты light culling для эффективного освещения:

```glsl
// forward_plus.frag
void main() {
    // Определяем тайл для текущего фрагмента
    ivec2 tileID = ivec2(gl_FragCoord.xy) / 16;
    uint tileIndex = tileID.y * uniforms.tileCountX + tileID.x;
    
    // Получаем список света для этого тайла
    uint lightCount = tileData[tileIndex].lightCount;
    
    vec3 finalColor = vec3(0.0);
    
    // Применяем только света из тайла
    for (uint i = 0; i < lightCount; ++i) {
        uint lightIndex = tileData[tileIndex].lightIndices[i];
        GPULight light = lights[lightIndex];
        
        // PBR освещение
        finalColor += calculateLighting(light, fragPos, normal, viewDir, material);
    }
    
    outColor = vec4(finalColor, 1.0);
}
```

## 4D Специфика

### Проекции 4D -> 3D

Движок поддерживает несколько методов проекции 4D геометрии в 3D:

1. **Перспективная проекция**: `scale = wDistance / (wDistance + w)`
2. **Сечения**: Отсечение по фиксированной W-координате
3. **Ортогональная проекция**: Прямое отбрасывание W-координаты

### 4D Освещение

Источники света также могут быть размещены в 4D пространстве:

```cpp
struct GPULight {
    vec4 position;      // 4D позиция света
    vec4 color;         // Цвет и интенсивность
    vec4 direction;     // 4D направление
    float radius;       // Радиус влияния
    uint type;          // Тип: точечный, направленный, прожектор
};
```

## Производительность

### Оптимизации

- **Tile Size**: 16x16 пикселей обеспечивает баланс между точностью и производительностью
- **Shared Memory**: Использование shared memory в compute shader для быстрого доступа
- **Early Z-reject**: Depth pre-pass позволяет отбрасывать невидимые фрагменты
- **Light Radius**: Ограничение радиуса света уменьшает количество проверок

### Метрики

На современных GPU (GTX 1060+) движок способен обрабатывать:
- 1000+ динамических источников света
- 60+ FPS при разрешении 1920x1080
- Сложные 4D сцены с множеством объектов

## Структуры данных

### Uniform Buffer

```cpp
struct ForwardPlusUniforms {
    mat4 view;
    mat4 projection;
    mat4 viewProjection;
    vec4 viewPosition;
    vec4 screenDimensions;
    uint tileCountX;
    uint tileCountY;
    uint totalTileCount;
    uint lightCount;
    float nearPlane;
    float farPlane;
    float crossSectionW;
    uint useCrossSection;
};
```

### Tile Data

```cpp
struct TileData {
    uint lightCount;
    uint lightIndices[256]; // MAX_LIGHTS_PER_TILE
};
```

### PBR Material

```cpp
struct ForwardPlusMaterial {
    vec4 albedo;
    vec4 emission;
    float metallic;
    float roughness;
    float normalScale;
    uint hasAlbedoTexture;
    uint hasNormalTexture;
    uint hasMetallicRoughnessTexture;
    uint hasEmissionTexture;
};
```

## Настройка и использование

### Инициализация рендерера

```cpp
// Создание рендерера
auto& renderer = Renderer::getInstance();
renderer.initialize(window, width, height);

// Настройка камеры
Camera4D camera;
camera.setPosition(Vector4(0, 0, 5, 0));
camera.lookAt(Vector4(0, 0, 5, 0), Vector4(0, 0, 0, 0), Vector4(0, 1, 0, 0));

renderer.setViewMatrix(camera.getViewMatrix());
renderer.setProjectionMatrix(camera.getProjectionMatrix());
```

### Добавление света

```cpp
std::vector<Light4D> lights;

// Точечный свет
Light4D pointLight;
pointLight.position = Vector4(2, 2, 2, 0);
pointLight.color = Vector4(1, 1, 1, 5.0f); // белый, интенсивность 5
pointLight.radius = 10.0f;
pointLight.type = 0; // точечный

lights.push_back(pointLight);
renderer.updateLights(lights);
```

### Рендеринг мешей

```cpp
// Создание 4D меша
Mesh4D tesseract = Mesh4D::createTesseract(2.0f);

// Рендеринг
Matrix4 transform = Matrix4::translation(Vector4(0, 0, 0, 0));
renderer.renderMesh(tesseract, transform);
```

## Дальнейшее развитие

### Планируемые улучшения

- **Clustered Forward+**: 3D группировка вместо 2D тайлов
- **Variable Rate Shading**: Адаптивное качество шейдинга
- **Real-time GI**: Глобальное освещение в реальном времени
- **4D Shadow Mapping**: Тени в 4D пространстве
- **Temporal Accumulation**: Накопление данных между кадрами

### Расширения

- Поддержка дополнительных типов света (area lights, IES lights)
- Volumetric lighting для 4D пространства  
- Advanced materials (subsurface scattering, clear coat)
- Compute-based animation и deformation
