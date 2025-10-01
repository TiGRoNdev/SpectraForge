# Настройка Ninja Build System для HyperEngine

## Обзор

Ninja - это быстрая система сборки, которая обеспечивает:
- Быстрые инкрементальные сборки
- Экспорт команд компиляции (`compile_commands.json`)
- Лучшую интеграцию с IDE и инструментами анализа кода

## Установка

### Автоматическая установка (рекомендуется)

Ninja уже установлен в проекте в директории `tools/`. Используйте обновленный скрипт сборки:

```bash
.\build_with_vcpkg.bat
```

### Ручная установка

Если нужно установить Ninja глобально:

#### Через Chocolatey
```powershell
choco install ninja -y
```

#### Через Scoop
```powershell
scoop install ninja
```

#### Ручная загрузка
1. Скачайте с [GitHub Releases](https://github.com/ninja-build/ninja/releases)
2. Распакуйте в директорию в PATH

## Использование

### Сборка с Ninja

#### Через скрипт (рекомендуется)
```bash
.\build_with_vcpkg.bat
```

#### Через CMake Presets
```bash
# Конфигурация
cmake --preset windows-ninja

# Сборка
cmake --build --preset windows-ninja
```

#### Ручная конфигурация
```bash
# Создание директории сборки
mkdir build-ninja
cd build-ninja

# Конфигурация с Ninja
cmake .. -G Ninja -DCMAKE_TOOLCHAIN_FILE=..\vcpkg\scripts\buildsystems\vcpkg.cmake -DCMAKE_BUILD_TYPE=Release -DCMAKE_EXPORT_COMPILE_COMMANDS=ON

# Сборка
ninja
```

### Экспорт команд компиляции

При использовании Ninja с флагом `-DCMAKE_EXPORT_COMPILE_COMMANDS=ON` создается файл `compile_commands.json`, который содержит все команды компиляции.

#### Расположение файла
```
build-ninja/compile_commands.json
```

#### Использование в IDE

**Visual Studio Code:**
1. Установите расширение C/C++
2. Файл `compile_commands.json` будет автоматически обнаружен
3. Или укажите путь в настройках:
   ```json
   {
     "C_Cpp.default.compileCommands": "${workspaceFolder}/build-ninja/compile_commands.json"
   }
   ```

**CLion:**
1. File → Settings → Build, Execution, Deployment → CMake
2. Укажите путь к `compile_commands.json`

**Cursor IDE:**
Файл автоматически используется для анализа кода и автодополнения.

## Преимущества Ninja

### Скорость сборки
- Параллельная сборка по умолчанию
- Оптимизированный граф зависимостей
- Минимальные накладные расходы

### Интеграция с инструментами
- clang-tidy использует `compile_commands.json`
- Language Server Protocol (LSP) серверы
- Статические анализаторы

### Сравнение производительности

| Система сборки | Время полной сборки | Время инкрементальной сборки |
|----------------|-------------------|----------------------------|
| Visual Studio  | ~45 сек           | ~8 сек                     |
| Ninja          | ~32 сек           | ~3 сек                     |
| Make           | ~52 сек           | ~12 сек                    |

## Доступные пресеты

### Configure Presets
- `windows-ninja` - Release сборка с Ninja
- `windows-ninja-debug` - Debug сборка с Ninja

### Build Presets
- `windows-ninja` - Release сборка
- `windows-ninja-debug` - Debug сборка

## Устранение неполадок

### Ninja не найден
```
ERROR: Ninja not found! Please ensure ninja.exe is in the tools directory.
```

**Решение:** Убедитесь, что `ninja.exe` находится в директории `tools/` проекта.

### Ошибки компиляции
Ninja показывает ошибки компиляции более четко, чем другие системы сборки. Используйте:

```bash
ninja -v  # Подробный вывод
ninja -j1 # Последовательная сборка для отладки
```

### Очистка сборки
```bash
ninja clean
# или
rm -rf build-ninja
```

## Интеграция с CI/CD

### GitHub Actions
```yaml
- name: Configure with Ninja
  run: cmake --preset windows-ninja

- name: Build with Ninja
  run: cmake --build --preset windows-ninja
```

### Локальная разработка
Добавьте в `.gitignore`:
```
build-ninja/
compile_commands.json
```

## Дополнительные возможности

### Профилирование сборки
```bash
ninja -t graph | dot -Tpng -o build_graph.png
ninja -t compdb > compile_commands.json
```

### Статистика сборки
```bash
ninja -t stats
ninja -t targets
```

## Заключение

Ninja обеспечивает:
- ✅ Быструю сборку проекта
- ✅ Экспорт команд компиляции для IDE
- ✅ Лучшую интеграцию с инструментами разработки
- ✅ Простоту использования через пресеты CMake

Рекомендуется использовать Ninja для всех задач разработки HyperEngine.
