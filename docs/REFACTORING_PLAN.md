# 📋 ПЛАН ПОЛНОГО РЕФАКТОРИНГА SpectraForge

## 🎯 ЦЕЛИ РЕФАКТОРИНГА

```yaml
Главная цель: Привести проект в полное соответствие с SOLID, TDD и всеми правилами проекта

Критические метрики:
  - SOLID Compliance: 100% (сейчас ~60%)
  - Test Coverage: 100% (сейчас 80%, 633/788 функций)
  - Code Quality: Все файлы < 500 строк
  - God Classes: 0 (сейчас 2: Engine.cpp, HybridFreGSRenderer.cpp)
  - Legacy Code: Удалить OpenGL (313 функций)
  - Documentation: 100% Doxygen coverage
  - Console Safety: 100% SAFE_TO_STRING usage
```

---

## 🗓️ ОБЩАЯ ВРЕМЕННАЯ ОЦЕНКА

```
┌─────────────────────────────────────────────────────┐
│ ФАЗЫ РЕФАКТОРИНГА                                   │
├─────────────────────────────────────────────────────┤
│ Фаза 0: Подготовка и аудит          │ 1-2 недели   │
│ Фаза 1: Критические исправления     │ 3-4 недели   │
│ Фаза 2: Архитектурный рефакторинг   │ 6-8 недель   │
│ Фаза 3: Тестирование до 100%        │ 4-5 недель   │
│ Фаза 4: Документация и polish       │ 2-3 недели   │
│ Фаза 5: Валидация и финализация     │ 1-2 недели   │
├─────────────────────────────────────────────────────┤
│ ИТОГО:                              │ 17-24 недели │
└─────────────────────────────────────────────────────┘
```

---

## 📊 ФАЗА 0: ПОДГОТОВКА И АУДИТ (1-2 недели)

### Задача 0.1: Детальный аудит кодовой базы
**Приоритет**: 🔴 КРИТИЧНО  
**Время**: 3-4 дня

**Действия**:
1. Запустить полный анализ через clang-tidy
2. Проверить все SOLID violations через статический анализ
3. Найти все использования raw pointers вместо smart pointers
4. Найти все прямые `std::cout` без `SAFE_TO_STRING`
5. Идентифицировать все God Classes (>500 строк, >10 методов)

**Инструменты**:
```bash
# Статический анализ
clang-tidy src/**/*.cpp -checks='*' > audit_report.txt

# Поиск проблем
grep -r "std::cout" src/ | grep -v "SAFE_TO_STRING"
grep -r "new \|delete " src/
grep -r "using namespace std" include/

# Метрики кода
cloc src/ include/ --by-file > code_metrics.txt
```

**Deliverables**:
- [ ] `REFACTORING_AUDIT_FULL.md` - полный отчет
- [ ] `SOLID_VIOLATIONS_LIST.md` - список нарушений
- [ ] `REFACTORING_PRIORITY_MATRIX.md` - приоритеты

---

### Задача 0.2: Настройка инфраструктуры рефакторинга
**Приоритет**: ⚠️ ВЫСОКИЙ  
**Время**: 2-3 дня

**Действия**:
1. Настроить pre-commit hooks для проверки правил
2. Добавить GitHub Actions для автоматической проверки
3. Создать benchmark suite для performance regression testing

**Файлы**:
```yaml
.pre-commit-config.yaml:
  - clang-format
  - clang-tidy
  - cppcheck
  - SAFE_TO_STRING checker
  - test coverage validator
```

**Deliverables**:
- [ ] CI/CD pipeline для проверки SOLID
- [ ] Performance regression tests

---

### Задача 0.3: Создание плана миграции тестов
**Приоритет**: ⚠️ ВЫСОКИЙ  
**Время**: 2 дня

**Действия**:
1. Проанализировать `MISSING_TESTS_MATRIX.md`
2. Создать список 225-255 недостающих тестов
3. Приоритизировать по критичности компонентов

**Deliverables**:
- [ ] `TEST_MIGRATION_PLAN.md` (225-255 тестов)
- [ ] Test templates для стандартизации

---

## 🔥 ФАЗА 1: КРИТИЧЕСКИЕ ИСПРАВЛЕНИЯ (3-4 недели)

### 🎯 Sprint 1.1: Декомпозиция HybridFreGSRenderer (1 неделя)

