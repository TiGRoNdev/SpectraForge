# FreqVox Sponza Demo - Полнофункциональное демо FreqVox Renderer

## 📖 Описание

**FreqVox Sponza Demo** - это полнофункциональное демо-приложение, демонстрирующее все возможности FreqVox Renderer с использованием знаменитой сцены Crytek Sponza.

### Реализованные функции

1. **Hardware Detection** - Автоматическое определение GPU и выбор оптимального FFT backend'а ⭐ **НОВОЕ**
2. **OBJ Loader** - Загрузка геометрии из .obj/.mtl файлов
3. **Voxelizer** - Конвертация мешей в воксельное представление
4. **Foveated Selection** - Фовеированная выборка вокселей для оптимизации
5. **Frequency-Domain Shading** - Частотное шейдинг через DCT/FFT преобразования (cuFFT/VkFFT)
6. **Temporal Reprojection** - Временная репроекция для стабильности кадра
7. **Neural Upscaling** - Нейронный апскейлинг (DLSS/FSR2/Bilinear)
8. **Interactive Camera** - Полностью интерактивная камера с управлением WASD + мышь
9. **Real-time Statistics** - Детальная статистика производительности в реальном времени

## 🎮 Управление

### Движение камеры
- **W/S** - Движение вперед/назад
- **A/D** - Движение влево/вправо
- **SPACE** - Движение вверх
- **LEFT SHIFT** - Движение вниз
- **Мышь** - Поворот камеры
- **Scroll** - Изменение скорости движения (0.5 - 20.0)

### Переключение функций
- **F** - Включить/выключить фовеацию
- **T** - Включить/выключить темпоральную репроекцию
- **U** - Включить/выключить апскейлинг

### Прочее
- **ESC** - Выход из приложения

## 🛠️ Сборка

### Предварительные требования

- C++17 или новее
- CMake 3.16+
- GLFW3
- GLM
- CUDA Toolkit (опционально, для cuFFT backend)
- Vulkan SDK (опционально, для VkFFT backend)

### Команды сборки

```bash
# Из корня репозитория
mkdir build && cd build

# Конфигурация
cmake .. \
    -DENABLE_FREQVOX=ON \
    -DBUILD_EXAMPLES=ON \
    -DBUILD_WITH_CUDA=ON

# Сборка
cmake --build . --config Release --target FreqVox_Sponza_Demo

# Запуск (из корня репозитория!)
./build/FreqVox_Sponza_Demo
```

### Важно: Рабочая директория

⚠️ **Демо должно запускаться из корневой директории проекта**, так как путь к сцене Sponza задан относительно:

```bash
# ✅ ПРАВИЛЬНО
cd /path/to/SpectraForge
./build/FreqVox_Sponza_Demo

# ❌ НЕПРАВИЛЬНО
cd /path/to/SpectraForge/build
./FreqVox_Sponza_Demo  # Не найдет examples/scenes/sponza/
```

## 📁 Структура сцены

Демо ожидает следующую структуру файлов:

```
examples/
└── scenes/
    └── sponza/
        ├── sponza.obj      # Основной OBJ файл
        ├── sponza.mtl      # Файл материалов
        ├── *.JPG           # Текстуры
        └── *.jpg
```

Сцена Sponza уже включена в репозиторий в `examples/scenes/sponza/`.

## 📊 Статистика производительности

### Hardware Detection

При запуске демо автоматически определяет ваше железо:

```
[Vulkan] Инициализация для Hardware Detection...
[Vulkan] GPU обнаружен: NVIDIA GeForce RTX 3060
[Vulkan] Вендор: NVIDIA
[Vulkan] VRAM: 12288 MB
[Vulkan] CUDA поддержка: Да

💡 Информация о FFT backend:
   Режим: Hardware-Aware Selection
   GPU: NVIDIA GeForce RTX 3060
   Ожидаемый backend: cuFFT (NVIDIA CUDA)
```

**Автоматический выбор:**
- **NVIDIA GPU + CUDA** → cuFFT (максимальная производительность)
- **AMD/Intel/Other GPU** → VkFFT (кроссплатформенный)
- **Fallback** → Simple (для тестирования)

### Статистика производительности

Демо выводит детальную статистику каждые 60 кадров:

```
━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
📊 Статистика FreqVox Pipeline
━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
FPS: 60 (16.67 ms)
Камера: (0.0, 2.0, 5.0)

Этапы рендеринга:
  1. Voxel Selection:     2.5 ms (15000/50000 вокселей, 30%)
  2. Frequency Shading:   5.2 ms
  3. Temporal Reprojection: 1.8 ms
  4. Neural Upscaling:    3.1 ms

Настройки:
  Фовеация: ✅ ВКЛ
  Temporal: ✅ ВКЛ
  Upscaling: ✅ ВКЛ
━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
```

