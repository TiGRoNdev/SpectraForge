# FreqVox Renderer (AFS-NVR) — Алгоритм, стратегия внедрения и дальнейшие действия

Документ фиксирует целевой алгоритм FreqVox, архитектурную стратегию его интеграции в HyperEngine и детальный план работ. Основано на: `docs/concept/FreqVox Renderer.md`, `docs/concept/FreqVox Renderer Math.md`.


## 1) Цели и критерии готовности

- Реализовать гибридный пайплайн: фовеированная выборка вокселей → частотный шейдинг (DCT/FFT) → темпоральная репроекция → нейро‑апскейл.
- Достичь низкого времени кадра за счёт снижения активных вокселей V_eff, батчированных DCT/FFT и апскейлинга.
- Включаемость через флаг `ENABLE_FREQVOX` и стратегию `FreqVoxStrategy` без ломки legacy путей.
- Тесты ≥80% покрытия для новых модулей; AAA; GoogleTest/GMock.
- Документация, Doxygen к публичным API; CI зелёный.


## 2) Архитектура и точки интеграции

- Новые модули (созданы):
  - `include/HyperEngine/Rendering/FreqVox/` — заголовки модели данных и этапов
    - `FreqVoxTypes.h` — SH9, `Voxel`, `DctBlockConfig`
    - `FoveatedSelector.h` — интерфейс фовеированной выборки
    - `FrequencyShading.h` — абстракция `IFrequencyBackend` и `FrequencyShadingPipeline`
    - `FreqVoxRenderStage.h` — этап рендера FreqVox (IRenderStage)
    - `FreqVoxStrategy.h` — стратегия рендера FreqVox (IRenderStrategy)
    - `Backends/SimpleDctBackend.h` — простой backend-заглушка DCT
  - `src/rendering/freqvox/` — реализации
    - `FoveatedSelector.cpp`, `FreqVoxRenderStage.cpp`, `FreqVoxStrategy.cpp`
    - `Backends/SimpleDctBackend.cpp`
- Подключение в сборку:
  - `CMakeLists.txt`: флаг `ENABLE_FREQVOX` + линковка `HyperEngine_FreqVox` к `HyperEngine` при наличии.
  - `src/CMakeLists.txt`: цель `HyperEngine_FreqVox` при `ENABLE_FREQVOX`.
- Выбор стратегии:
  - `StrategyFactory`: `create_strategy_by_name("freqvox")` → `FreqVoxStrategy` (при `HyperEngine_ENABLE_FREQVOX`).
  - Интеграция в место создания `ModernRenderer3D` (через фабрику/DI): передать `FreqVoxStrategy` в конструктор либо вызвать `setRenderStrategy(...)`.


## 3) Алгоритм FreqVox (высокоуровневый)

1. Sparse Voxel Encoding
   - Разрежённое представление сцены (решётка/октодерево/хэш‑грид).
   - Воксель хранит позицию, LOD bias, низкочастотную радиансную информацию в SH до ℓ=2 (9 коэф/канал).

2. Foveated Selection & LOD
   - Вес пикселя по Гауссу от углового отклонения от взгляда; `σ≈5°`.
   - Снижение детальности вне фовеа; фрустум‑ и occlusion‑culling; пороги отсечения по размеру; учёт дистанции.
   - Результат — активные воксели `V_eff`.

3. Frequency‑Domain Shading (DCT/FFT)
   - Оценка SH → низкочастотная «текстура» `L_i[p,q]` на воксель‑блок.
   - Прямое преобразование (DCT/FFT) по блокам и батчам: `\tilde{L}_i`.
   - Покомпонентное умножение с частотным BRDF/материалом `\tilde{M}`: `\tilde{S}_i = \tilde{L}_i ⊙ \tilde{M}`.
   - Обратное преобразование (IDCT/IFFT) → `S_i[p,q]`.

4. Temporal Reprojection & Reconstruction
   - Ре‑проекция `u_{t-1} = u_t - v(u_t)`; смешивание `C_t = α S_t + (1-α) C_{t-1}`; пороги по скорости/глубине.

5. Neural Enhancement / Upscaling
   - NVIDIA: Streamline DLSS; AMD/остальное: FSR2; фоллбек: Tiny CNN (≈0.5 ms/frame, ≤1e9 FLOPs).
   - Рендер с базовым `ρ≈0.5` → апскейл → перцептуальное усиление.

6. CUDA–Vulkan Interop и синхронизация
   - `VK_KHR_external_memory`/`external_semaphore` (timeline), корректные лейауты и барьеры при handoff.
   - CUDA streams для FFT/нейро; Vulkan queue для презентации; минимум копий.


## 4) Текущая реализация (MVP каркас)

- Файлы:
  - `FreqVoxTypes.h` — SH9/Voxel/DctBlockConfig готово.
  - `FoveatedSelector` — заглушка (возвращает все воксели) — будет расширена камерой/взглядом.
  - `FrequencyShadingPipeline` — абстракция над бэкендом, поддержка forward→inverse.
  - `SimpleDctBackend` — заглушка: тождественные преобразования для интеграционных тестов.
  - `FreqVoxRenderStage` — сборка этапа, замер времени, регистрация в `RenderContext.debug.stageTimes`.
  - `FreqVoxStrategy` — стратегия на базе IRenderStrategy, вызывает стадию.
  - `StrategyFactory` — отдаёт `FreqVoxStrategy` по имени `"freqvox"`.
- Тесты:
  - `freqvox_stage_test`, `freqvox_strategy_test` — smoke‑проверки жизненного цикла.