**Проблема**: 1383 строки, нарушает SRP, God Class

#### Задача 1.1.1: Написать тесты для текущего поведения
**TDD Step 1: RED** ✍️  
**Время**: 2 дня

```cpp
// tests/hybrid_fregs_renderer_comprehensive_test.cpp
// Создать 40-50 тестов для КАЖДОЙ функции HybridFreGSRenderer
// ПЕРЕД рефакторингом, чтобы гарантировать сохранение поведения

TEST(HybridFreGSRenderer, InitializeSuccess) { /* ... */ }
TEST(HybridFreGSRenderer, AttachWindowSuccess) { /* ... */ }
TEST(HybridFreGSRenderer, BeginFrameSuccess) { /* ... */ }
TEST(HybridFreGSRenderer, RenderFrameTriangleSplatting) { /* ... */ }
TEST(HybridFreGSRenderer, RenderFrameGaussianSplatting) { /* ... */ }
TEST(HybridFreGSRenderer, SwitchRenderMode) { /* ... */ }
// ... 40+ тестов
```

**Deliverables**:
- [ ] 40-50 новых тестов для HybridFreGSRenderer
- [ ] 100% coverage для этого класса

---

#### Задача 1.1.2: Декомпозиция на компоненты
**TDD Step 2: GREEN** ✅  
**Время**: 3 дня

**Новая архитектура**:
```cpp
// include/SpectraForge/Rendering/Core/RendererCore.h
class RendererCore {
    // Базовая Vulkan инициализация (SRP)
    // ~200 строк
};

// include/SpectraForge/Rendering/Core/SwapchainManager.h
class SwapchainManager {
    // Управление swapchain (SRP)
    // ~150 строк
};

// include/SpectraForge/Rendering/Core/FrameManager.h
class FrameManager {
    // Управление кадрами, fences, semaphores (SRP)
    // ~180 строк
};

// include/SpectraForge/Rendering/Core/PipelineManager.h
class PipelineManager {
    // Управление compute pipelines (SRP)
    // ~200 строк
};

// include/SpectraForge/Rendering/HybridFreGSRenderer.h (НОВЫЙ)
class HybridFreGSRenderer : public IRenderer {
private:
    std::unique_ptr<RendererCore> m_core;           // DIP
    std::unique_ptr<SwapchainManager> m_swapchain;  // DIP
    std::unique_ptr<FrameManager> m_frameManager;   // DIP
    std::unique_ptr<PipelineManager> m_pipelines;   // DIP
    std::unique_ptr<TriangleSplattingPass> m_trianglePass; // DIP
    std::unique_ptr<FreGSPass> m_fregsPass;         // DIP
    std::unique_ptr<WaveletPass> m_waveletPass;     // DIP
    
public:
    // Теперь только ~250 строк - координирует компоненты
    HybridFreGSRenderer(
        std::unique_ptr<RendererCore> core,
        std::unique_ptr<SwapchainManager> swapchain,
        /* ... */
    ); // Constructor injection (DIP)
};
```

**Deliverables**:
- [ ] 7 новых классов (RendererCore, SwapchainManager, etc.)
- [ ] HybridFreGSRenderer сокращен до ~250 строк
- [ ] Все тесты GREEN ✅

---

#### Задача 1.1.3: Рефакторинг тестов
**TDD Step 3: REFACTOR** 🔧  
**Время**: 1 день

```cpp
// Обновить тесты для использования mock injection
class MockRendererCore : public RendererCore { /* ... */ };

TEST(HybridFreGSRenderer, InitializeWithMocks) {
    auto mockCore = std::make_unique<MockRendererCore>();
    // ...
}
```

**Deliverables**:
- [ ] Обновленные тесты с dependency injection
- [ ] Coverage остался 100%

---

### 🎯 Sprint 1.2: Декомпозиция Engine.cpp (1 неделя)

**Проблема**: 1010 строк, God Class

#### Задача 1.2.1: Написать тесты (TDD RED)
**Время**: 2 дня

```cpp
// tests/app_engine_comprehensive_test.cpp
// 30-40 тестов для ВСЕХ функций Engine
```

**Deliverables**:
- [ ] 30-40 новых тестов для Engine

---

#### Задача 1.2.2: Декомпозиция (TDD GREEN)
**Время**: 3 дня

