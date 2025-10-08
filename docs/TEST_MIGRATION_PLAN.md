# 🧪 ПЛАН МИГРАЦИИ ТЕСТОВ SpectraForge

**Дата создания**: 2025-10-08  
**Версия**: 1.0.0  
**Статус**: 🔄 АКТИВЕН - Фаза 0, Задача 0.3  
**Владелец**: Команда разработки SpectraForge

---

## 📊 ИСПОЛНИТЕЛЬНОЕ РЕЗЮМЕ

```yaml
Текущее покрытие:   80% (633/788 функций)
Целевое покрытие:   100% (788/788 функций)
Недостает функций:  155 функций
Оценка тестов:      225-255 новых тестов
Время выполнения:   4-5 недель (Фаза 3 рефакторинга)
Критичность:        ⚠️ ВЫСОКАЯ
```

---

## 🎯 ЦЕЛИ И МЕТРИКИ

### Основные цели
1. ✅ Достичь 100% покрытия non-legacy кода
2. ✅ Обеспечить качество через TDD подход
3. ✅ Стандартизировать структуру тестов
4. ✅ Ускорить CI/CD через быстрые тесты

### Ключевые метрики

| Метрика | Сейчас | Цель | Дельта |
|---------|--------|------|--------|
| Line Coverage | 80% | 100% | +20% |
| Function Coverage | 80% (633/788) | 100% (788/788) | +155 функций |
| Branch Coverage | ~75% | >95% | +20% |
| Новых тестов | 0 | 225-255 | +225-255 |
| Всего тестов | ~450 | 675-705 | +50% |

---

## 📋 ДЕТАЛЬНЫЙ ПЛАН ПО ФАЗАМ

---

## 🔴 ФАЗА 1: КРИТИЧЕСКИЙ ПРИОРИТЕТ (Недели 1-2)

### Sprint 3.1: HybridFreGSRenderer (44 функции → 60-80 тестов)

**Время**: 1 неделя  
**Приоритет**: 🔴🔴🔴 МАКСИМАЛЬНЫЙ  
**Файл**: `tests/hybrid_fregs_renderer_complete_test.cpp`

#### День 1-2: Инфраструктура и базовые тесты (20 тестов)

**Задачи**:
1. Создать Mock Vulkan API
2. Создать Test Fixture
3. Написать тесты lifecycle

**Функции для покрытия**:
```cpp
// Lifecycle (5 функций)
✓ initialize()                  // 3 теста: success, fail, already_initialized
✓ shutdown()                    // 2 теста: normal, double_shutdown
✓ ~HybridFreGSRenderer()        // 2 теста: destructor_cleanup, destructor_with_resources
✓ createInstance()              // 3 теста: success, validation_enabled, fail
✓ createLogicalDevice()         // 4 теста: success, fail, queue_selection, extensions
```

**Пример тестов**:
```cpp
TEST_F(HybridFreGSRendererTest, Initialize_Success) {
    // Arrange
    setupMockVulkan();
    
    // Act
    bool result = renderer->initialize();
    
    // Assert
    EXPECT_TRUE(result);
    EXPECT_TRUE(renderer->isInitialized());
}

TEST_F(HybridFreGSRendererTest, Initialize_AlreadyInitialized) {
    // Arrange
    renderer->initialize();
    
    // Act & Assert
    EXPECT_FALSE(renderer->initialize()); // Должен вернуть false
}
```

**Deliverables**:
- [ ] Mock Vulkan API classes (MockInstance, MockDevice, MockSwapchain)
- [ ] Test Fixture: `HybridFreGSRendererTest`
- [ ] 14 базовых тестов lifecycle

---

#### День 3-4: Vulkan интеграция (25 тестов)

**Функции для покрытия**:
```cpp
// Vulkan Setup (10 функций)
✓ pickPhysicalDevice()          // 4 теста: dedicated_gpu, integrated_gpu, no_suitable, multiple_gpus
✓ attachWindow()                // 4 теста: success, null_display, null_window, already_attached
✓ createSurfaceX11()            // 3 теста: success, fail, wrong_platform
✓ createSwapchainAndViews()     // 5 теста: success, recreate, optimal_format, fallback_format, fail
✓ createRenderPass()            // 3 теста: success, fail, custom_config
✓ createFramebuffers()          // 2 теста: success, fail
✓ createCommandPool()           // 2 теста: success, fail
✓ createCommandBuffers()        // 2 теста: success, fail
✓ createSyncObjects()           // 2 теста: success, fail
✓ createAllocator()             // 2 теста: success, fail
```