## 5) План развития (инкременты)

### 5.1 Бэкенды FFT/DCT

- `cuFFTBackend` (NVIDIA):
  - Планирование: `cufftPlanMany` для батчей 2D, `cufftSetStream` для overlap.
  - DCT: через FFT‑трюк или VkFFT‑DCT; сначала — C2C/реал‑комплексный путь.
  - CUDA error‑handling, RAII, reuse планов, pinned/pooled память.
- `VkFFTBackend` (кроссплатформенно):
  - Интеграция VkFFT (header‑only/FetchContent), DCT‑II/III.
  - Vulkan буферы, внешняя память при совместном использовании с CUDA.
- Выбор бэкенда: по вендору/флагам `ENABLE_FREQVOX`, `BUILD_WITH_CUDA`.

### 5.2 Фовеированная выборка

- Интеграция данных камеры/взгляда; расчёт угла и дистанции; корректные веса `w_i`.
- Frustum‑/Occlusion‑culling; LOD bias/порог отбора; профилирование.

### 5.3 Частотный BRDF/материалы

- Подготовка `\tilde{M}` (статично/динамически), кэширование по материалам.
- Контроль размеров блоков (например, 8×8) и выравнивание батчей.

### 5.4 Temporal & Motion Vectors

- Хранение истории, вычисление/приём motion vectors; выбор `α`, порогов `δ`, `ε`.
- Стабильность: отработка дисокклюзий, clamp по яркости/вариансе.

### 5.5 Upscaling

- DLSS (Streamline) на Windows и FSR2 на Linux; фоллбек Tiny CNN (CUDA/TensorRT/OpenCL).
- API фасад `NeuralUpscaler` с переключением по `HardwareDetector`.

### 5.6 Vulkan–CUDA Interop

- Allocation c `VK_KHR_external_memory`; импорт/экспорт в CUDA; timeline семафоры.
- Барьеры/лейауты для внешнего доступа; тест `ExternalMemory_Test` как эталон.


## 6) Интеграция в пайплайн движка

- Стратегия:
  - В месте создания `ModernRenderer3D` использовать `StrategyFactory` по конфигу (`renderer.strategy = "freqvox"`).
  - Либо `renderer->setRenderStrategy(create_strategy_by_name("freqvox"))` после инициализации.
- Этапы:
  - `FreqVoxRenderStage` можно зарегистрировать в существующем менеджере стадий (если используется фабрика стадий) или вызывать из стратегии.
- Переключаемость:
  - Конфиг/CLI `--renderer=freqvox|legacy`.
  - Автовыбор по `HardwareDetector` (вендор, RT, tensor cores).


## 7) Build/Deps

- Флаги: `ENABLE_FREQVOX`, `BUILD_WITH_CUDA`, `BUILD_WITH_FSR`, `BUILD_WITH_DLSS`.
- Подключение cuFFT (при CUDA), VkFFT (FetchContent или vcpkg рецепт), VK_KHR external memory/semaphores.
- CI: матрица с CUDA ON/OFF, Vulkan ON, Linux/Windows (DLSS ограничение — Windows).


## 8) Тестирование и качество

- Unit: SH оценка, фовеация, DCT/IDCT корректность, pipeline шаги, upscaler фасад.
- Integration: маленькая сцена → кадр; измерение времени этапов; артефакты/стабильность.
- Performance: бенч batched FFT (разные размеры), профилирование, аллокации → пулы.
- Стандарты: SOLID, RAII, const‑correctness, smart‑pointers, C++17/20; SAFE_TO_STRING для вывода.
- Документация: Doxygen на публичные API; обновление `docs/architecture`.


## 9) Безопасность и производительность

- Внешние входы валидировать; без небезопасных C‑API; без C‑массивов в ownership.
- Оптимизация allocations; переиспользование FFT планов; `cufftSetStream` и CUDA streams.
- Синхронизация с внешними семафорами (timeline), корректные лейауты.


## 10) Поэтапная миграция core пути

1. Фаза 1 — параллельный путь (фич‑флаг), legacy не ломаем.
2. Фаза 2 — дефолт на поддерживаемом железе, автоселектор vendor.
3. Фаза 3 — отключение legacy после стабильности и бенчмарков.


## 11) Дорожная карта (тикеты)

- Бэкенды FFT/DCT
  - cuFFTBackend: планирование батчей, RAII, тесты — P0
  - VkFFTBackend: DCT‑II/III, интеграция, тесты — P1
- Фовеация/LOD — P0: камера, веса, culling, LOD пороги
- Материалы/BRDF в частотной области — P1
- Темпоральная репроекция — P0
- Upscaling: FSR2/DLSS/TinyCNN — P1 (DLSS Win‑only)
- Interop: external memory/semaphores, тесты — P0
- Интеграция стратегии в фабрику/DI — P0
- Тесты: unit/integration/perf (≥80%) — P0
- Документация и UML — P1


## 12) Ссылки

- Концепция: `docs/concept/FreqVox Renderer.md`
- Математика: `docs/concept/FreqVox Renderer Math.md`
- Vulkan External Memory/Semaphores (timeline): VK_KHR спецификации
- cuFFT: планирование батчей (`cufftPlanMany`), `cufftSetStream`
- VkFFT: DCT/FFT для Vulkan буферов
- DLSS/FSR2: Streamline/FFX


---
Обновлено: текущая ветка `feature/freqvox-renderer`; сборка контролируется `ENABLE_FREQVOX`.