**Новая архитектура**:
```cpp
// include/SpectraForge/App/SubsystemManager.h
class SubsystemManager {
    // Управление подсистемами (SRP)
    void registerSubsystem(std::shared_ptr<ISubsystem>);
    void initializeAll();
    void shutdownAll();
};

// include/SpectraForge/App/GameLoop.h
class GameLoop {
    // Главный игровой цикл (SRP)
    void run();
    void stop();
};

// include/SpectraForge/App/EventDispatcher.h
class EventDispatcher {
    // Обработка событий (SRP)
};

// include/SpectraForge/App/Engine.h (НОВЫЙ)
class Engine {
private:
    std::unique_ptr<SubsystemManager> m_subsystems;    // DIP
    std::unique_ptr<GameLoop> m_gameLoop;              // DIP
    std::unique_ptr<EventDispatcher> m_eventDispatcher; // DIP
    std::shared_ptr<IRenderer> m_renderer;             // DIP
    
public:
    Engine(
        std::unique_ptr<SubsystemManager> subsystems,
        std::unique_ptr<GameLoop> gameLoop,
        /* ... */
    ); // Constructor injection
    
    // Теперь только ~200 строк
};
```

**Deliverables**:
- [ ] 4 новых класса
- [ ] Engine сокращен до ~200 строк
- [ ] Тесты GREEN ✅

---

### 🎯 Sprint 1.3: Удаление Legacy OpenGL кода (1 неделя)

**Проблема**: 313 функций устаревшего OpenGL кода

#### Задача 1.3.1: Идентификация зависимостей
**Время**: 1 день

```bash
# Найти все использования OpenGL классов
grep -r "Renderer3D\|HybridRenderer3D\|OptimalRenderer3D" src/ include/
```

**Deliverables**:
- [ ] Список всех зависимостей на OpenGL код

---

#### Задача 1.3.2: Миграция на Vulkan API
**Время**: 2 дня

**Действия**:
- Все примеры переписать на HybridFreGSRenderer
- Удалить все `src/rendering/opengl/*`

**Deliverables**:
- [ ] Все примеры используют только Vulkan
- [ ] OpenGL код удален

---

#### Задача 1.3.3: Обновление тестов
**Время**: 2 дня

**Deliverables**:
- [ ] Удалены тесты OpenGL рендереров
- [ ] Добавлены тесты для Vulkan замен

---

### 🎯 Sprint 1.4: Безопасность консольного вывода (3 дня)

**Проблема**: Не все `std::cout` используют `SAFE_TO_STRING`

#### Задача 1.4.1: Аудит консольного вывода
**Время**: 1 день

```bash
# Найти все небезопасные вызовы
grep -rn "std::cout\|std::cerr\|printf" src/ include/ | grep -v "SAFE_TO_STRING"
```

**Deliverables**:
- [ ] Список всех небезопасных вызовов

---

#### Задача 1.4.2: Замена на SAFE_TO_STRING
**Время**: 2 дня

```cpp
// ❌ БЫЛО
std::cout << "Value: " << value << std::endl;

// ✅ СТАЛО
std::cout << "Value: " << SAFE_TO_STRING(value) << std::endl;
```

**Deliverables**:
- [ ] 100% использование SAFE_TO_STRING
- [ ] Тесты для проверки безопасности

---

## 🏗️ ФАЗА 2: АРХИТЕКТУРНЫЙ РЕФАКТОРИНГ (6-8 недель)

### 🎯 Sprint 2.1: Dependency Injection контейнер (1 неделя)

#### Задача 2.1.1: Написать тесты для DI контейнера (TDD RED)
**Время**: 2 дня

```cpp
// tests/core_di_container_test.cpp
TEST(DIContainer, RegisterAndResolveInterface) {
    DIContainer container;
    container.registerSingleton<IRenderer, HybridFreGSRenderer>();
    
    auto renderer = container.resolve<IRenderer>();
    ASSERT_NE(renderer, nullptr);
}

TEST(DIContainer, ResolveWithDependencies) {
    // Тест автоматического разрешения зависимостей
}
```

**Deliverables**:
- [ ] 15-20 тестов для DI контейнера

---

#### Задача 2.1.2: Реализация DI контейнера (TDD GREEN)
**Время**: 3 дня