**Deliverables**:
- [ ] 25 тестов Vulkan инициализации
- [ ] Mock X11 Display/Window

---

#### День 5-6: Рендеринг pipeline (25 тестов)

**Функции для покрытия**:
```cpp
// Rendering Core (8 функций)
✓ beginFrame()                  // 4 теста: success, image_acquire_fail, fence_timeout, device_lost
✓ recordCommandBuffer()         // 6 теста: triangle_mode, gaussian_mode, empty_scene, multiple_passes, debug_mode, fail
✓ renderFrame()                 // 5 теста: full_frame, empty_scene, device_lost, swapchain_invalid, success
✓ endFrame()                    // 4 теста: success, present_fail, queue_submit_fail, fence_error

// Upload Functions (3 функции)
✓ uploadGaussians()             // 3 теста: success, empty_data, large_data
✓ uploadTriangles()             // 3 теста: success, empty_data, large_data
✓ uploadMesh()                  // 3 теста: success, invalid_mesh, large_mesh

// Mesh Conversion (2 функции)
✓ initializeTriangleSplatting() // 2 теста: success, fail
✓ convertMeshToTriangles()      // 4 теста: simple_mesh, complex_mesh, invalid_mesh, empty_mesh
```

**Deliverables**:
- [ ] 34 теста рендеринга
- [ ] Mock FrameData, Mesh3D

---

#### День 7: Debug и статистика (16 тестов)

**Функции для покрытия**:
```cpp
// Debug API (11 функций)
✓ setDebugMode()                // 3 теста: mode_0, mode_1, mode_2
✓ getDebugMode()                // 1 тест
✓ enableWireframe()             // 2 теста: enable, disable
✓ enableBackfaceCulling()       // 2 теста: enable, disable
✓ enableDepthTest()             // 2 теста: enable, disable
✓ setBackgroundColor()          // 2 теста: valid_color, edge_values
✓ getBackgroundColor()          // 1 тест
✓ setViewport()                 // 2 теста: valid, invalid
✓ enableAlphaBlending()         // 2 теста: enable, disable
✓ setTriangleBudget()           // 2 теста: valid, invalid
✓ enableEarlyTermination()      // 2 теста: enable, disable

// Statistics (4 функции)
✓ getDetailedStats()            // 2 теста: normal, after_render
✓ getGPUInfo()                  // 2 теста: valid_gpu, mock_gpu
✓ saveScreenshot()              // 3 теста: success, fail, invalid_path
✓ getFramebufferData()          // 2 теста: success, fail

// Utility (3 функции)
✓ supportsFeature()             // 2 теста: supported, not_supported
✓ setDebugCallback()            // 1 тест
✓ flushUniforms()               // 1 тест
✓ getCorrectedViewProjMatrix()  // 1 тест
✓ destroySwapchainAndViews()    // 2 теста: normal, null_swapchain
```

**Deliverables**:
- [ ] 35 тестов debug и статистики
- [ ] Полное покрытие HybridFreGSRenderer: 80 тестов

---

## ⚠️ ФАЗА 2: ВЫСОКИЙ ПРИОРИТЕТ (Неделя 3)

### Sprint 3.2: ModernRenderer3D (21 функция → 30-40 тестов)

**Время**: 3 дня  
**Приоритет**: ⚠️⚠️ ВЫСОКИЙ  
**Файл**: `tests/modern_renderer3d_complete_test.cpp`

#### День 1: Core функциональность (15 тестов)

**Функции**:
```cpp
✓ ModernRenderer3D(strategy)    // 3 теста: valid_strategy, null_strategy, different_strategies
✓ initialize()                  // 3 теста: success, fail, reinitialize
✓ renderFrame(frameData)        // 4 теста: normal, empty_data, multiple_frames, error_handling
✓ shutdown()                    // 2 теста: normal, already_shutdown
✓ isInitialized()               // 1 тест
✓ isReady()                     // 2 теста: ready, not_ready
```

**Deliverables**:
- [ ] Test Fixture: `ModernRenderer3DTest`
- [ ] Mock IRenderStrategy
- [ ] 15 core тестов

---

#### День 2: Configuration и features (13 тестов)

**Функции**:
```cpp
✓ setConfigParameter()          // 3 теста: valid, invalid, override
✓ getConfigParameter()          // 2 теста: exists, not_exists
✓ hasConfigParameter()          // 2 теста: exists, not_exists
✓ loadConfig()                  // 2 теста: valid_file, invalid_file
✓ saveConfig()                  // 2 теста: success, fail
✓ supportsFeature()             // 2 теста: supported, not_supported
```

