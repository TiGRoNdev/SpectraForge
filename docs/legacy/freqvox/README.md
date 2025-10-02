# FreqVox Legacy Documentation

⚠️ **ВНИМАНИЕ:** Данная документация относится к устаревшему рендереру FreqVox (Frequency Voxel Renderer), основанному на FFT/DCT подходе. Проект был переведен на новую архитектуру **Hybrid DWT + FreGS** (Wavelet Lifting + Frequency-Encoded Gaussian Splatting).

## Статус: LEGACY / Исследовательские материалы

Эти документы сохранены для:
- Исторической справки
- Сравнения производительности с новым пайплайном
- Исследовательских целей
- Возможной активации через feature-флаг `SPECTRAFORGE_RENDERER=FREQVOX`

## Новая архитектура

Текущий рендерер использует **Hybrid DWT + FreGS**:
- **Wavelet Lifting** (Daubechies-4) вместо FFT/DCT - O(N) вместо O(N log N)
- **Frequency-Encoded Gaussian Splatting** с аналитическими ядрами
- Vulkan 1.3 Subgroup Operations без внешних библиотек
- Производительность: ~500 FPS @ 8K на мобильных GPU (≤5W)

См. актуальную документацию: `docs/architecture/Renderer.md`

## Содержание Legacy-документации

### Математические основы
- `FreqVox Renderer Math.md` - Математическая модель FFT/DCT-подхода
- `FreqVox Renderer.md` - Описание архитектуры FreqVox
- `FREQVOX_MATHEMATICAL_ANALYSIS.md` - Глубокий математический анализ
- `FREQVOX_MATH_AUDIT_REPORT.md` - Аудит математической корректности

### Реализация
- `FREQVOX_IMPL.md` - Общее описание реализации
- `FREQVOX_VKFFT_IMPLEMENTATION.md` - Интеграция VkFFT
- `FREQVOX_DCT_IMPLEMENTATION.md` - DCT-конвейер
- `FREQVOX_COMPUTE_IMPLEMENTATION.md` - Compute-шейдеры
- `FREQVOX_VULKAN_IMPLEMENTATION.md` - Vulkan-специфика

### Производительность и оптимизации
- `FREQVOX_PERFORMANCE.md` - Отчеты по производительности
- `FREQVOX_FOVEATION.md` - Фовеация в частотной области
- `FREQVOX_FREQUENCY_CONVOLUTION.md` - Частотная свертка
- `FREQVOX_SPHERICAL_HARMONICS.md` - Сферические гармоники

### Интеграция и демо
- `FREQVOX_HARDWARE_DETECTION_UPDATE.md` - Детекция аппаратуры
- `FREQVOX_SPONZA_IMPLEMENTATION.md` - Sponza демо-сцена
- `FREQVOX_INTEGRATION.md` - Интеграция с движком
- `FREQVOX_SPONZA_DEMO.md` - Инструкции для демо

### Анализ и планирование
- `FREQVOX_COMPARATIVE_ANALYSIS.md` - Сравнение с другими подходами
- `FREQVOX_FINAL_REPORT.md` - Финальный отчет
- `FREQVOX_IMPLEMENTATION_SUMMARY.md` - Сводка реализации
- `FREQVOX_REFACTORING_PLAN.md` - План рефакторинга (устарел)
- `FREQVOX_BUGFIX_PLAN.md` - План исправлений
- `FREQVOX_QUICKSTART.md` - Быстрый старт

## Сравнение: FreqVox vs Hybrid DWT + FreGS

| Аспект | FreqVox (AFS-NVR) | Hybrid DWT + FreGS |
|--------|-------------------|-------------------|
| Transform | O(PQ log(PQ)) FFT/DCT | O(N) Wavelet Lifting |
| Splat/Shade | O(M log M + P) DCT-конволюция | O(M+P) аналитические Гауссианы |
| Memory | Плотные SH + DCT буферы | Разреженные wavelet subbands fp16 |
| FPS @ 8K | ~300 FPS | ~500 FPS |
| Энергия (Adreno 650) | ~4.2W | ~4.8W |
| Зависимости | VkFFT, нейронный апскейл | Только Vulkan 1.3 Subgroups |

## Миграция с FreqVox на Hybrid DWT + FreGS

Если у вас есть код, использующий FreqVox:

1. **Удалите зависимости от VkFFT** - больше не нужны
2. **Замените FFT/DCT пасс** на `WaveletLifting.comp`
3. **Замените частотную свертку** на `GaussFreqSplat.comp`
4. **Обновите дескрипторы** - теперь только wavelet subbands
5. **Включите Vulkan 1.3** с `shaderSubgroupExtendedTypes`

См. миграционный гайд: `docs/guides/FreqVox_to_HybridDWT_Migration.md` (TODO)

## Активация Legacy-режима (экспериментально)

```bash
cmake -DSPECTRAFORGE_RENDERER=FREQVOX ..
```

⚠️ **Предупреждение:** Legacy-режим не поддерживается активно и может иметь проблемы совместимости с новыми версиями зависимостей.

---

**Дата архивирования:** 2025-10-02  
**Причина:** Переход на более эффективную архитектуру Hybrid DWT + FreGS  
**Статус:** Read-Only, исследовательские материалы

