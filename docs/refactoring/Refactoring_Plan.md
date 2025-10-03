# SpectraForge: План рефакторинга под Hybrid DWT + FreGS и унификацию структуры проекта

Версия: 1.0 • Дата: 2025‑10‑02 • Автор: Core Team

Цель: перевести движок на гибридный пайплайн Hybrid DWT + FreGS (Wavelet Lifting + Frequency‑Encoded Gaussian Splatting) согласно `docs/concept/Renderer.md`, унифицировать структуру исходников и документации, удалить устаревшие артефакты, усилить тестирование и CI/CD, обеспечить соблюдение SOLID и внутренних мастер‑правил.

---

## 0. Краткие цели и KPI

- Производительность: ≥500 FPS при 8K на целевых мобильных GPU (Adreno 640–650/Mali‑G77) при P≤5W
- Качество: сохранение ключевого частотного контента, минимизация артефактов при фовеации и репроекции
- Технический долг: уменьшение за счёт централизации docs, удаления дублей и legacy‑кода
- Качество кода: соблюдение SOLID, современный C++17/20, RAII, VMA, отсутствие сырых указателей для владения
- Тесты: ≥80% покрытия для нового кода, AAA‑паттерн, gtest/GoogleMock

---

## 1. Архитектурная целевая модель (согласно `docs/concept/Renderer.md`)

1) Wavelet Lifting (Daubechies‑4, 2D, fused H+V) на Vulkan compute с Subgroups
   - Локальные тайлы 32×32, coalesced loads, одна барьера; O(≈8) FLOP/пиксель
   - Выход: LL/HH (и при необходимости LH/HL) субдиапазоны в fp16

2) Frequency‑Encoded Gaussian Splatting (FreGS)
   - Аналитическая свертка в частотной области: экспоненциальное подавление по σ, cos‑фаза по μ
   - Аккумуляция через `OpSubgroupReduceAdd`, без глобальных атомиков

3) Foveation alignment
   - Полный спектр в центральном конусе ~10°, понижение уровней вне фовеа; до −75% операций

4) Temporal reprojection & blending
   - Полноспектрально в фовеа, low‑pass в периферии; velocity/depth‑thresholding для skip

Вендор‑специфика (опционально):
   - Upscaling: NVIDIA Streamline DLSS (RTX) или AMD FidelityFX FSR2 (широкая поддержка)
   - Denoising (при RTX‑пути): OptiX AI Denoiser

---

## 2. Область работ и негарантированные аспекты

Входит:
- Рендер‑пайплайн Hybrid DWT + FreGS на Vulkan 1.3 (Subgroups)
- Опциональный апскейлинг через Streamline DLSS/FSR2
- Стандартизация структуры исходников и docs, удаление дублей/legacy
- Тесты, статический анализ, CI/CD

Не входит (в этой итерации):
- Полная переработка OptiX‑RT пайплайна; оставляем модуль как дополнительный путь
- Глубокая физика/инпут/игровая логика — только минимальные адаптации интерфейсов

---

## 3. План изменений по модулям

### 3.1 Rendering
- Добавить compute‑пайплайн `WaveletLifting.comp` и `GaussFreqSplat.comp` (GLSL 450 + subgroups)
- Обновить `Renderer`/`RendererAdapter`: 
  - Абстракции для passes: `WaveletPass`, `FreGSPass`, `FoveationStage`, `ReprojectionStage`
  - Ввести DIP: зависимости через интерфейсы; фабрика рендер‑пайплайна по конфигу
- Состояния Vulkan:
  - Требования: Vulkan 1.3, `shaderSubgroupExtendedTypes` где нужно, `subgroupSupportedStages` включает compute
  - Барьеры: compute→compute, корректные `src/dstStage` и `access` маски
- Ресурсы:
  - Изображения субполос в `rgba16f`; выходной аккумулятор — fp16/fp32 в зависимости от платформы
  - Дескрипторы: группы на тайл/субдиапазон, пулл с pre‑allocation

### 3.2 Upscaling
- Абстракция `Upscaler` с реализациями:
  - `UpscalerDLSS` через NVIDIA Streamline (Vulkan интеграция)
  - `UpscalerFSR2` через AMD FidelityFX FSR2
- Выбор реализации — `HardwareDetector` (NV→DLSS, прочее→FSR2), фича‑флаг для ручной фиксации

### 3.3 Resource/Memory
- Повсеместно использовать VMA (Vulkan Memory Allocator); 
  - Пулы под transient images/buffers для compute‑пасс
  - Политика дефрагментации off‑frame/инкрементальная (ограничить bytes/allocations per pass)