```cpp
// include/SpectraForge/Core/DependencyInjection/Container.h
class DIContainer {
public:
    template<typename Interface, typename Implementation>
    void registerSingleton();
    
    template<typename Interface, typename Implementation>
    void registerTransient();
    
    template<typename T>
    std::shared_ptr<T> resolve();
    
private:
    std::unordered_map<std::type_index, std::function<std::shared_ptr<void>()>> m_factories;
    std::unordered_map<std::type_index, std::shared_ptr<void>> m_singletons;
};
```

**Deliverables**:
- [ ] Полнофункциональный DI контейнер
- [ ] Тесты GREEN ✅

---

#### Задача 2.1.3: Интеграция DI во всем проекте
**Время**: 2 дня

```cpp
// main.cpp
int main() {
    DIContainer container;
    
    // Регистрация зависимостей
    container.registerSingleton<ILogger, Logger>();
    container.registerSingleton<IRenderer, HybridFreGSRenderer>();
    container.registerSingleton<IResourceManager, ResourceManager>();
    
    // Автоматическое разрешение
    auto engine = container.resolve<Engine>();
    engine->run();
}
```

**Deliverables**:
- [ ] Все классы используют DI
- [ ] Удалены hardcoded dependencies

---

### 🎯 Sprint 2.2: Mesh Connectivity для Triangle Splatting+ (2 недели)

**Ссылка на план**: `docs/triangle-mesh-conversion-plan.md`

#### Задача 2.2.1: Написать тесты для connectivity (TDD RED)
**Время**: 3 дня

```cpp
// tests/mesh_connectivity_test.cpp
TEST(MeshConnectivity, VertexDeduplication) {
    // Тест удаления дубликатов вершин
}

TEST(MeshConnectivity, AdjacencyBuilding) {
    // Тест построения adjacency
}

TEST(MeshConnectivity, TopologyValidation) {
    // Тест валидации топологии
}
```

**Deliverables**:
- [ ] 20-25 тестов для connectivity

---

#### Задача 2.2.2: Реализация connectivity (TDD GREEN)
**Время**: 5 дней

```cpp
// include/SpectraForge/Rendering/MeshConnectivity.hpp
struct ConnectedVertex {
    glm::vec3 position;
    glm::vec3 color;
    float opacity;
    std::vector<uint32_t> adjacentTriangles;
    std::vector<uint32_t> adjacentVertices;
};

struct ConnectedTriangle {
    uint32_t indices[3];
    float smoothness;
    uint32_t neighbors[3];
    glm::vec3 normal;
    float area;
};

class MeshConnectivityBuilder {
public:
    ConnectedMesh buildFromTriangleSoup(
        const std::vector<TriangleSplattingPass::Triangle>&
    );
    
private:
    void deduplicateVertices();
    void buildAdjacency();
    void validateTopology();
};
```

**Deliverables**:
- [ ] Полная реализация connectivity
- [ ] Тесты GREEN ✅

---

#### Задача 2.2.3: Интеграция в TriangleSplattingPass
**Время**: 3 дня

```cpp
// src/rendering/TriangleSplattingPass.cpp
void TriangleSplattingPass::uploadMesh(const ConnectedMesh& mesh) {
    // Загрузка с учетом connectivity
}
```

**Deliverables**:
- [ ] TriangleSplattingPass использует connectivity
- [ ] Performance улучшен на 10-15%

---

### 🎯 Sprint 2.3: Рефакторинг интерфейсов (ISP) (1 неделя)

**Проблема**: Некоторые интерфейсы слишком "жирные"

#### Задача 2.3.1: Сегрегация IRenderer
**Время**: 3 дня

```cpp
// include/SpectraForge/Rendering/Common/IRenderer.h
class IRenderer {
    virtual bool initialize() = 0;
    virtual void shutdown() = 0;
    virtual void renderFrame(const FrameData&) = 0;
};

// include/SpectraForge/Rendering/Common/IDebugRenderer.h
class IDebugRenderer {
    virtual void setDebugMode(int mode) = 0;
    virtual void enableWireframe(bool) = 0;
    virtual DetailedRenderingStats getDetailedStats() = 0;
};

// include/SpectraForge/Rendering/Common/IScreenshotCapture.h
class IScreenshotCapture {
    virtual bool saveScreenshot(const std::string&) = 0;
};

// HybridFreGSRenderer implements все 3
class HybridFreGSRenderer : public IRenderer,
                             public IDebugRenderer,
                             public IScreenshotCapture {
    // Теперь соответствует ISP
};
```

