SpectraForge API Release Plan (based on IR_C)
Date: 2025-10-03
Version: 0.1.0

Scope
 - Ввод высокоуровневого App-фасада (`SpectraForge::App::Engine`, `IApp`, `AppConfig`)
 - Стабилизация пайплайна гибридного рендеринга (GS → OptiX → Denoise → Upscale → Present)
 - Безопасная синхронизация Vulkan/CUDA/OptiX (timeline semaphores, external memory/semaphores)
 - Апскейлинг через Streamline DLSS и FSR2 за фабрикой
 - Тесты и документация API

Milestones
 M1: Facade & Wiring (2 недели)
  - Создать заголовки: `include/SpectraForge/App/{Engine.h,IApp.h,Config.h}`
  - Реализовать фасад поверх `Core::EngineCore` (инициализация, update, render, shutdown)
  - Адаптеры: сцена (`load_scene` → `Vulkan::SceneManager`), окно/заголовок, логирование
  - Демонстрация: адаптация PSEUDO-дев примера под новый фасад

 M2: Interop & Sync Hardening (2 недели)
  - Ввести опциональный путь submit2 + timeline семафоры
  - Обернуть CUDA/OptiX interop (external memory + semaphores) единым интерфейсом
  - Проверки корректности layout/barrier по Vulkan spec (validation layers CI)

 M3: Upscaling Providers (2-3 недели)
  - Интеграция Streamline (DLSS SR, RR, FG поэтапно; fallback при недоступности)
  - Интеграция FSR2 (качество/баланс/перфоманс режимы, reactive/transparency masks)
  - Завязка выбора на `HardwareDetector`/`UpscalerFactory`

 M4: Tests & Docs (1-2 недели)
  - Unit: фасад (80%+), mocks для renderer/scene/window
  - Integration: загрузка/кадр с заглушкой рендера, upscaling stub
  - Документация: API guide, Quickstart, IR_* публикация, Doxygen

Deliverables
 - Публичные заголовки `SpectraForge::App` в `include/`
 - Обновленные демо (минимум один пример на новый API)
 - CI: сборка + validation layers + тесты (ctest), покрытие 80%+ для нового кода
 - Документация в `docs/api` и `docs/ir` (IR_1, IR_2, IR_C)

Risk & Mitigation
 - Различия драйверов/вендоров: добавить capability registry и feature gates
 - Streamline/FSR2 зависимости: опциональные сборочные флаги, graceful fallback
 - Сложность синхронизации: validation layers, Nsight/RenderDoc маркеры, тест-кейсы

Backward Compatibility
 - Существующие подсистемы и демо сохраняются; фасад добавляется как тонкий слой
 - Никаких breaking изменений в `Core::EngineCore`/`VulkanRenderer` на этом этапе

Acceptance Criteria
 - Новый фасад позволяет запустить демо: init → load_scene → update/render → shutdown
 - На поддерживаемом железе включается соответствующий апскейлер; иначе fallback
 - Валидаторы Vulkan без критических ошибок; тесты 80%+ покрытия по новому коду

Post-Release
 - План на редактор, ECS слой, расширенную сцену, многопоточность загрузки
 - Профилирование и оптимизация горячих участков (allocations, descriptor updates)


