# Быстрое руководство по тестированию SpectraForge

## 🚀 Быстрый старт

### 1. Запустить все тесты с покрытием
```bash
./run_tests_with_coverage.sh
```

### 2. Просмотреть отчет покрытия
```bash
firefox build/coverage_report/index.html
```

### 3. Запустить быстрые тесты
```bash
cd build
make run_unit_tests
```

## 📋 Доступные команды

| Команда | Описание | Время |
|---------|----------|-------|
| `./run_tests_with_coverage.sh` | Полный запуск с покрытием | ~2 мин |
| `make run_all_tests` | Все тесты без покрытия | ~30 сек |
| `make run_unit_tests` | Быстрые unit-тесты | ~5 сек |
| `make run_math_tests` | Только Math модуль | ~2 сек |
| `make run_core_tests` | Только Core модуль | ~3 сек |
| `make run_integration_tests` | Интеграционные тесты | ~5 сек |
| `make coverage` | Генерация отчета покрытия | ~1 мин |

## 📊 Структура тестов

```
tests/
├── Math Tests (4 файла, ~280 тестов)
│   ├── math_vector3_test.cpp
│   ├── math_matrix4_test.cpp
│   ├── math_quaternion_test.cpp
│   └── math_vector_extra_test.cpp
│
├── Core Tests (4 файла, ~170 тестов)
│   ├── core_transform3d_test.cpp
│   ├── core_logger_test.cpp
│   ├── core_gameobject3d_test.cpp
│   └── core_vma_memory_test.cpp
│
├── Integration Tests (1 файл, 11 сценариев)
│   └── integration_pipeline_test.cpp
│
└── Rendering Tests (6 файлов, существующие тесты)
    ├── triangle_splatting_test.cpp
    ├── connectivity_phase1_test.cpp
    ├── render_strategy_interfaces_test.cpp
    └── ...
```

## ✅ Чек-лист перед коммитом

1. [ ] Запустить unit-тесты: `make run_unit_tests`
2. [ ] Проверить, что все тесты проходят
3. [ ] Запустить с покрытием: `./run_tests_with_coverage.sh`
4. [ ] Убедиться, что покрытие >98%
5. [ ] Исправить найденные проблемы

## 🎯 Целевые метрики

- **Покрытие строк:** >98% ✅
- **Покрытие функций:** >95% ✅
- **Покрытие веток:** >90% ✅
- **Время unit-тестов:** <5 секунд ✅

## 🐛 Отладка тестов

### Запуск отдельного теста
```bash
cd build/bin
./math_vector3_test
```

### Запуск с детальным выводом
```bash
./math_vector3_test --gtest_verbose=1
```

### Запуск конкретного теста
```bash
./math_vector3_test --gtest_filter=Vector3Test.Addition
```

## 📈 Просмотр покрытия

### Генерация отчета
```bash
cd build
make coverage
```

### HTML отчет
- Откройте: `build/coverage_report/index.html`
- Навигация по модулям
- Детальный просмотр каждого файла
- Непокрытые строки выделены красным

### Консольный вывод
```bash
lcov --summary build/coverage_filtered.info
```

## 🔧 Устранение проблем

### Тесты не компилируются
```bash
cd build
cmake .. -DENABLE_TESTS=ON
cmake --build . --target run_all_tests -j$(nproc)
```

### lcov не найден
```bash
sudo apt-get install lcov
```

### Google Test не найден
```bash
sudo apt-get install libgtest-dev
```

## 📝 Добавление новых тестов

1. Создайте файл `tests/my_component_test.cpp`
2. Добавьте в `tests/CMakeLists.txt`:
```cmake
add_spectraforge_test(my_component_test my_component_test.cpp)
```
3. Следуйте AAA паттерну:
```cpp
TEST_F(MyComponentTest, TestName) {
    // Arrange - подготовка
    MyComponent component;
    
    // Act - действие
    component.doSomething();
    
    // Assert - проверка
    EXPECT_TRUE(component.isValid());
}
```

## 📚 Дополнительные ресурсы

- [Полный план покрытия](TEST_COVERAGE_PLAN.md)
- [Google Test документация](https://google.github.io/googletest/)
- [lcov руководство](http://ltp.sourceforge.net/coverage/lcov.php)

---

**Создано:** 2025-10-07  
**Версия:** 1.0.0