**Deliverables**:
- [ ] Разделены интерфейсы
- [ ] Все классы соответствуют ISP

---

### 🎯 Sprint 2.4: Open/Closed Principle (OCP) (1 неделя)

#### Задача 2.4.1: Стратегии рендеринга (OCP)
**Время**: 4 дня

```cpp
// include/SpectraForge/Rendering/RenderStrategy/IRenderStrategy.h
class IRenderStrategy {
public:
    virtual ~IRenderStrategy() = default;
    virtual void execute(const RenderContext&) = 0;
};

// Новые стратегии без изменения существующего кода
class TriangleSplattingStrategy : public IRenderStrategy { /* ... */ };
class GaussianSplattingStrategy : public IRenderStrategy { /* ... */ };
class HybridStrategy : public IRenderStrategy { /* ... */ };

// Factory для выбора стратегии
class RenderStrategyFactory {
public:
    static std::unique_ptr<IRenderStrategy> create(RenderMode mode);
};
```

**Deliverables**:
- [ ] Расширяемая система стратегий (OCP)

---

### 🎯 Sprint 2.5: Liskov Substitution (LSP) валидация (3 дня)

#### Задача 2.5.1: Проверка всех иерархий наследования
**Время**: 2 дня

```cpp
// tests/lsp_validation_test.cpp
// Тесты для проверки LSP во всех иерархиях

template<typename Base, typename Derived>
void testLiskovSubstitution() {
    std::unique_ptr<Base> base = std::make_unique<Derived>();
    // Все операции базового класса должны работать
}

TEST(LSP, RendererHierarchy) {
    testLiskovSubstitution<IRenderer, HybridFreGSRenderer>();
    testLiskovSubstitution<IRenderer, ModernRenderer3D>();
}
```

**Deliverables**:
- [ ] LSP тесты для всех иерархий
- [ ] Исправлены все нарушения LSP

---

## 🧪 ФАЗА 3: ТЕСТИРОВАНИЕ ДО 100% (4-5 недель)

### 🎯 Sprint 3.1-3.5: Написание 225-255 новых тестов

**Распределение по неделям**:

#### Sprint 3.1 (Неделя 1): HybridFreGSRenderer (44 функции)
**Тесты**: 50-60 новых тестов  
**Время**: 1 неделя

```cpp
// tests/hybrid_fregs_renderer_complete_test.cpp
// По 1-2 теста на каждую функцию
```

---

#### Sprint 3.2 (Неделя 2): ModernRenderer3D + FreGSPass (33 функции)
**Тесты**: 40-45 новых тестов  
**Время**: 1 неделя

---

#### Sprint 3.3 (Неделя 3): Engine + WaveletPass (36 функций)
**Тесты**: 45-50 новых тестов  
**Время**: 1 неделя

---

#### Sprint 3.4 (Неделя 4): X11Window + Platform (22 функции)
**Тесты**: 30-35 новых тестов  
**Время**: 1 неделя

---

#### Sprint 3.5 (Неделя 5): Integration tests + Edge cases
**Тесты**: 40-50 новых тестов  
**Время**: 1 неделя

**Типы тестов**:
- Integration tests между компонентами
- Edge cases и error handling
- Performance regression tests
- Memory leak tests (Valgrind)

---

## 📝 ФАЗА 4: ДОКУМЕНТАЦИЯ И POLISH (2-3 недели)

### 🎯 Sprint 4.1: Doxygen документация (1 неделя)

#### Задача 4.1.1: Аудит документации
**Время**: 1 день

```bash
# Найти все недокументированные публичные API
doxygen -g && doxygen Doxyfile
# Проверить warnings
```

**Deliverables**:
- [ ] Список недокументированных API

---

#### Задача 4.1.2: Написание Doxygen комментариев
**Время**: 4 дня

```cpp
/**
 * @brief Гибридный рендерер, поддерживающий Triangle Splatting и FreGS
 * 
 * Этот класс реализует два режима рендеринга:
 * - Triangle Splatting для mesh данных (.obj)
 * - Frequency Gaussian Splatting для point clouds (.ply)
 * 
 * @note Использует Vulkan 1.3 compute shaders для GPU-ускорения
 * 
 * @see TriangleSplattingPass, FreGSPass, WaveletPass
 * 
 * @example
 * @code
 * auto renderer = std::make_unique<HybridFreGSRenderer>();
 * renderer->initialize();
 * renderer->setRenderMode(RenderMode::TriangleSplatting);
 * renderer->renderFrame(frameData);
 * @endcode
 */
class HybridFreGSRenderer : public IRenderer {
    /**
     * @brief Инициализирует Vulkan контекст и ресурсы
     * 
     * @return true если инициализация успешна, false иначе
     * @throws std::runtime_error если Vulkan недоступен
     */
    bool initialize() override;
};
```