**Deliverables**:
- [ ] 13 тестов конфигурации

---

#### День 3: Strategy и post-processing (12 тестов)

**Функции**:
```cpp
✓ setRenderStrategy()           // 4 теста: change_strategy, null_strategy, same_strategy, runtime_change
✓ addPostProcessEffect()        // 3 теста: single, multiple, duplicate
✓ removePostProcessEffect()     // 2 теста: exists, not_exists
✓ applyPostProcessEffects()     // 3 теста: no_effects, single, multiple

// Getters
✓ getType()                     // 1 тест
✓ getName()                     // 1 тест
✓ getApiVersion()               // 1 тест
✓ getStats()                    // 2 теста
✓ initializeGraphicsAPI()       // 3 теста
```

**Deliverables**:
- [ ] 17 тестов стратегий и эффектов
- [ ] Полное покрытие ModernRenderer3D: 45 тестов

---

### Sprint 3.2: FreGSPass (12 функций → 20-25 тестов)

**Время**: 2 дня  
**Файл**: `tests/fregs_pass_complete_test.cpp`

**Функции для покрытия**:
```cpp
✓ FreGSPass(config)             // 2 теста: valid_config, invalid_config
✓ initialize(context)           // 3 теста: success, fail, reinitialize
✓ execute(cmd, imageIndex)      // 5 тестов: normal, empty_gaussians, with_subbands, error, multiple_calls
✓ cleanup()                     // 2 теста: normal, double_cleanup
✓ setInputSubbands(subbands)    // 3 теста: valid, null, invalid
✓ uploadGaussians(vector)       // 4 теста: small, large, empty, invalid
✓ updateFoveation(point, radius)// 3 теста: valid, edge_cases, invalid
✓ createOutputImage(context)    // 2 теста: success, fail
✓ createGaussianBuffer(context) // 2 теста: success, fail
✓ createDescriptorSets(context) // 3 теста: success, fail, update
✓ loadShaderSPIRV()             // 2 теста: success, file_not_found
✓ updateViewProjection(mat4)    // 2 теста: valid, identity
```

**Deliverables**:
- [ ] Test Fixture: `FreGSPassTest`
- [ ] Mock WaveletSubbands, GaussianSplat
- [ ] 33 теста FreGSPass

---

### Sprint 3.2: WaveletPass (10 функций → 15-20 тестов)

**Время**: 2 дня  
**Файл**: `tests/wavelet_pass_complete_test.cpp`

**Функции для покрытия**:
```cpp
✓ WaveletPass(config)           // 2 теста
✓ initialize(context)           // 3 теста
✓ execute(cmd, imageIndex)      // 4 теста: normal, no_input, multiple_levels, error
✓ cleanup()                     // 2 теста
✓ setInputImage(image, view)    // 3 теста: valid, null, invalid
✓ updateConfig(config)          // 2 теста: valid, invalid
✓ createSubbandImages(context)  // 3 теста: success, fail, multiple_levels
✓ createDescriptorSets(context) // 3 теста
✓ loadShaderSPIRV()             // 2 теста
✓ ensureInputImage(context)     // 2 теста: exists, create_new
```

**Deliverables**:
- [ ] Test Fixture: `WaveletPassTest`
- [ ] Mock Vulkan images
- [ ] 26 тестов WaveletPass

---

**Итого Фаза 2**: 104 теста, 43 функции покрыто

---

## ⚡ ФАЗА 3: СРЕДНИЙ ПРИОРИТЕТ (Неделя 4)

### Sprint 3.3: Engine.cpp (26 функций → 40-50 тестов)

**Время**: 4 дня  
**Приоритет**: ⚡⚡ СРЕДНИЙ  
**Файл**: `tests/app_engine_complete_test.cpp`

#### Категории тестов:

**1. Lifecycle (10 тестов)**
```cpp
✓ Engine(config, logger)        // 2 теста
✓ Engine(config, ...)           // 2 теста
✓ ~Engine()                     // 2 теста
✓ init()                        // 4 теста: success, fail, reinit, partial_init
```

**2. Core Loop (12 тестов)**
```cpp
✓ update(deltaTime)             // 4 теста: normal, zero_dt, negative_dt, large_dt
✓ render()                      // 4 теста: normal, no_scene, device_lost, error
✓ load_scene(sceneData)         // 4 теста: valid_scene, empty, large, invalid
```

