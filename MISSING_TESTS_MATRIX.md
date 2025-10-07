# 🔴 МАТРИЦА ФУНКЦИЙ БЕЗ ТЕСТОВ - SpectraForge

**Дата:** 2025-10-07  
**Статус:** Требуют покрытия тестами  
**Приоритет:** По убыванию критичности

---

## 📊 СВОДНАЯ СТАТИСТИКА

```
Всего функций без тестов:        203
Критический приоритет:            44 (22%)
Высокий приоритет:                43 (21%)
Средний приоритет:                20 (10%)
Низкий приоритет:                 48 (24%)
Legacy (не требует):              48 (23%)
```

---

## ⚠️⚠️⚠️ КРИТИЧЕСКИЙ ПРИОРИТЕТ (44 функции)

### HybridFreGSRenderer.cpp

**Файл:** `src/rendering/HybridFreGSRenderer.cpp` (1,383 строки)  
**Критичность:** ⚠️⚠️⚠️ МАКСИМАЛЬНАЯ  
**Функций без тестов:** 44  
**Ожидаемых тестов:** 60-80

| № | Функция | Строк | Описание | Приоритет |
|---|---------|-------|----------|-----------|
| 1 | `initialize()` | ~15 | Инициализация рендерера | ⚠️⚠️⚠️ |
| 2 | `createInstance()` | ~35 | Создание Vulkan instance | ⚠️⚠️⚠️ |
| 3 | `pickPhysicalDevice()` | ~25 | Выбор физического GPU | ⚠️⚠️⚠️ |
| 4 | `createLogicalDevice()` | ~70 | Создание логического устройства | ⚠️⚠️⚠️ |
| 5 | `attachWindow()` | ~75 | Привязка окна к рендереру | ⚠️⚠️⚠️ |
| 6 | `createSurfaceX11()` | ~20 | Создание X11 surface | ⚠️⚠️ |
| 7 | `createSwapchainAndViews()` | ~100 | Создание swapchain | ⚠️⚠️⚠️ |
| 8 | `createRenderPass()` | ~45 | Создание render pass | ⚠️⚠️⚠️ |
| 9 | `createFramebuffers()` | ~25 | Создание framebuffers | ⚠️⚠️⚠️ |
| 10 | `createCommandPool()` | ~15 | Создание command pool | ⚠️⚠️ |
| 11 | `createCommandBuffers()` | ~18 | Создание command buffers | ⚠️⚠️⚠️ |
| 12 | `createSyncObjects()` | ~23 | Создание sync объектов | ⚠️⚠️ |
| 13 | `beginFrame()` | ~24 | Начало кадра | ⚠️⚠️⚠️ |
| 14 | `recordCommandBuffer()` | ~250 | Запись команд рендеринга | ⚠️⚠️⚠️ |
| 15 | `renderFrame()` | ~32 | Рендеринг кадра | ⚠️⚠️⚠️ |
| 16 | `endFrame()` | ~50 | Завершение кадра | ⚠️⚠️⚠️ |
| 17 | `shutdown()` | ~62 | Очистка ресурсов | ⚠️⚠️⚠️ |
| 18 | `destroySwapchainAndViews()` | ~12 | Уничтожение swapchain | ⚠️⚠️ |
| 19 | `supportsFeature()` | ~15 | Проверка возможностей | ⚠️⚠️ |
| 20 | `uploadGaussians()` | ~5 | Загрузка гауссианов | ⚠️⚠️⚠️ |
| 21 | `uploadTriangles()` | ~9 | Загрузка треугольников | ⚠️⚠️⚠️ |
| 22 | `createAllocator()` | ~17 | Создание VMA allocator | ⚠️⚠️ |
| 23 | `initializeTriangleSplatting()` | ~27 | Инициализация triangle splatting | ⚠️⚠️⚠️ |
| 24 | `convertMeshToTriangles()` | ~57 | Конвертация меша | ⚠️⚠️ |
| 25 | `uploadMesh()` | ~25 | Загрузка меша | ⚠️⚠️⚠️ |
| 26 | `setDebugMode()` | ~36 | Установка debug режима | ⚠️ |
| 27 | `getDebugMode()` | ~4 | Получение debug режима | ⚠️ |
| 28 | `enableWireframe()` | ~10 | Включение wireframe | ⚠️ |
| 29 | `enableBackfaceCulling()` | ~11 | Включение culling | ⚠️ |
| 30 | `enableDepthTest()` | ~7 | Включение depth test | ⚠️ |
| 31 | `setBackgroundColor()` | ~12 | Установка цвета фона | ⚠️ |
| 32 | `getBackgroundColor()` | ~4 | Получение цвета фона | ⚠️ |
| 33 | `setViewport()` | ~14 | Установка viewport | ⚠️ |
| 34 | `enableAlphaBlending()` | ~10 | Включение alpha blending | ⚠️ |
| 35 | `setTriangleBudget()` | ~9 | Установка бюджета треугольников | ⚠️ |
| 36 | `enableEarlyTermination()` | ~10 | Early Z termination | ⚠️ |
| 37 | `getDetailedStats()` | ~49 | Получение детальной статистики | ⚠️⚠️ |
| 38 | `saveScreenshot()` | ~19 | Сохранение screenshot | ⚠️ |
| 39 | `getFramebufferData()` | ~11 | Получение данных framebuffer | ⚠️ |
| 40 | `setDebugCallback()` | ~5 | Установка debug callback | ⚠️ |
| 41 | `flushUniforms()` | ~7 | Flush uniform данных | ⚠️ |
| 42 | `getGPUInfo()` | ~49 | Получение информации о GPU | ⚠️⚠️ |
| 43 | `getCorrectedViewProjMatrix()` | ~3 | Корректировка матрицы | ⚠️ |
| 44 | `~HybridFreGSRenderer()` | ~3 | Деструктор | ⚠️⚠️⚠️ |