### 3.4 OptiX (опционально)
- Сохранить модуль для денойзинга (если включён RTX‑путь)
- CUDA/Vulkan interop — только при активном OptiX, иначе дефолт чисто Vulkan

### 3.5 Math
- Упростить зависимости от FFT/DCT в Hot‑path (FreqVox)
- Оставить VkFFT как LEGACY/Research путь за фича‑флагом `SPECTRAFORGE_RENDERER=FREQVOX`

### 3.6 Examples
- Добавить `examples/hybrid_fregs_demo.cpp` (минимальный путь: загрузка сцены → DWT → FreGS → upscaler → present)
- Обновить существующие демо: пометить LEGACY, где применимо; перенаправить на новый пайплайн

---

## 4. Унификация структуры проекта

Целевая структура:

```
include/
  SpectraForge/                  # публичные API (классы, интерфейсы)
src/
  core/                          # Engine/Scene/ResourceManager/HardwareDetector
  rendering/                     # Renderer, passes (Wavelet, FreGS, Foveation, Reprojection)
  upscaling/                     # Upscaler (DLSS/FSR2 adapters)
  math/                          # Вспомогательная математика
  optix/                         # Опциональный денойз/RT
  cuda/                          # Только при OptiX‑пути (interop)
shaders/
  WaveletLifting.comp
  GaussFreqSplat.comp
tests/
  unit/                          # gtest + gmock (AAA)
  integration/
docs/
  architecture/
    ARCHITECTURE.md
    Renderer.md                  # (переместить из docs/concept)
  modules/
    rendering.md
    upscaling.md
    resource_memory.md
  api/
  guides/
  reports/
  concept/                       # Исследовательские материалы
  legacy/                        # FreqVox и др. унаследованные материалы
examples/
cmake/
scripts/
```

Правила:
- Один класс — один файл; заголовки с `#pragma once`
- Именование: PascalCase — классы; snake_case — функции/переменные; UPPER_CASE — макросы
- Никаких `using namespace std;` в заголовках
- RAII, unique_ptr/shared_ptr для владения; исключения — только по делу

---

## 5. Централизация и миграция документации

Переместить/сгруппировать:
- `docs/concept/Renderer.md` → `docs/architecture/Renderer.md`
- Все `FREQVOX_*.md` из корня → `docs/legacy/freqvox/` (с индексом и предупреждением об устаревании)
- `VULKAN_*FIX*.md`, `ИСПРАВЛЕНИЕ_VULKAN_RU.md` → `docs/reports/`
- `BUILD_*.md`, `QUICK_INSTALL.md`, `INSTALL_GUIDE_RU.md` → `docs/guides/`
- `CODING_STANDARDS.md`, `DEVELOPER_GUIDE.md`, `DEVELOPMENT_TOOLS.md` уже в `docs/` — сверить дубли, выровнять оглавления

Обновить перекрёстные ссылки и ToC.

---

## 6. Кандидаты на удаление/перемещение (предварительный список)

Перемещение в `docs/legacy/` (сохранить как исторические материалы):
- Все файлы `FREQVOX_*.md` из корня
- Демонстрации, завязанные на старый FreqVox путь: `examples/freqvox_*` (если не используются в новых тестах)

Архивация/удаление после валидации:
- `test_glfw_vulkan_hpp_conflict.cpp` (если фикс применён и тест неактуален)
- Дублирующиеся build‑инструкции (оставить один канонический источник в `docs/guides/`)

Оставить, но пометить LEGACY (фича‑флаг `FREQVOX`):
- Любые FFT/DCT‑пути в коде и демо, если они ещё нужны для сравнения/бенчей

Примечание: финальный список формируется после прохода по использованию символов и бинарных зависимостей (CI логика «неиспользуемых целей» в CMake, `compile_commands.json`, статанализ).

---

## 7. CMake и зависимости

Новые опции:
- `SPECTRAFORGE_RENDERER`: `FREGS` (по умолчанию) | `FREQVOX`
- `SPECTRAFORGE_UPSCALER`: `AUTO` (по умолчанию) | `DLSS` | `FSR2` | `NONE`
- `SPECTRAFORGE_RT_DENOISER`: `OFF` (по умолчанию) | `OPTIX`

Требования:
- Vulkan SDK 1.3+, включить Subgroup‑фичи
- VMA (VulkanMemoryAllocator)
- (Опционально) NVIDIA Streamline SDK (DLSS), AMD FidelityFX FSR2

Сборка шейдеров: `glslangValidator` → SPIR‑V, преднастройка через `compile_shaders.*`

