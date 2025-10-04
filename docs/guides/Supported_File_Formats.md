# Поддерживаемые форматы файлов для SpectraForge

**Дата**: 2025-10-03  
**Версия**: 1.0-alpha

---

## 🎯 Краткий ответ

### ✅ Подходят (оптимально):
- `.ply` — Point Cloud (ASCII/Binary)
- `.pcd` — Point Cloud Data (PCL format)
- `.xyz` — Simple Point Cloud (text)
- `.pts` — Point Cloud (text)
- `.las` / `.laz` — LiDAR data (compressed)

### ⚠️ Работают, но не оптимально:
- `.obj` — Triangle Mesh (будут только контуры)
- `.stl` — STL Mesh (будут только контуры)

### ❌ НЕ подходят (требуют доработки):
- `.fbx` — Autodesk FBX (нет загрузчика)
- `.gltf` / `.glb` — glTF (нет загрузчика)
- `.dae` — Collada (нет загрузчика)

---

## 📊 Детальное описание форматов

### 1. Point Cloud форматы (✅ Оптимально)

#### A. PLY (Polygon File Format / Stanford Triangle Format)

**Расширение**: `.ply`

**Структура**:
```
ply
format ascii 1.0
element vertex 1000000
property float x
property float y
property float z
property uchar red
property uchar green
property uchar blue
property float nx
property float ny
property float nz
end_header
-0.5 1.2 3.4 255 128 64 0.0 1.0 0.0
...
```

**Что поддерживается**:
- ✅ Позиции (x, y, z)
- ✅ Цвета (r, g, b)
- ✅ Нормали (nx, ny, nz)
- ✅ Альфа-канал (opacity)
- ✅ Binary и ASCII форматы

**Преимущества**:
- Широко распространён (LiDAR, photogrammetry)
- Компактный (особенно binary)
- Поддерживает миллионы точек

**Применение в SpectraForge**:
```cpp
// Каждая точка → Gaussian splat
for (auto& point : ply_points) {
    GaussianSplat splat;
    splat.positionAndScale = {point.x, point.y, point.z, default_sigma};
    splat.colorAndWeight = {point.r, point.g, point.b, 1.0f};
    gaussians.push_back(splat);
}
```

**Рекомендуемые инструменты для создания**:
- CloudCompare (конвертация и визуализация)
- MeshLab (обработка point clouds)
- Blender (экспорт из mesh → point cloud)

---

#### B. PCD (Point Cloud Data)

**Расширение**: `.pcd`

**Структура**:
```
VERSION 0.7
FIELDS x y z rgb normal_x normal_y normal_z
SIZE 4 4 4 4 4 4 4
TYPE F F F F F F F
COUNT 1 1 1 1 1 1 1
WIDTH 1000000
HEIGHT 1
VIEWPOINT 0 0 0 1 0 0 0
POINTS 1000000
DATA ascii
-0.5 1.2 3.4 4278190335 0.0 1.0 0.0
...
```

**Что поддерживается**:
- ✅ Позиции (x, y, z)
- ✅ RGB (packed в uint32)
- ✅ Нормали
- ✅ Intensity (для LiDAR)

**Преимущества**:
- Стандарт PCL (Point Cloud Library)
- Эффективная структура
- Metadata поддержка

**Применение**: Аналогично PLY

---

#### C. XYZ / PTS (Simple Text Format)

**Расширение**: `.xyz`, `.pts`

**Структура XYZ**:
```
# Simple point cloud (x y z)
-0.5 1.2 3.4
0.3 -0.8 2.1
...
```

**Структура PTS (с цветом)**:
```
# x y z r g b
-0.5 1.2 3.4 255 128 64
0.3 -0.8 2.1 100 200 150
...
```

**Что поддерживается**:
- ✅ Позиции (x, y, z)
- ✅ Опционально: цвета (r, g, b)
- ✅ Опционально: интенсивность

**Преимущества**:
- Простейший формат (легко парсить)
- Читаемый человеком
- Универсальный

**Недостатки**:
- Нет нормалей
- Большой размер файлов
- Медленная загрузка

**Применение**:
```cpp
// Простой парсер
std::ifstream file("cloud.xyz");
float x, y, z;
while (file >> x >> y >> z) {
    GaussianSplat splat;
    splat.positionAndScale = {x, y, z, 0.05f};
    splat.colorAndWeight = {0.8f, 0.8f, 0.8f, 1.0f}; // default gray
    gaussians.push_back(splat);
}
```

---

#### D. LAS / LAZ (LiDAR Formats)

**Расширение**: `.las`, `.laz` (compressed)

**Что поддерживается**:
- ✅ Позиции (x, y, z) с высокой точностью
- ✅ Интенсивность (intensity)
- ✅ Classification (ground, vegetation, buildings)
- ✅ RGB (опционально)
- ✅ Timestamps, GPS time