**3. Getters (8 тестов)**
```cpp
✓ getSceneInfo()                // 1 тест
✓ getCamera()                   // 2 теста: exists, null
✓ getRenderer()                 // 2 теста
✓ getInputManager()             // 1 тест
✓ getRenderStats()              // 2 теста
```

**4. Configuration (12 тестов)**
```cpp
✓ setExternalCameraControl()    // 2 теста
✓ setDebugMode()                // 3 теста: 0, 1, 2
✓ getDebugMode()                // 1 тест
✓ enableWireframe()             // 2 теста
✓ setBackgroundColor()          // 2 теста
✓ resetCameraForSponza()        // 2 теста
```

**5. Input Handling (8 тестов)**
```cpp
✓ isKeyPressed(key)             // 2 теста
✓ setupCallbacks()              // 1 тест
✓ keyCallback()                 // 2 теста
✓ mouseCallback()               // 2 теста
✓ mouseButtonCallback()         // 1 тест
```

**6. Utilities (10 тестов)**
```cpp
✓ shutdown()                    // 2 теста
✓ logDebugInfo()                // 2 теста
✓ updateSceneInfo()             // 2 теста
✓ updateRenderStats()           // 2 теста
```

**Deliverables**:
- [ ] 60 тестов Engine
- [ ] Mock Window, Renderer, Input

---

### Sprint 3.3: X11Window (22 функции → 30-40 тестов)

**Время**: 3 дня  
**Файл**: `tests/x11_window_complete_test.cpp`

**Категории**:

**1. Lifecycle (8 тестов)**
```cpp
✓ X11Window()                   // 1 тест
✓ ~X11Window()                  // 2 теста
✓ create(w, h, title)           // 3 теста: success, fail, duplicate
✓ close()                       // 2 теста
```

**2. Window State (10 тестов)**
```cpp
✓ isOpen()                      // 2 теста
✓ getSize()                     // 1 тест
✓ setSize()                     // 2 теста: valid, invalid
✓ getTitle()                    // 1 тест
✓ setTitle()                    // 2 теста
✓ getNativeHandle()             // 2 теста
```

**3. Fullscreen (6 тестов)**
```cpp
✓ isFullscreenSupported()       // 1 тест
✓ setFullscreen()               // 3 теста: enable, disable, not_supported
✓ isFullscreen()                // 2 теста
```

**4. Events (16 тестов)**
```cpp
✓ update()                      // 3 теста: no_events, with_events, error
✓ processEvent(XEvent)          // 5 теста: key, mouse, resize, close, unknown
✓ setWindowEventCallback()      // 1 тест
✓ setMouseEventCallback()       // 1 тест
✓ setKeyEventCallback()         // 1 тест
✓ sendWindowEvent()             // 2 теста
✓ sendMouseEvent()              // 2 теста
✓ sendKeyEvent()                // 2 теста
✓ convertKeyCode()              // 3 теста: normal, special, unknown
✓ convertMouseButton()          // 2 теста: valid, invalid
```

**Deliverables**:
- [ ] 40 тестов X11Window
- [ ] Mock X11 API

---

**Итого Фаза 3**: 100 тестов, 48 функций покрыто

---

## 🔧 ФАЗА 4: ДОПОЛНИТЕЛЬНЫЕ КОМПОНЕНТЫ (Неделя 5)

### Остальные компоненты (20 функций → 30-40 тестов)

**RenderPass.cpp (5 функций → 8-10 тестов)**
```cpp
✓ RenderPass()
✓ initialize()
✓ execute()
✓ cleanup()
✓ isInitialized()
```

**FreGSRenderStrategy.cpp (7 функций → 10-15 тестов)**
```cpp
✓ FreGSRenderStrategy()
✓ execute()
✓ setConfig()
✓ getRequiredFeatures()
✓ getName()
✓ isSupported()
✓ createResources()
```

**InstancedMeshPass.cpp (8 функций → 12-15 тестов)**
```cpp
✓ InstancedMeshPass()
✓ initialize()
✓ execute()
✓ cleanup()
✓ uploadInstanceData()
✓ setMesh()
✓ setTransforms()
✓ updateUniforms()
```

**Deliverables**:
- [ ] 30-40 дополнительных тестов

---

## 📊 ИТОГОВАЯ СТАТИСТИКА

