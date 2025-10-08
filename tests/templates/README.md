# 📝 ШАБЛОНЫ ТЕСТОВ SpectraForge

**Версия**: 1.0.0  
**Дата**: 2025-10-08  
**Статус**: ✅ АКТИВЕН

---

## 📋 ДОСТУПНЫЕ ШАБЛОНЫ

### 1. `unit_test_template.cpp`
**Назначение**: Юнит-тесты для обычных C++ классов  
**Использовать для**:
- Core компонентов (Logger, GameObject3D, Transform3D)
- Math компонентов (Vector3, Matrix4, Quaternion)
- Utility классов

**Особенности**:
- ✅ AAA Pattern (Arrange-Act-Assert)
- ✅ Test Fixtures с SetUp/TearDown
- ✅ Mock зависимостей через GMock
- ✅ Edge cases и error handling
- ✅ Performance tests

**Пример использования**:
```bash
cp tests/templates/unit_test_template.cpp tests/my_component_test.cpp
# Замените все <Component>, <Module>, <Dependency> на реальные имена
```

---

### 2. `vulkan_component_test_template.cpp`
**Назначение**: Тесты для компонентов с Vulkan API  
**Использовать для**:
- Rendering компонентов (HybridFreGSRenderer, TriangleSplattingPass)
- Vulkan passes (FreGSPass, WaveletPass)
- VMA memory managers

**Особенности**:
- ✅ Mock Vulkan objects (Instance, Device, Allocator)
- ✅ VMA buffer/image testing
- ✅ Command buffer recording tests
- ✅ Resource cleanup validation
- ✅ Error handling (VK_ERROR_*)

**Пример использования**:
```bash
cp tests/templates/vulkan_component_test_template.cpp tests/my_vulkan_component_test.cpp
```

---

### 3. `integration_test_template.cpp`
**Назначение**: Интеграционные и E2E тесты  
**Использовать для**:
- Тестирование взаимодействия компонентов
- Full lifecycle тесты
- Performance benchmarks
- E2E сценарии

**Особенности**:
- ✅ Реальные компоненты (минимум mocking)
- ✅ Multi-component interactions
- ✅ Resource management tests
- ✅ Concurrency tests
- ✅ Realistic workflows

**Пример использования**:
```bash
cp tests/templates/integration_test_template.cpp tests/system_integration_test.cpp
```

---

## 🎯 БЫСТРЫЙ СТАРТ

### Шаг 1: Выбрать шаблон
Определите тип теста:
- **Unit test** → `unit_test_template.cpp`
- **Vulkan component** → `vulkan_component_test_template.cpp`
- **Integration** → `integration_test_template.cpp`

### Шаг 2: Скопировать и переименовать
```bash
cd /home/tigron/Documents/GITHUB/SpectraForge/tests
cp templates/<template_name> <your_component>_test.cpp
```

### Шаг 3: Заменить placeholders
Найдите и замените все вхождения:
- `<Component>` → имя вашего класса (например, `HybridFreGSRenderer`)
- `<Module>` → имя модуля (например, `Rendering`)
- `<Dependency>` → зависимости класса
- `<System>` → имя системы (для integration тестов)

**Использование IDE**:
- **VSCode**: `Ctrl+H` (Replace)
- **CLion**: `Ctrl+R` (Replace in Files)

### Шаг 4: Добавить в CMakeLists.txt
```cmake
# tests/CMakeLists.txt
add_spectraforge_test(my_component_test my_component_test.cpp)
```

### Шаг 5: Реализовать тесты
Следуйте структуре шаблона:
1. Заполните SetUp() / TearDown()
2. Реализуйте тесты для каждого метода
3. Добавьте edge cases
4. Добавьте error handling tests

### Шаг 6: Запустить тесты
```bash
cd build
make
./bin/my_component_test
```

---

## 📐 СТАНДАРТЫ ТЕСТИРОВАНИЯ

### Обязательные требования

#### 1. AAA Pattern
```cpp
TEST_F(ComponentTest, MethodName_Scenario_ExpectedResult) {
    // Arrange - подготовка
    Component obj = createObject();
    
    // Act - действие
    Result result = obj.doSomething();
    
    // Assert - проверка
    EXPECT_EQ(result, expectedValue);
}
```

#### 2. Naming Convention
```
<ClassUnderTest>Test.<MethodName>_<InputCondition>_<ExpectedBehavior>

Примеры:
✅ Vector3Test.Add_TwoVectors_ReturnsSum
✅ RendererTest.Initialize_WithValidDevice_Success
✅ BufferTest.Allocate_WithZeroSize_ThrowsException
```

#### 3. Test Coverage
Для каждой функции минимум:
- ✅ 1 тест success path
- ✅ 1 тест failure path (если применимо)
- ✅ 1 тест edge case (границы)
- ✅ 1 тест error handling

#### 4. Mocking Strategy
- **Unit tests**: Mock все зависимости
- **Integration tests**: Mock только внешние системы
- **E2E tests**: Минимум mocking

---

