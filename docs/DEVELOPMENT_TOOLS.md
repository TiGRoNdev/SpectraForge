# Инструменты разработки HyperEngine

## Обзор

Этот документ описывает инструменты и скрипты, доступные для разработки проекта HyperEngine. Все инструменты настроены для обеспечения высокого качества кода и автоматизации рутинных задач.

## Содержание

- [Скрипты качества кода](#скрипты-качества-кода)
- [Инструменты форматирования](#инструменты-форматирования)
- [Git hooks](#git-hooks)
- [CI/CD интеграция](#cicd-интеграция)
- [IDE настройки](#ide-настройки)
- [Устранение неполадок](#устранение-неполадок)

## Скрипты качества кода

### Комплексная проверка качества

**Файл:** `scripts/quality_check.sh`

Выполняет полную проверку качества кода проекта.

```bash
# Запуск полной проверки
./scripts/quality_check.sh
```

**Что проверяется:**
- ✅ Форматирование кода (clang-format)
- ✅ Статический анализ (clang-tidy, cppcheck)
- ✅ Сборка проекта (CMake)
- ✅ Запуск тестов (CTest)
- ✅ Покрытие кода (gcov/lcov)
- ✅ Генерация документации (Doxygen)
- ✅ Проверка безопасности
- ✅ Соответствие стандартам кодирования

**Отчеты сохраняются в:** `build/quality-reports/`

### Быстрая проверка перед коммитом

**Файл:** `scripts/pre_commit_check.sh`

Выполняет быструю проверку измененных файлов.

```bash
# Быстрая проверка
./scripts/pre_commit_check.sh
```

**Что проверяется:**
- 🎨 Форматирование измененных файлов
- 🔒 Проверка на небезопасные функции
- 📋 Проверка namespace'ов
- ⚡ Базовые проверки безопасности

## Инструменты форматирования

### Автоматическое форматирование

**Windows:** `scripts/format_code.bat`
**Linux/macOS:** `scripts/format_code.sh` (создается автоматически)

```bash
# Windows
scripts\format_code.bat

# Linux/macOS
./scripts/format_code.sh
```

**Возможности:**
- 🎨 Форматирование всех C++ файлов
- 📊 Отчет о количестве обработанных файлов
- ✅ Проверка результата форматирования
- 📋 Сохранение отчетов

### Ручное форматирование

```bash
# Форматирование конкретного файла
clang-format -i --style=file src/MyFile.cpp

# Проверка форматирования без изменений
clang-format --dry-run --Werror --style=file src/MyFile.cpp

# Форматирование всех файлов в директории
find src -name "*.cpp" -o -name "*.h" | xargs clang-format -i --style=file
```

## Git Hooks

### Установка локальных hooks

**Linux/macOS:**
```bash
./scripts/setup_git_hooks.sh
```

**Windows:**
```bash
scripts\setup_git_hooks.bat
```

### Доступные hooks

#### Pre-commit Hook
Выполняется перед каждым коммитом:
- 🔍 Проверка форматирования staged файлов
- 🔒 Проверка на небезопасные функции
- 📋 Проверка TODO/FIXME с номерами issue
- ⚠️ Проверка namespace'ов

#### Pre-push Hook
Выполняется перед push:
- 🚀 Запуск быстрой проверки качества
- 🔨 Проверка сборки проекта
- ✅ Базовая валидация

#### Commit-msg Hook
Проверяет формат commit message:
- 📝 Соответствие Conventional Commits
- 🏷️ Правильные типы коммитов
- 📏 Длина сообщения

**Пример правильного commit message:**
```
feat: добавить поддержку 4D рендеринга
fix(vulkan): исправить утечку памяти в буферах
docs: обновить README с инструкциями по сборке
```

### Обход hooks (при необходимости)

```bash
# Обход pre-commit hook
git commit --no-verify -m "commit message"

# Обход pre-push hook
git push --no-verify
```

## CI/CD Интеграция

### GitHub Actions Workflows

#### Основной CI Pipeline
**Файл:** `.github/workflows/ci.yml`

**Этапы:**
1. **Code Quality Check** - проверка качества кода
2. **Build and Test** - сборка и тестирование на разных платформах

#### Проверка качества кода
**Файл:** `.github/workflows/quality-checks.yml`

**Включает:**
- 🔍 Детальный анализ кода
- ⚡ Проверка производительности компиляции
- 📊 Анализ зависимостей

### Pre-commit Integration

Проект использует pre-commit framework:

```bash
# Установка pre-commit
pip install pre-commit

# Установка hooks
pre-commit install

# Запуск всех проверок
pre-commit run --all-files

# Обновление hooks
pre-commit autoupdate
```

**Конфигурация:** `.pre-commit-config.yaml`

## IDE Настройки

### Visual Studio Code

**Настройки:** `.vscode/settings.json`

**Возможности:**
- 🎨 Автоматическое форматирование при сохранении
- 🔍 Интеграция с clang-tidy
- 📝 IntelliSense для C++
- 🔧 Настройки CMake

**Рекомендуемые расширения:** `.vscode/extensions.json`
- C/C++ Extension Pack
- CMake Tools
- Clang-Format
- GitLens

### EditorConfig

**Файл:** `.editorconfig`

Обеспечивает единообразное форматирование во всех редакторах:
- 📏 Отступы: 4 пробела для C++
- 📄 Кодировка: UTF-8
- 🔚 Окончания строк: LF
- ✂️ Удаление trailing whitespace

## Статический анализ

### Clang-Tidy

```bash
# Анализ всех файлов
find src -name "*.cpp" | xargs clang-tidy --config-file=.clang-tidy

# Анализ с исправлениями
clang-tidy --fix --config-file=.clang-tidy src/MyFile.cpp
```

**Конфигурация:** `.clang-tidy`

### Cppcheck

```bash
# Базовый анализ
cppcheck --enable=all --std=c++20 src/ include/

# С подавлением предупреждений
cppcheck --enable=all --std=c++20 --suppress-xml=cppcheck.cfg src/ include/
```

**Конфигурация:** `cppcheck.cfg`

## Документация

### Doxygen

```bash
# Генерация документации
doxygen Doxyfile

# Просмотр документации
open docs/generated/html/index.html  # macOS
start docs/generated/html/index.html # Windows
```

**Конфигурация:** `Doxyfile`

## Покрытие кода

### Генерация отчета покрытия

```bash
# Запуск скрипта покрытия
./scripts/coverage.sh

# Просмотр HTML отчета
open build/coverage/coverage_html/index.html
```

**Требования:**
- Сборка с флагами покрытия
- Установленные gcov/lcov

## Устранение неполадок

### Проблемы с форматированием

**Проблема:** clang-format не найден
```bash
# Ubuntu/Debian
sudo apt-get install clang-format

# macOS
brew install clang-format

# Windows (через vcpkg)
vcpkg install llvm[clang-format]:x64-windows
```

**Проблема:** Ошибки форматирования в CI
```bash
# Локальная проверка
clang-format --dry-run --Werror --style=file src/**/*.cpp

# Исправление
clang-format -i --style=file src/**/*.cpp
```

### Проблемы с pre-commit

**Проблема:** Pre-commit hooks не работают
```bash
# Переустановка hooks
pre-commit uninstall
pre-commit install

# Проверка конфигурации
pre-commit validate-config
```

**Проблема:** Медленная работа pre-commit
```bash
# Очистка кэша
pre-commit clean

# Запуск только для измененных файлов
pre-commit run
```

### Проблемы со сборкой

**Проблема:** CMake не находит зависимости
```bash
# Обновление vcpkg
git -C vcpkg pull
./vcpkg/bootstrap-vcpkg.bat  # Windows
./vcpkg/bootstrap-vcpkg.sh   # Linux/macOS

# Переустановка зависимостей
./vcpkg/vcpkg install --recurse
```

**Проблема:** Ошибки компиляции после форматирования
```bash
# Проверка синтаксиса
clang++ -fsyntax-only -std=c++20 src/MyFile.cpp

# Полная пересборка
rm -rf build/
cmake -B build -DCMAKE_TOOLCHAIN_FILE=vcpkg/scripts/buildsystems/vcpkg.cmake
cmake --build build
```

## Интеграция с внешними инструментами

### SonarQube

**Конфигурация:** `sonar-project.properties`

```bash
# Анализ с SonarQube
sonar-scanner
```

### Docker

**Файлы:** `Dockerfile`, `docker-compose.yml`

```bash
# Сборка в Docker
docker-compose build

# Запуск тестов в контейнере
docker-compose run --rm test
```

## Лучшие практики

### Ежедневная разработка

1. **Перед началом работы:**
   ```bash
   git pull
   pre-commit run --all-files
   ```

2. **Во время разработки:**
   - Используйте автоформатирование в IDE
   - Запускайте быстрые проверки регулярно
   - Следите за предупреждениями статического анализа

3. **Перед коммитом:**
   ```bash
   ./scripts/pre_commit_check.sh
   git add .
   git commit -m "feat: описание изменений"
   ```

4. **Перед push:**
   ```bash
   ./scripts/quality_check.sh
   git push
   ```

### Командная работа

- 📋 Используйте conventional commits
- 🔍 Проводите code review
- 🧪 Пишите тесты для нового кода
- 📚 Обновляйте документацию
- 🔧 Следите за метриками качества

## Обновления инструментов

### Обновление pre-commit hooks

```bash
pre-commit autoupdate
```

### Обновление clang-format

```bash
# Проверка версии
clang-format --version

# Обновление через package manager
sudo apt-get update && sudo apt-get upgrade clang-format  # Ubuntu
brew upgrade clang-format  # macOS
```

### Обновление конфигураций

Регулярно проверяйте обновления:
- `.clang-format` - стиль форматирования
- `.clang-tidy` - правила статического анализа
- `.pre-commit-config.yaml` - версии hooks
- `cppcheck.cfg` - настройки cppcheck

---

*Версия документа: 1.0*  
*Последнее обновление: 2024*