CI: кэш артефактов шейдеров и внешних SDK; матрица сборок (Linux, Windows)

---

## 8. Тестирование и качество

- Unit: 
  - Верификация lifting‑операций на синтетике (сопоставление с CPU‑эталоном на малых тайлах)
  - Редакции FreGS накопления (погрешность суммирования vs. референс)
- Integration:
  - Кадровые «golden images» (фовеа/периферия, движение/статичная сцена)
  - Режимы апскейлинга DLSS/FSR2 ( smoke tests )
- AAA‑паттерн; ≥80% покрытия на новом коде; ASAN/UBSAN в CI; clang‑tidy, cppcheck

---

## 9. Безопасность и производительность

- Input validation на внешних данных/шейдерах
- Исключительная безопасность (RAII), отсутствие сырых владений
- VMA‑пулы, минимизация аллокаций в hot‑path; предвыделение дескрипторов
- Subgroup‑редукции вместо глобальных атомиков; разумные рабочие размеры (учёт `maxComputeWorkGroup*`)

---

## 10. Пошаговый план миграции (итерации)

1) Аудит и каталогизация артефактов (код, шейдеры, docs, скрипты)
2) Введение feature‑флагов и базовой каркаса `Renderer`/passes
3) Реализация `WaveletLifting.comp` + host‑обвязка; тесты
4) Реализация `GaussFreqSplat.comp` + интеграция; тесты
5) Foveation alignment + маски; тесты производительности
6) Temporal reprojection & blending; регресс‑тесты
7) Upscaling адаптеры (DLSS/FSR2) + smoke tests
8) Docs: миграция/централизация, выравнивание ToC, ссылки
9) Cleanup: перенос в `legacy/`, удаление неисп. тестов/скриптов
10) Финальная стабилизация, релиз‑ноутсы, обновление примеров

---

## 11. Риски и меры

- Различия в поддержке Subgroup‑операций — capability/limits‑пробы на старте, graceful fallback
- Синхронизация compute‑пасс — строгие барьеры и валидация через VK‑Layers
- Апскейлинг SDK (лицензии/доставка DLL/SO) — опционально, фича‑флаг, документация по установке

---

## 12. Критерии приёмки

- Демо сцены рендерятся Hybrid DWT + FreGS со стабильным FPS и без критических артефактов
- Тесты ≥80% покрытия, CI зелёный на целевых платформах
- Docs централизованы, дубли устранены, ссылки корректны
- Линтеры/статанализ без регрессий высокого приоритета

---

## 13. Ссылки (документация и интеграции)

- Vulkan Subgroups (Vulkan 1.3): см. `khronosgroup/vulkan-docs`
- CUDA/Vulkan interop (timeline semaphores, external memory): `docs.nvidia.com/cuda` (External Semaphores)
- VMA (VulkanMemoryAllocator): дефраг, пулы
- VkFFT (legacy/исслед.): `github.com/DTolm/VkFFT`
- Streamline DLSS: `github.com/NVIDIA-RTX/Streamline`, пример Vulkan: `github.com/nvpro-samples/vk_streamline`
- FidelityFX FSR2: `github.com/GPUOpen-Effects/FidelityFX-FSR2`, обзор `gpuopen.com/fidelityfx-superresolution-2/`

---

## 14. Приложение A — Таблица миграции файлов (пример)

| Текущее расположение | Действие | Новое расположение |
|---|---|---|
| docs/concept/Renderer.md | Переместить | docs/architecture/Renderer.md |
| FREQVOX_*.md (корень) | Переместить | docs/legacy/freqvox/ |
| VULKAN_*FIX*.md, ИСПРАВЛЕНИЕ_VULKAN_RU.md | Переместить | docs/reports/ |
| BUILD_*.md, QUICK_INSTALL.md, INSTALL_GUIDE_RU.md | Переместить/объединить | docs/guides/ |
| examples/freqvox_* | Пометить LEGACY / перенести | examples/legacy/ или удалить после валидации |
| test_glfw_vulkan_hpp_conflict.cpp | Проверить актуальность | Удалить/архивировать |

---

## 15. Приложение B — Политики качества

- SOLID: SRP/OCP/LSP/ISP/DIP — проверка при ревью
- Док‑комментарии Doxygen для публичных API
- Консольный вывод: SAFE_TO_STRING‑макросы
- Соответствие clang‑format/clang‑tidy/cppcheck

\
Документ подготовлен как основа для PR с поэтапным внедрением. Все удаления/перемещения — через отдельные «mechanical» PR‑ы с чистыми диффами.