### Метрики:
- **FPS** - кадры в секунду
- **Voxel Selection** - время фовеированной выборки + количество выбранных вокселей
- **Frequency Shading** - время FFT/DCT преобразований и шейдинга
- **Temporal Reprojection** - время темпоральной репроекции
- **Neural Upscaling** - время апскейлинга

## 🧪 Тестирование различных конфигураций

### 1. Только базовый рендеринг (без оптимизаций)
```
Нажмите: F (выкл фовеацию) + T (выкл temporal) + U (выкл upscaling)
```

### 2. С фовеацией (оптимизация количества вокселей)
```
Нажмите: F (вкл фовеацию) + T (выкл temporal) + U (выкл upscaling)
```

### 3. С темпоральной репроекцией (стабильность)
```
Нажмите: F (выкл фовеацию) + T (вкл temporal) + U (выкл upscaling)
```

### 4. Полный FreqVox пайплайн (все оптимизации)
```
Нажмите: F (вкл фовеацию) + T (вкл temporal) + U (вкл upscaling)
```

## 🔧 Настройка параметров

### Параметры вокселизации

В файле `examples/freqvox_sponza_demo.cpp`, строка ~428:

```cpp
voxels_ = Voxelizer::voxelizeMeshes(meshes_, 0.3f);  // 0.3f - размер вокселя в метрах
```

- **Меньше** = больше вокселей, выше качество, ниже FPS
- **Больше** = меньше вокселей, ниже качество, выше FPS

Рекомендуемые значения: 0.2 - 0.5

### Параметры фовеации

В файле `examples/freqvox_sponza_demo.cpp`, строки ~441-445:

```cpp
FoveatedParams foveatedParams;
foveatedParams.foveal_radius = 15.0f;     // Радиус фовеальной области (м)
foveatedParams.foveal_angle = 20.0f;      // Угол фовеации (градусы)
foveatedParams.peripheral_sigma = 8.0f;   // Сигма спада на периферии
foveatedParams.far_plane = 200.0f;        // Дальняя плоскость отсечения
```

### Параметры темпоральной репроекции

В файле `examples/freqvox_sponza_demo.cpp`, строки ~470-473:

```cpp
TemporalReprojectionParams temporalParams;
temporalParams.blendFactor = 0.1f;                 // Фактор смешивания (0-1)
temporalParams.motionVectorThreshold = 5.0f;       // Порог motion vector (пиксели)
temporalParams.depthChangeThreshold = 0.05f;       // Порог изменения глубины
```

### Разрешение рендеринга

В файле `examples/freqvox_sponza_demo.cpp`, строки ~633-636:

```cpp
uint32_t renderWidth_ = 1920 / 2;   // Рендер в половинном разрешении
uint32_t renderHeight_ = 1080 / 2;
uint32_t displayWidth_ = 1920;      // Полное разрешение дисплея
uint32_t displayHeight_ = 1080;
```

## 🐛 Известные ограничения

1. **Упрощенная растеризация** - текущая реализация использует простую проекцию вокселей, а не полноценный Gaussian Splatting
2. **Заглушка motion vectors** - motion vectors пока не вычисляются, используются нулевые
3. **Простой апскейлер** - по умолчанию используется Bilinear upscaler, DLSS/FSR2 требуют дополнительной интеграции
4. **Нет текстур** - текстуры из .mtl файлов загружаются, но пока не используются в рендеринге
5. **Упрощенные SH коэффициенты** - SH инициализируются на основе цвета материала и нормали

## 🚀 Планы развития

- [ ] Реальная интеграция Gaussian Splatting (FlashGS)
- [ ] Вычисление реальных motion vectors
- [ ] Интеграция DLSS через Streamline
- [ ] Интеграция FSR2
- [ ] Поддержка текстур
- [ ] GPU-ускоренная вокселизация (CUDA)
- [ ] Octree для воксельной структуры
- [ ] PBR материалы и IBL
- [ ] Post-processing эффекты (bloom, tone mapping)

## 📚 Связанная документация

- [FreqVox Концепция](../docs/concept/FreqVox%20Renderer.md)
- [FreqVox Математика](../docs/concept/FreqVox%20Renderer%20Math.md)
- [FreqVox Интеграция](../docs/FreqVox_Integration.md)
- [FreqVox Реализация](../FREQVOX_IMPL.md)
- [Architecture](../docs/architecture/ARCHITECTURE.md)

## 🤝 Вклад

При внесении изменений в демо:

1. Следуйте [Coding Standards](../docs/CODING_STANDARDS.md)
2. Обновляйте документацию
3. Тестируйте на различных конфигурациях
4. Измеряйте производительность

## 📝 Лицензия

См. [LICENSE](../LICENSE) в корне репозитория.

---

**Версия:** 1.0.0  
**Дата:** 2025-10-02  
**Автор:** SpectraForge Team