**Тестовый файл:** `tests/hybrid_fregs_renderer_test.cpp` (НУЖНО СОЗДАТЬ)

**Пример структуры тестов:**
```cpp
class HybridFreGSRendererTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Mock Vulkan instance
        // Mock X11 display/window
    }
    
    std::unique_ptr<HybridFreGSRenderer> renderer;
};

TEST_F(HybridFreGSRendererTest, InitializeTest) { /* ... */ }
TEST_F(HybridFreGSRendererTest, CreateInstanceTest) { /* ... */ }
TEST_F(HybridFreGSRendererTest, RenderFrameTest) { /* ... */ }
// И т.д. для всех 44 функций
```

---

## ⚠️⚠️ ВЫСОКИЙ ПРИОРИТЕТ (43 функции)

### 1. ModernRenderer3D.cpp

**Файл:** `src/rendering/ModernRenderer3D.cpp` (350 строк)  
**Критичность:** ⚠️⚠️ ВЫСОКАЯ  
**Функций без тестов:** 21  
**Ожидаемых тестов:** 30-40

| № | Функция | Описание | Приоритет |
|---|---------|----------|-----------|
| 1 | `ModernRenderer3D(const std::shared_ptr<IRenderStrategy>&)` | Конструктор | ⚠️⚠️ |
| 2 | `initialize()` | Инициализация | ⚠️⚠️⚠️ |
| 3 | `renderFrame(const FrameData&)` | Рендеринг кадра | ⚠️⚠️⚠️ |
| 4 | `shutdown()` | Завершение работы | ⚠️⚠️ |
| 5 | `getType()` | Тип рендерера | ⚠️ |
| 6 | `supportsFeature()` | Проверка возможностей | ⚠️⚠️ |
| 7 | `getName()` | Имя рендерера | ⚠️ |
| 8 | `getApiVersion()` | Версия API | ⚠️ |
| 9 | `isReady()` | Готовность | ⚠️ |
| 10 | `getStats()` | Статистика | ⚠️⚠️ |
| 11 | `isInitialized()` | Проверка инициализации | ⚠️ |
| 12 | `setConfigParameter()` | Установка параметра | ⚠️⚠️ |
| 13 | `getConfigParameter()` | Получение параметра | ⚠️ |
| 14 | `hasConfigParameter()` | Проверка параметра | ⚠️ |
| 15 | `loadConfig()` | Загрузка конфигурации | ⚠️⚠️ |
| 16 | `saveConfig()` | Сохранение конфигурации | ⚠️⚠️ |
| 17 | `setRenderStrategy()` | Установка стратегии | ⚠️⚠️⚠️ |
| 18 | `addPostProcessEffect()` | Добавление эффекта | ⚠️⚠️ |
| 19 | `removePostProcessEffect()` | Удаление эффекта | ⚠️ |
| 20 | `initializeGraphicsAPI()` | Инициализация API | ⚠️⚠️ |
| 21 | `applyPostProcessEffects()` | Применение эффектов | ⚠️⚠️ |

**Тестовый файл:** `tests/modern_renderer_test.cpp` (НУЖНО СОЗДАТЬ)

---

### 2. FreGSPass.cpp

**Файл:** `src/rendering/FreGSPass.cpp` (545 строк)  
**Критичность:** ⚠️⚠️ ВЫСОКАЯ  
**Функций без тестов:** 12  
**Ожидаемых тестов:** 20-25