**Deliverables**:
- [ ] 100% Doxygen coverage для публичных API
- [ ] Сгенерированная HTML документация

---

### 🎯 Sprint 4.2: Обновление документов проекта (1 неделя)

#### Задача 4.2.1: Обновление README и CONTRIBUTING
**Время**: 2 дня

**Deliverables**:
- [ ] Обновленный README.md с новой архитектурой
- [ ] Обновленный CONTRIBUTING.md с SOLID guidelines

---

#### Задача 4.2.2: Создание архитектурной документации
**Время**: 3 дня

**Новые документы**:
- `docs/ARCHITECTURE.md` - детальная архитектура
- `docs/SOLID_COMPLIANCE.md` - как мы соблюдаем SOLID
- `docs/TESTING_STRATEGY.md` - стратегия тестирования
- `docs/API_REFERENCE.md` - справочник API

**Deliverables**:
- [ ] 4 новых документа
- [ ] Диаграммы классов (PlantUML)

---

### 🎯 Sprint 4.3: Code quality polish (3 дня)

#### Задача 4.3.1: Форматирование и стиль
**Время**: 1 день

```bash
# Применить clang-format ко всем файлам
find src include -name "*.cpp" -o -name "*.h" | xargs clang-format -i

# Проверить naming conventions
./scripts/check_naming.sh
```

**Deliverables**:
- [ ] 100% соответствие .clang-format
- [ ] 100% соответствие naming conventions

---

#### Задача 4.3.2: Удаление мертвого кода
**Время**: 2 дня

```bash
# Найти неиспользуемые функции
cppcheck --enable=unusedFunction src/
```

**Deliverables**:
- [ ] Удален весь мертвый код
- [ ] Удалены неиспользуемые includes

---

## ✅ ФАЗА 5: ВАЛИДАЦИЯ И ФИНАЛИЗАЦИЯ (1-2 недели)

### 🎯 Sprint 5.1: Финальная валидация (1 неделя)

#### Задача 5.1.1: SOLID Audit
**Время**: 2 дня

```bash
# Запустить все проверки
./scripts/solid_audit.sh
```

**Критерии**:
- [ ] ✅ SRP: Все классы < 500 строк
- [ ] ✅ OCP: Используются стратегии/фабрики
- [ ] ✅ LSP: Все иерархии проходят LSP тесты
- [ ] ✅ ISP: Нет "жирных" интерфейсов
- [ ] ✅ DIP: DI контейнер везде

---

#### Задача 5.1.2: Test Coverage Validation
**Время**: 1 день

```bash
./run_tests_with_coverage.sh
# Должно быть 100% (788/788 функций)
```

**Критерии**:
- [ ] ✅ 100% line coverage (не считая legacy)
- [ ] ✅ 100% branch coverage
- [ ] ✅ Все тесты GREEN

---

#### Задача 5.1.3: Performance Regression Testing
**Время**: 2 дня

```bash
./build/SpectraForge_Benchmark --compare-baseline
```

**Критерии**:
- [ ] ✅ Рендеринг не медленнее ±5%
- [ ] ✅ Memory usage не выше +10%
- [ ] ✅ 60+ FPS на мобильных GPU

---

### 🎯 Sprint 5.2: Релиз подготовка (3 дня)

#### Задача 5.2.1: Создание Release Notes
**Время**: 1 день

**Deliverables**:
- [ ] `RELEASE_NOTES_v2.0.md`
- [ ] Migration guide для существующих пользователей

---

#### Задача 5.2.2: Финальный merge
**Время**: 2 дня

**Действия**:
1. Merge `refactor/solid-compliance` → `develop`
2. Полное тестирование на develop
3. Merge `develop` → `release/master`
4. Создание тега `v2.0.0-refactored`

**Deliverables**:
- [ ] Релиз v2.0.0 на GitHub
- [ ] Docker image обновлен

---

## 📊 МЕТРИКИ УСПЕХА