```
┌─────────────────────────────────────────────────────────┐
│ ПОЛНЫЙ ОХВАТ ПЛАНА МИГРАЦИИ ТЕСТОВ                      │
├─────────────────────────────────────────────────────────┤
│ Фаза 1 (Критическая):     80 тестов,  44 функции       │
│ Фаза 2 (Высокая):        104 теста,   43 функции       │
│ Фаза 3 (Средняя):        100 тестов,  48 функций       │
│ Фаза 4 (Дополнительно):   40 тестов,  20 функций       │
├─────────────────────────────────────────────────────────┤
│ ИТОГО:                   324 теста,  155 функций       │
│                                                         │
│ Текущие тесты:           ~450 тестов                   │
│ После миграции:          ~774 теста                    │
│                                                         │
│ Покрытие:                100% (788/788) ✅              │
└─────────────────────────────────────────────────────────┘
```

---

## 🎯 СТАНДАРТЫ И ШАБЛОНЫ

### Обязательные требования

1. **AAA Pattern**: Arrange → Act → Assert
2. **Test Fixtures**: Для каждого класса
3. **Naming**: `<Component><Action>_<Scenario>`
4. **Mocking**: Mock все внешние зависимости
5. **Coverage**: Каждая функция ≥ 1 тест
6. **Documentation**: Doxygen комментарии для тестов

### Структура теста

```cpp
/**
 * @brief Тест успешной инициализации компонента
 * 
 * @test Проверяет, что компонент корректно инициализируется
 *       с валидными параметрами
 */
TEST_F(ComponentTest, Initialize_Success) {
    // Arrange - Подготовка
    ComponentConfig config = createValidConfig();
    
    // Act - Действие
    bool result = component->initialize(config);
    
    // Assert - Проверка
    EXPECT_TRUE(result);
    EXPECT_TRUE(component->isInitialized());
}
```

---

## 🚀 EXECUTION PLAN

### Неделя 1: Критический приоритет (Фаза 1)
```
День 1-2:  HybridFreGSRenderer - Infrastructure (20 тестов)
День 3-4:  HybridFreGSRenderer - Vulkan (25 тестов)
День 5-6:  HybridFreGSRenderer - Rendering (25 тестов)
День 7:    HybridFreGSRenderer - Debug (10 тестов)

Checkpoint: 80 тестов, 44 функции ✅
```

### Неделя 2-3: Высокий приоритет (Фаза 2)
```
День 8-10:  ModernRenderer3D (45 тестов)
День 11-12: FreGSPass (33 теста)
День 13-14: WaveletPass (26 тестов)

Checkpoint: 184 теста, 87 функций ✅
```

### Неделя 4: Средний приоритет (Фаза 3)
```
День 15-18: Engine (60 тестов)
День 19-21: X11Window (40 тестов)

Checkpoint: 284 теста, 135 функций ✅
```

### Неделя 5: Финализация (Фаза 4)
```
День 22-24: Остальные компоненты (40 тестов)
День 25:    Code review и рефакторинг
День 26:    Финальная валидация

Checkpoint: 324 теста, 155 функций ✅
```

---

## ✅ КРИТЕРИИ УСПЕХА

### Обязательные метрики
- [ ] ✅ 100% function coverage (788/788)
- [ ] ✅ >95% line coverage
- [ ] ✅ >90% branch coverage
- [ ] ✅ Все тесты GREEN
- [ ] ✅ CI/CD проходит (<5 мин)

### Качество тестов
- [ ] ✅ Все тесты следуют AAA pattern
- [ ] ✅ Test Fixtures для всех классов
- [ ] ✅ Mock объекты для зависимостей
- [ ] ✅ Doxygen комментарии

### Документация
- [ ] ✅ README тестов обновлен
- [ ] ✅ Шаблоны созданы
- [ ] ✅ Примеры для каждого типа

---

## 🔗 СВЯЗАННЫЕ ДОКУМЕНТЫ

- `REFACTORING_PLAN.md` - Общий план рефакторинга
- `MISSING_TESTS_MATRIX.md` - Матрица недостающих тестов
- `TEST_TEMPLATES.md` - Шаблоны тестов
- `QUICK_TEST_GUIDE.md` - Быстрое руководство

---

## 📝 ПРИМЕЧАНИЯ

1. **Мокирование Vulkan**: Использовать VMA mocks и vk::DynamicLoader
2. **X11 тесты**: Требуют Mock X11 Display (XLib)
3. **Integration тесты**: Минимум 10-15 E2E сценариев
4. **Performance**: Каждый unit-тест < 10ms
5. **Isolation**: Тесты должны быть независимыми

---

**Статус**: 🔄 ГОТОВ К ИСПОЛНЕНИЮ  
**Следующий документ**: `TEST_TEMPLATES.md`  
**Владелец**: Команда SpectraForge  
**Версия**: 1.0.0