| № | Функция | Описание | Приоритет |
|---|---------|----------|-----------|
| 1 | `FreGSPass(const FreGSPassConfig&)` | Конструктор | ⚠️⚠️ |
| 2 | `initialize(const VulkanContext&)` | Инициализация | ⚠️⚠️⚠️ |
| 3 | `execute(vk::CommandBuffer, uint32_t)` | Выполнение прохода | ⚠️⚠️⚠️ |
| 4 | `cleanup()` | Очистка ресурсов | ⚠️⚠️ |
| 5 | `setInputSubbands(const WaveletSubbands&)` | Установка subbands | ⚠️⚠️ |
| 6 | `uploadGaussians(const vector<GaussianSplat>&)` | Загрузка гауссианов | ⚠️⚠️⚠️ |
| 7 | `updateFoveation(glm::vec2, float)` | Foveated rendering | ⚠️⚠️ |
| 8 | `createOutputImage(const VulkanContext&)` | Создание output image | ⚠️⚠️ |
| 9 | `createGaussianBuffer(const VulkanContext&)` | Создание буфера | ⚠️⚠️ |
| 10 | `createDescriptorSets(const VulkanContext&)` | Descriptor sets | ⚠️⚠️⚠️ |
| 11 | `loadShaderSPIRV()` | Загрузка шейдеров | ⚠️⚠️⚠️ |
| 12 | `updateViewProjection(const glm::mat4&)` | Обновление матриц | ⚠️⚠️ |

**Тестовый файл:** `tests/fregs_pass_test.cpp` (НУЖНО СОЗДАТЬ)

---

### 3. WaveletPass.cpp

**Файл:** `src/rendering/WaveletPass.cpp` (665 строк)  
**Критичность:** ⚠️⚠️ ВЫСОКАЯ  
**Функций без тестов:** 10  
**Ожидаемых тестов:** 15-20

| № | Функция | Описание | Приоритет |
|---|---------|----------|-----------|
| 1 | `WaveletPass(const WaveletPassConfig&)` | Конструктор | ⚠️⚠️ |
| 2 | `initialize(const VulkanContext&)` | Инициализация | ⚠️⚠️⚠️ |
| 3 | `execute(vk::CommandBuffer, uint32_t)` | Выполнение прохода | ⚠️⚠️⚠️ |
| 4 | `cleanup()` | Очистка ресурсов | ⚠️⚠️ |
| 5 | `setInputImage(vk::Image, vk::ImageView)` | Установка input | ⚠️⚠️ |
| 6 | `updateConfig(const WaveletPassConfig&)` | Обновление конфигурации | ⚠️ |
| 7 | `createSubbandImages(const VulkanContext&)` | Создание subband изображений | ⚠️⚠️ |
| 8 | `createDescriptorSets(const VulkanContext&)` | Descriptor sets | ⚠️⚠️⚠️ |
| 9 | `loadShaderSPIRV()` | Загрузка шейдеров | ⚠️⚠️⚠️ |
| 10 | `ensureInputImage(const VulkanContext&)` | Проверка input image | ⚠️ |

**Тестовый файл:** `tests/wavelet_pass_test.cpp` (НУЖНО СОЗДАТЬ)

---

## ⚠️ СРЕДНИЙ ПРИОРИТЕТ (20 функций)

### 1. Engine.cpp (App Module)

**Файл:** `src/app/Engine.cpp` (1,010 строк)  
**Критичность:** ⚠️ СРЕДНЯЯ  
**Функций без тестов:** 26  
**Ожидаемых тестов:** 40-50