### До рефакторинга (текущее состояние)
```yaml
SOLID Compliance:      60% (частичное)
Test Coverage:         80% (633/788)
God Classes:           2 (Engine, HybridFreGSRenderer)
Avg File Size:         ~350 lines
Documentation:         60%
SAFE_TO_STRING:        ~70%
Legacy Code:           313 functions (OpenGL)
```

### После рефакторинга (целевое состояние)
```yaml
SOLID Compliance:      100% ✅
Test Coverage:         100% (788/788) ✅
God Classes:           0 ✅
Avg File Size:         ~200 lines ✅
Max File Size:         500 lines ✅
Documentation:         100% Doxygen ✅
SAFE_TO_STRING:        100% ✅
Legacy Code:           0 functions ✅
```

---

## 🎯 КРИТИЧЕСКИЕ ПРАВИЛА ПРИ РЕФАКТОРИНГЕ

### 🔴 ОБЯЗАТЕЛЬНО (из MASTER RULES):

1. **TDD ALWAYS**: Тесты ПЕРЕД кодом (RED → GREEN → REFACTOR)
2. **SOLID STRICT**: Блокировать merge при нарушениях SOLID
3. **DI EVERYWHERE**: Dependency Injection через конструкторы
4. **NO GOD CLASSES**: Максимум 500 строк на файл
5. **SAFE_TO_STRING**: Всегда для console output
6. **100% DOXYGEN**: Для всех публичных API
7. **SMART POINTERS**: NO raw pointers для ownership

### ⚠️ ВАЖНО:

1. **Incremental Changes**: Маленькие commits, часто
2. **Always Tested**: После каждого шага все тесты GREEN
3. **Performance**: Benchmark после каждой фазы
4. **Documentation**: Обновлять синхронно с кодом

---

## 🛠️ ИНСТРУМЕНТЫ И АВТОМАТИЗАЦИЯ

### Скрипты для автоматизации:

```bash
# scripts/solid_audit.sh
#!/bin/bash
echo "🔍 Проверка SOLID принципов..."
clang-tidy src/**/*.cpp -checks='cppcoreguidelines-*,modernize-*'
echo "✅ Аудит завершен"

# scripts/test_coverage.sh
#!/bin/bash
echo "🧪 Проверка покрытия тестами..."
./run_tests_with_coverage.sh
echo "📊 Coverage: $(cat coverage.txt)"

# scripts/refactor_check.sh
#!/bin/bash
./scripts/solid_audit.sh
./scripts/test_coverage.sh
./scripts/performance_baseline.sh
echo "✅ Все проверки пройдены"
```

---

## 📅 КАЛЕНДАРНЫЙ ПЛАН

```
МЕСЯЦ 1:
  Неделя 1-2: Фаза 0 (Подготовка)
  Неделя 3-4: Фаза 1 Sprint 1.1-1.2

МЕСЯЦ 2:
  Неделя 5-6: Фаза 1 Sprint 1.3-1.4
  Неделя 7-8: Фаза 2 Sprint 2.1-2.2

МЕСЯЦ 3:
  Неделя 9-10: Фаза 2 Sprint 2.3-2.5
  Неделя 11-12: Фаза 3 Sprint 3.1-3.2

МЕСЯЦ 4:
  Неделя 13-14: Фаза 3 Sprint 3.3-3.4
  Неделя 15-16: Фаза 3 Sprint 3.5 + Фаза 4 Sprint 4.1

МЕСЯЦ 5:
  Неделя 17-18: Фаза 4 Sprint 4.2-4.3
  Неделя 19: Фаза 5 Sprint 5.1

МЕСЯЦ 6:
  Неделя 20: Фаза 5 Sprint 5.2
  РЕЛИЗ v2.0.0 🎉
```

---

## ✨ ИТОГОВЫЙ РЕЗУЛЬТАТ

После завершения рефакторинга проект будет:

✅ **Полностью SOLID-compliant**  
✅ **100% покрытие тестами**  
✅ **Отсутствие God Classes**  
✅ **Полная документация API**  
✅ **Современный C++ (smart pointers, RAII, DI)**  
✅ **Расширяемая архитектура (OCP)**  
✅ **Безопасный console output**  
✅ **Производительность не снижена**  
✅ **Ready for production**

---

**Готов начать рефакторинг с Фазы 0?** 🚀