## 🔧 ИНСТРУМЕНТЫ И МАКРОСЫ

### Google Test Assertions

**Основные**:
```cpp
EXPECT_TRUE(condition)
EXPECT_FALSE(condition)
EXPECT_EQ(val1, val2)
EXPECT_NE(val1, val2)
EXPECT_LT(val1, val2)  // Less Than
EXPECT_GT(val1, val2)  // Greater Than
```

**Float/Double**:
```cpp
EXPECT_FLOAT_EQ(val1, val2)
EXPECT_DOUBLE_EQ(val1, val2)
EXPECT_NEAR(val1, val2, epsilon)
```

**Exceptions**:
```cpp
EXPECT_THROW(statement, exception_type)
EXPECT_NO_THROW(statement)
EXPECT_ANY_THROW(statement)
```

### Google Mock Macros

**Method mocking**:
```cpp
class MockComponent : public IComponent {
public:
    MOCK_METHOD(bool, initialize, (), (override));
    MOCK_METHOD(void, process, (int value), (override));
    MOCK_METHOD(int, getValue, (), (const, override));
};
```

**Expectations**:
```cpp
EXPECT_CALL(mockObj, method())
    .Times(1)
    .WillOnce(Return(true));

EXPECT_CALL(mockObj, method(_))  // Any argument
    .Times(AtLeast(1))
    .WillRepeatedly(Return(42));
```

---

## 📊 ПРИМЕРЫ ПО КАТЕГОРИЯМ

### Example 1: Simple Unit Test
```cpp
TEST_F(Vector3Test, Add_TwoVectors_ReturnsSum) {
    // Arrange
    Vector3 v1(1.0f, 2.0f, 3.0f);
    Vector3 v2(4.0f, 5.0f, 6.0f);
    
    // Act
    Vector3 result = v1 + v2;
    
    // Assert
    EXPECT_FLOAT_EQ(result.x, 5.0f);
    EXPECT_FLOAT_EQ(result.y, 7.0f);
    EXPECT_FLOAT_EQ(result.z, 9.0f);
}
```

### Example 2: Mock Dependency Test
```cpp
TEST_F(RendererTest, Initialize_WhenDeviceFails_ReturnsFalse) {
    // Arrange
    auto mockDevice = std::make_shared<MockVulkanDevice>();
    EXPECT_CALL(*mockDevice, isValid())
        .WillOnce(Return(false));
    
    auto renderer = std::make_unique<Renderer>(mockDevice);
    
    // Act
    bool result = renderer->initialize();
    
    // Assert
    EXPECT_FALSE(result);
}
```

### Example 3: Edge Case Test
```cpp
TEST_F(BufferTest, Allocate_WithMaxSize_HandlesCorrectly) {
    // Arrange
    size_t maxSize = std::numeric_limits<size_t>::max();
    
    // Act & Assert
    EXPECT_THROW({
        buffer->allocate(maxSize);
    }, std::bad_alloc);
}
```

### Example 4: Performance Test
```cpp
TEST_F(SystemTest, Process_1000Items_CompletesInTime) {
    // Arrange
    auto items = createTestItems(1000);
    auto start = std::chrono::high_resolution_clock::now();
    
    // Act
    system->process(items);
    
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    
    // Assert
    EXPECT_LT(duration.count(), 100); // < 100ms
}
```

---

## ✅ CHECKLIST ПЕРЕД COMMIT

Перед коммитом тестов проверьте:

- [ ] ✅ Все тесты проходят (`make test`)
- [ ] ✅ Следуют AAA pattern
- [ ] ✅ Naming convention соблюден
- [ ] ✅ Doxygen комментарии добавлены
- [ ] ✅ Mock объекты корректны
- [ ] ✅ Edge cases покрыты
- [ ] ✅ Error handling протестирован
- [ ] ✅ Нет дублирования кода
- [ ] ✅ Performance приемлемый (<10ms/test)
- [ ] ✅ Добавлен в CMakeLists.txt

---

## 🔗 СВЯЗАННЫЕ ДОКУМЕНТЫ

- `TEST_MIGRATION_PLAN.md` - План миграции тестов
- `MISSING_TESTS_MATRIX.md` - Матрица недостающих тестов
- `QUICK_TEST_GUIDE.md` - Быстрое руководство по тестам
- `REFACTORING_PLAN.md` - Общий план рефакторинга

---

## 📞 ПОДДЕРЖКА

Если возникли вопросы по использованию шаблонов:
1. Посмотрите существующие тесты: `tests/math_vector3_test.cpp`
2. Изучите документацию Google Test/Mock
3. Обратитесь к команде разработки

---

## 📝 ИСТОРИЯ ИЗМЕНЕНИЙ

| Версия | Дата | Изменения |
|--------|------|-----------|
| 1.0.0 | 2025-10-08 | Первый релиз шаблонов |

---

**Статус**: ✅ ГОТОВ К ИСПОЛЬЗОВАНИЮ  
**Версия**: 1.0.0  
**Автор**: SpectraForge Team