| № | Функция | Описание | Приоритет |
|---|---------|----------|-----------|
| 1 | `Engine(const AppConfig&, shared_ptr<ILogger>)` | Конструктор #1 | ⚠️⚠️ |
| 2 | `Engine(const AppConfig&, ...)` | Конструктор #2 | ⚠️⚠️ |
| 3 | `~Engine()` | Деструктор | ⚠️⚠️ |
| 4 | `setExternalCameraControl(bool)` | Управление камерой | ⚠️ |
| 5 | `init()` | Инициализация движка | ⚠️⚠️⚠️ |
| 6 | `update(float)` | Обновление логики | ⚠️⚠️⚠️ |
| 7 | `render()` | Рендеринг кадра | ⚠️⚠️⚠️ |
| 8 | `load_scene(const Vulkan::SceneData&)` | Загрузка сцены | ⚠️⚠️⚠️ |
| 9 | `shutdown()` | Завершение работы | ⚠️⚠️ |
| 10 | `getSceneInfo()` | Информация о сцене | ⚠️ |
| 11 | `getCamera()` | Получение камеры | ⚠️⚠️ |
| 12 | `getRenderer()` | Получение рендерера | ⚠️⚠️ |
| 13 | `getInputManager()` | Получение input manager | ⚠️ |
| 14 | `getRenderStats()` | Статистика рендеринга | ⚠️⚠️ |
| 15 | `setDebugMode()` | Установка debug режима | ⚠️ |
| 16 | `getDebugMode()` | Получение debug режима | ⚠️ |
| 17 | `enableWireframe(bool)` | Включение wireframe | ⚠️ |
| 18 | `setBackgroundColor()` | Установка цвета фона | ⚠️ |
| 19 | `resetCameraForSponza()` | Сброс камеры для Sponza | ⚠️ |
| 20 | `logDebugInfo(const string&)` | Логирование debug info | ⚠️ |
| 21 | `isKeyPressed(int)` | Проверка нажатия клавиши | ⚠️ |
| 22 | `updateSceneInfo()` | Обновление scene info | ⚠️ |
| 23 | `updateRenderStats()` | Обновление render stats | ⚠️ |
| 24 | `setupCallbacks()` | Установка callbacks | ⚠️⚠️ |
| 25 | `keyCallback()` | Callback клавиатуры | ⚠️ |
| 26 | `mouseCallback()` | Callback мыши | ⚠️ |
| 27 | `mouseButtonCallback()` | Callback кнопок мыши | ⚠️ |

**Тестовый файл:** `tests/app_engine_test.cpp` (НУЖНО СОЗДАТЬ)

**Тип тестов:** Интеграционные + E2E

---

### 2. X11Window.cpp (Platform Module)

**Файл:** `src/platform/X11Window.cpp` (399 строк)  
**Критичность:** ⚠️ СРЕДНЯЯ  
**Функций без тестов:** 22  
**Ожидаемых тестов:** 30-40

| № | Функция | Описание | Приоритет |
|---|---------|----------|-----------|
| 1 | `X11Window()` | Конструктор | ⚠️⚠️ |
| 2 | `~X11Window()` | Деструктор | ⚠️⚠️ |
| 3 | `create(uint32_t, uint32_t, const string&)` | Создание окна | ⚠️⚠️⚠️ |
| 4 | `isOpen()` | Проверка открытия | ⚠️⚠️ |
| 5 | `update()` | Обновление и обработка событий | ⚠️⚠️⚠️ |
| 6 | `getSize(uint32_t&, uint32_t&)` | Получение размера | ⚠️ |
| 7 | `setSize(uint32_t, uint32_t)` | Установка размера | ⚠️⚠️ |
| 8 | `getTitle()` | Получение заголовка | ⚠️ |
| 9 | `setTitle(const string&)` | Установка заголовка | ⚠️ |
| 10 | `getNativeHandle()` | Получение native handle | ⚠️⚠️ |
| 11 | `setWindowEventCallback()` | Callback событий окна | ⚠️ |
| 12 | `setMouseEventCallback()` | Callback событий мыши | ⚠️ |
| 13 | `setKeyEventCallback()` | Callback событий клавиатуры | ⚠️ |
| 14 | `close()` | Закрытие окна | ⚠️⚠️ |
| 15 | `isFullscreenSupported()` | Поддержка fullscreen | ⚠️ |
| 16 | `setFullscreen(bool)` | Установка fullscreen | ⚠️⚠️ |
| 17 | `isFullscreen()` | Проверка fullscreen | ⚠️ |
| 18 | `processEvent(const XEvent&)` | Обработка X11 событий | ⚠️⚠️⚠️ |
| 19 | `convertKeyCode(KeySym)` | Конвертация кода клавиши | ⚠️⚠️ |
| 20 | `convertMouseButton(int)` | Конвертация кнопки мыши | ⚠️ |
| 21 | `sendWindowEvent()` | Отправка события окна | ⚠️ |
| 22 | `sendMouseEvent()` | Отправка события мыши | ⚠️ |
| 23 | `sendKeyEvent()` | Отправка события клавиатуры | ⚠️ |

**Тестовый файл:** `tests/x11_window_test.cpp` (НУЖНО СОЗДАТЬ)

**Требуется:** Mock X11 Display/Window

---

### 3. Остальные Rendering компоненты

| Файл | Функций | Ожидаемых тестов | Приоритет |
|------|---------|------------------|-----------|
| `RenderPass.cpp` | 5 | 8-10 | ⚠️ |
| `FreGSRenderStrategy.cpp` | 7 | 10-15 | ⚠️ |
| `InstancedMeshPass.cpp` | 8 | 12-15 | ⚠️ |