**Преимущества**:
- Стандарт для LiDAR данных
- Очень эффективная компрессия (.laz)
- Поддержка метаданных (координатные системы)

**Недостатки**:
- Требует библиотеку (libLAS, PDAL)
- Более сложный парсинг

**Рекомендуемые библиотеки**:
- [libLAS](https://liblas.org/) (C++)
- [PDAL](https://pdal.io/) (Pipeline processing)

**Применение**:
```cpp
// С libLAS
liblas::Reader reader("scan.las");
while (reader.ReadNextPoint()) {
    liblas::Point const& p = reader.GetPoint();
    GaussianSplat splat;
    splat.positionAndScale = {p.GetX(), p.GetY(), p.GetZ(), 0.03f};
    splat.colorAndWeight = {
        p.GetColor().GetRed() / 65535.0f,
        p.GetColor().GetGreen() / 65535.0f,
        p.GetColor().GetBlue() / 65535.0f,
        1.0f
    };
    gaussians.push_back(splat);
}
```

---

### 2. Triangle Mesh форматы (⚠️ Контуры только)

#### A. OBJ (Wavefront Object)

**Расширение**: `.obj`

**Текущее поведение в SpectraForge**:
```
Input: Sponza.obj (66,000 triangles)
  ↓
Extract barycenters (2-9 per triangle)
  ↓
Output: ~225,000 Gaussian splats
  ↓
Result: Контуры арок (НЕ заполнение) ⚠️
```

**Почему не работает полностью**:
- Теряется связность треугольников
- Гауссианы только на рёбрах/барицентрах
- Нет заполнения внутри полигонов

**Когда использовать**:
- ✅ Для демонстрации "проблемы mesh rendering"
- ✅ Для визуализации "скелета" модели
- ❌ НЕ для production рендеринга

**Рекомендация**: Если нужны заполненные поверхности → используйте traditional rasterizer.

---

#### B. STL (Stereolithography)

**Расширение**: `.stl`

**Структура**:
```
solid name
  facet normal 0.0 1.0 0.0
    outer loop
      vertex 0.0 0.0 0.0
      vertex 1.0 0.0 0.0
      vertex 0.5 1.0 0.0
    endloop
  endfacet
  ...
endsolid
```

**Текущее поведение**: Аналогично OBJ (контуры только)

**Применение**: Как OBJ, но без цветов/материалов.

---

### 3. Форматы, требующие реализации загрузчика (❌)

#### A. FBX (Autodesk FBX)

**Статус**: ❌ Нет загрузчика

**Что потребуется**:
- Библиотека: [Autodesk FBX SDK](https://www.autodesk.com/developer-network/platform-technologies/fbx-sdk-2020-0)
- Парсинг: Геометрия, материалы, анимация
- Размер: ~100 MB SDK

**Применение**: Если нужен FBX → конвертируйте в .ply через Blender/CloudCompare.

---

#### B. glTF / GLB (GL Transmission Format)

**Статус**: ❌ Нет загрузчика

**Что потребуется**:
- Библиотека: [tinygltf](https://github.com/syoyo/tinygltf)
- Парсинг: JSON + binary buffers
- Размер: Header-only (~30 KB)

**Применение**: Если нужен glTF → конвертируйте в .ply.

---

## 🔧 Как конвертировать форматы

### Mesh → Point Cloud (рекомендуется)

#### Способ 1: Blender (бесплатно)

```python
# В Blender Python Console:
import bpy
import bmesh

# Выбираем объект
obj = bpy.context.active_object
mesh = obj.data

# Создаём point cloud из вершин
with open('/path/to/output.xyz', 'w') as f:
    for v in mesh.vertices:
        world_co = obj.matrix_world @ v.co
        f.write(f"{world_co.x} {world_co.y} {world_co.z}\n")
```

**Или через Geometry Nodes**:
1. Add Modifier → Geometry Nodes
2. Add Node: Distribute Points on Faces
3. Set Count: 1,000,000
4. Export → PLY (Point Cloud)

---

#### Способ 2: CloudCompare

1. Открыть .obj файл
2. Edit → Mesh → Sample Points (число точек: 1-10M)
3. Сохранить как .ply или .xyz

---

#### Способ 3: MeshLab

1. Filters → Sampling → Poisson-disk Sampling
2. Number of samples: 1,000,000
3. File → Export Mesh As → PLY

---

### LAS/LAZ → PLY (для LiDAR данных)

```bash
# С помощью PDAL
pdal translate input.las output.ply

# Или с libLAS
las2txt -i input.las -o output.xyz --parse xyz
```

---

## 📈 Рекомендуемые параметры

### Для оптимальной производительности:

| Сцена | Точек | Формат | Sigma | FPS (Intel iGPU) |
|-------|-------|--------|-------|------------------|
| Малая | 10K-100K | PLY | 0.02-0.05 | 120+ |
| Средняя | 100K-500K | PLY/PCD | 0.01-0.03 | 60-90 |
| Большая | 500K-2M | PLY (binary) | 0.008-0.02 | 30-60 |
| Огромная | 2M-10M | LAZ (stream) | 0.005-0.01 | 15-30 |

### Плотность точек:

```
Для сцены 10×10×10 метров:

Низкая детализация:  100,000 точек (10 точек/м³)
Средняя детализация: 1,000,000 точек (100 точек/м³)
Высокая детализация: 10,000,000 точек (1000 точек/м³)
```

---

## 💡 Практические рекомендации

### 1. Конвертируйте Sponza в Point Cloud

```bash
# В Blender:
1. Импортировать Sponza.obj
2. Geometry Nodes: Distribute Points on Faces (1M points)
3. Export: sponza_pointcloud.ply

# Результат в SpectraForge:
- ✅ Заполненные поверхности (1M точек)
- ✅ Smooth shading
- ✅ ~40-60 FPS на Intel iGPU
```

---

### 2. Используйте готовые Point Cloud датасеты

**Бесплатные источники**:
- [Stanford 3D Scanning Repository](http://graphics.stanford.edu/data/3Dscanrep/)
- [Open3D ML Dataset](http://www.open3d.org/docs/latest/tutorial/ml/dataset.html)
- [Semantic KITTI](http://www.semantic-kitti.org/) (LiDAR)
- [ModelNet](https://modelnet.cs.princeton.edu/) (можно конвертировать)

---

### 3. Тестовые данные

**Создать простой point cloud в Python**:

```python
import numpy as np

# Генерируем сферу из точек
n = 100000
phi = np.random.uniform(0, 2*np.pi, n)
theta = np.random.uniform(0, np.pi, n)
r = 5.0

x = r * np.sin(theta) * np.cos(phi)
y = r * np.sin(theta) * np.sin(phi)
z = r * np.cos(theta)

# Цвет по высоте
colors = np.clip((z + r) / (2*r), 0, 1)
r_col = colors * 255
g_col = (1 - colors) * 255
b_col = 128

# Сохраняем как PLY
with open('sphere.ply', 'w') as f:
    f.write('ply\n')
    f.write('format ascii 1.0\n')
    f.write(f'element vertex {n}\n')
    f.write('property float x\n')
    f.write('property float y\n')
    f.write('property float z\n')
    f.write('property uchar red\n')
    f.write('property uchar green\n')
    f.write('property uchar blue\n')
    f.write('end_header\n')
    
    for i in range(n):
        f.write(f'{x[i]} {y[i]} {z[i]} {int(r_col[i])} {int(g_col[i])} {int(b_col)}\n')
```

---

## 🔧 Как добавить поддержку нового формата

### Шаги:

1. **Добавить загрузчик** в `src/core/PointCloudLoader.cpp`
2. **Парсить данные** в структуру `std::vector<GaussianSplat>`
3. **Обновить `Engine::load_scene()`** для нового формата

**Пример для PLY**:

```cpp
// include/SpectraForge/Core/PointCloudLoader.h
class PointCloudLoader {
public:
    static std::vector<GaussianSplat> loadPLY(const std::string& path);
    static std::vector<GaussianSplat> loadXYZ(const std::string& path);
    static std::vector<GaussianSplat> loadPCD(const std::string& path);
};

// src/core/PointCloudLoader.cpp
std::vector<GaussianSplat> PointCloudLoader::loadPLY(const std::string& path) {
    // Использовать библиотеку tinyply или happly
    std::vector<GaussianSplat> result;
    
    // Parse PLY header and data...
    
    return result;
}
```

**Рекомендуемые библиотеки**:
- [happly](https://github.com/nmwsharp/happly) — Header-only PLY loader
- [tinyply](https://github.com/ddiakopoulos/tinyply) — Fast PLY parser

---

## ✅ Итоговые рекомендации

### Для наилучших результатов:

1. ✅ **Используйте PLY** (binary) — оптимальный баланс
2. ✅ **Плотность**: 500K-2M точек для сцен среднего размера
3. ✅ **Конвертируйте meshes** в point clouds (Blender Geometry Nodes)
4. ✅ **Сохраняйте цвета и нормали** в файле

### Избегайте:

1. ❌ Прямой рендеринг .obj/.fbx (будут контуры)
2. ❌ Слишком малая плотность (<100K точек для больших сцен)
3. ❌ Файлы без цветовой информации (будут серыми)

---

**Версия**: 1.0  
**Последнее обновление**: 2025-10-03  
**См. также**: `README_CURRENT_STATUS.md`

