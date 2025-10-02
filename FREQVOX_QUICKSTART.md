# FreqVox Sponza Demo - Быстрый старт 🚀

## ✅ Сборка завершена!

Демо успешно собрано с **Hardware Detection** и готово к запуску!

```
Исполняемый файл: /home/tigron/Documents/GITHUB/SpectraForge/build/FreqVox_Sponza_Demo
Размер: 12 MB
Дата сборки: 2025-10-02
Особенности: Hardware-Aware FFT Backend Selection
```

## 🎮 Запуск демо

### Вариант 1: Через скрипт (рекомендуется)

```bash
cd /home/tigron/Documents/GITHUB/SpectraForge
./run_freqvox_sponza.sh
```

Скрипт автоматически:
- Проверит наличие сцены Sponza
- Проверит системные зависимости
- Запустит демо из правильной директории

### Вариант 2: Вручную

```bash
cd /home/tigron/Documents/GITHUB/SpectraForge
./build/FreqVox_Sponza_Demo
```

⚠️ **Важно:** Запускайте из корня проекта, чтобы демо нашло сцену `examples/scenes/sponza/`

## 🎮 Управление

### Камера
- **W** - Движение вперед
- **S** - Движение назад  
- **A** - Движение влево
- **D** - Движение вправо
- **SPACE** - Движение вверх
- **LEFT SHIFT** - Движение вниз
- **Мышь** - Поворот камеры
- **Scroll** - Изменение скорости (0.5 - 20.0)

### Переключение оптимизаций
- **F** - Фовеация ВКЛ/ВЫКЛ
- **T** - Temporal Reprojection ВКЛ/ВЫКЛ
- **U** - Upscaling ВКЛ/ВЫКЛ

### Прочее
- **ESC** - Выход из приложения

## 📊 Что вы увидите

### При запуске

**1. Hardware Detection (новое!):**
```
[Vulkan] Инициализация для Hardware Detection...
[Vulkan] GPU обнаружен: NVIDIA GeForce RTX 3060
[Vulkan] Вендор: NVIDIA
[Vulkan] VRAM: 12288 MB
[Vulkan] CUDA поддержка: Да
```

**2. Загрузка сцены:**
```
=== FreqVox Sponza Demo ===
Полнофункциональное демо FreqVox Renderer

[1/4] Загрузка сцены Sponza...
  - Мешей: 23
  - Вершин: 262267
  - Материалов: 20

[2/4] Вокселизация сцены...
  Создано 50000+ вокселей

[3/4] Инициализация FreqVox компонентов...
  ✓ FoveatedSelector создан
  
  [FFT Backend] Используется hardware-aware выбор
  ✓ FFT Backend инициализирован успешно

  💡 Информация о FFT backend:
     Режим: Hardware-Aware Selection
     GPU: NVIDIA GeForce RTX 3060
     Ожидаемый backend: cuFFT (NVIDIA CUDA)
  
  ✓ FrequencyShadingPipeline создан
  ✓ TemporalReprojection инициализирован
  ✓ NeuralUpscaler инициализирован

[4/4] Инициализация буферов рендеринга...

✅ Инициализация завершена успешно!
```

### Во время работы (каждые 60 кадров)
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

## 🧪 Эксперименты

### Тест 1: Базовый рендеринг (без оптимизаций)
```
1. Запустите демо
2. Нажмите F (отключить фовеацию)
3. Нажмите T (отключить temporal)
4. Нажмите U (отключить upscaling)
5. Наблюдайте базовую производительность
```

### Тест 2: Только фовеация
```
1. Нажмите F (включить фовеацию)
2. Оставьте T и U выключенными
3. Двигайте камеру и наблюдайте снижение вокселей
4. Проверьте разницу в производительности
```

### Тест 3: Полный пайплайн FreqVox
```
1. Нажмите F, T, U (включить всё)
2. Оцените максимальную производительность
3. Наблюдайте стабильность кадра
```

## 📚 Дополнительная документация

- **Полное руководство:** [examples/FREQVOX_SPONZA_DEMO.md](examples/FREQVOX_SPONZA_DEMO.md)
- **Технический отчет:** [FREQVOX_SPONZA_IMPLEMENTATION.md](FREQVOX_SPONZA_IMPLEMENTATION.md)
- **Примеры SpectraForge:** [examples/README.md](examples/README.md)
- **FreqVox интеграция:** [docs/FreqVox_Integration.md](docs/FreqVox_Integration.md)

## 🐛 Troubleshooting

### Демо не находит сцену
```bash
# Убедитесь что запускаете из правильной директории
cd /home/tigron/Documents/GITHUB/SpectraForge
./build/FreqVox_Sponza_Demo
```

### Ошибка "Cannot open display"
```bash
# Если запускаете через SSH, включите X11 forwarding
ssh -X user@host
# Или используйте xvfb для headless режима
xvfb-run ./build/FreqVox_Sponza_Demo
```

### Низкий FPS
- Проверьте что используется дискретная GPU (не интегрированная)
- Уменьшите размер вокселя в коде (строка ~428: `0.3f` → `0.5f`)
- Отключите Temporal/Upscaling для тестирования

### CUDA backend недоступен
```
Демо автоматически использует fallback на Simple backend.
Для использования cuFFT установите CUDA Toolkit и пересоберите:
  cmake .. -DBUILD_WITH_CUDA=ON
```

## 📝 Обратная связь

Если вы нашли баги или у вас есть предложения:

1. Проверьте [FREQVOX_SPONZA_IMPLEMENTATION.md](FREQVOX_SPONZA_IMPLEMENTATION.md) - Известные ограничения
2. Создайте issue на GitHub: https://github.com/TiGRoNdev/SpectraForge/issues
3. Или отправьте Pull Request с исправлением

## 🎉 Наслаждайтесь демо!

Вы только что запустили один из самых продвинутых алгоритмов рендеринга с:
- ✅ Фовеированной выборкой вокселей
- ✅ Частотным шейдингом (DCT/FFT)
- ✅ Темпоральной репроекцией
- ✅ Нейронным апскейлингом

Изучайте код, экспериментируйте с параметрами и создавайте что-то удивительное! 🚀

---

**Версия:** 1.0.0  
**Дата:** 2025-10-02  
**Лицензия:** MIT