---

## 📌 LEGACY КОД (313 функций - не требует тестирования)

### OpenGL Module (DEPRECATED)

| Файл | Функций | Статус |
|------|---------|--------|
| `HybridRenderer3D.cpp` | ~54 | 📌 Legacy |
| `Renderer3D.cpp` | ~31 | 📌 Legacy |
| `OptimalRenderer3D.cpp` | ~47 | 📌 Legacy |
| `Camera3D.cpp` | ~31 | 📌 Legacy |
| `Mesh3D.cpp` | ~21 | 📌 Legacy |
| `Shader3D.cpp` | ~29 | 📌 Legacy |
| `Gaussian3D.cpp` | ~32 | 📌 Legacy |
| `RendererAdapter.cpp` | ~68 | 📌 Legacy |

**Примечание:** Legacy OpenGL код будет заменен на Vulkan. Тестирование не требуется.

---

## 📈 ПЛАН ПОКРЫТИЯ

### Фаза 1: Критический приоритет (2-3 недели)

**Цель:** Покрыть HybridFreGSRenderer (44 функции)

```
Неделя 1: Mock Vulkan API + базовые тесты (20 тестов)
Неделя 2: Тесты рендеринга + lifecycle (25 тестов)  
Неделя 3: Тесты debug режимов + статистика (15 тестов)

Итого: 60 тестов, 44 функции покрыто
```

### Фаза 2: Высокий приоритет (2 недели)

**Цель:** Покрыть ModernRenderer3D, FreGSPass, WaveletPass (43 функции)

```
Неделя 4: ModernRenderer3D (30-40 тестов)
Неделя 5: FreGSPass + WaveletPass (35-45 тестов)

Итого: 65-85 тестов, 43 функции покрыто
```

### Фаза 3: Средний приоритет (2 недели)

**Цель:** Покрыть Engine.cpp, X11Window.cpp, остальные (48 функций)

```
Неделя 6: Engine.cpp + X11Window (70-90 тестов)
Неделя 7: Остальные компоненты (30-40 тестов)

Итого: 100-130 тестов, 48 функций покрыто
```

---

## 🎯 ИТОГОВАЯ ЦЕЛЬ

**После завершения всех фаз:**

```
┌────────────────────────────────────────┐
│ ПОКРЫТИЕ ФУНКЦИЙ (без Legacy):         │
│                                        │
│ До:   633/788 функций     (80%)        │
│ После: 788/788 функций   (100%) ✅     │
│                                        │
│ Новых тестов:  ~225-255               │
│ Всего тестов:  ~984-1,014             │
│                                        │
│ Время:         6-7 недель             │
└────────────────────────────────────────┘
```

---

## 🛠️ РЕКОМЕНДАЦИИ ПО ТЕСТИРОВАНИЮ

### Для HybridFreGSRenderer:

1. **Mock Vulkan API:**
   ```cpp
   class MockVulkanInstance { /* ... */ };
   class MockVulkanDevice { /* ... */ };
   class MockVulkanSwapchain { /* ... */ };
   ```

2. **Test Fixtures:**
   ```cpp
   class HybridFreGSRendererTest : public ::testing::Test {
   protected:
       void SetUp() override;
       void TearDown() override;
       
       std::unique_ptr<MockVulkanInstance> mockInstance;
       std::unique_ptr<HybridFreGSRenderer> renderer;
   };
   ```

3. **AAA Pattern для каждого теста**

### Для X11Window:

1. **Mock X11:**
   ```cpp
   class MockX11Display { /* ... */ };
   class MockX11Window { /* ... */ };
   ```

2. **Симуляция событий:**
   ```cpp
   TEST_F(X11WindowTest, ProcessKeyEventTest) {
       XEvent mockEvent = createMockKeyEvent();
       window->processEvent(mockEvent);
       EXPECT_TRUE(keyCallbackCalled);
   }
   ```

### Для Engine:

1. **Интеграционные тесты:**
   ```cpp
   TEST_F(EngineIntegrationTest, FullLifecycleTest) {
       ASSERT_TRUE(engine->init());
       engine->update(0.016f);
       engine->render();
       engine->shutdown();
   }
   ```

2. **Mock компонентов:**
   ```cpp
   class MockRenderer : public IRenderer { /* ... */ };
   class MockResourceManager : public IResourceManager { /* ... */ };
   ```

---

**Дата создания:** 2025-10-07  
**Статус:** ✅ Готов к использованию  
**Цель:** Достижение 100% покрытия non-legacy кода

