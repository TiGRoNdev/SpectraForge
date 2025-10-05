# 🚀 Настройка CI/CD пайплайна SpectraForge

## 📋 Обзор

Данный документ описывает полностью настроенный CI/CD пайплайн для проекта SpectraForge, включающий автоматическое тестирование, анализ качества кода, сборку и развертывание.

## 🏗️ Структура пайплайна

### 1. Основной CI/CD workflow (`.github/workflows/ci-cd.yml`)

**Триггеры:**
- Push в ветки: `main`, `develop`, `refactoring/*`
- Pull requests в ветки: `main`, `develop`
- Создание релизов

**Этапы:**
1. **Проверка качества кода** - clang-format, clang-tidy, cppcheck
2. **Сборка и тестирование** - на Ubuntu, Windows, macOS
3. **Тесты производительности** - бенчмарки с трекингом
4. **Анализ безопасности** - Trivy сканер

### 2. Расширенная проверка качества (`.github/workflows/quality-checks.yml`)

**Дополнительные проверки:**
- Детальный анализ кода с множественными инструментами
- Анализ производительности компиляции
- Проверка зависимостей и циклических включений
- Анализ include guards и namespace консистентности

### 3. Релизный пайплайн (`.github/workflows/release.yml`)

**Функциональность:**
- Автоматическая сборка релизов для всех платформ
- Создание Docker образов
- Генерация release notes из CHANGELOG.md
- Обновление документации
- Уведомления в Discord

### 4. Анализ безопасности CodeQL (`.github/workflows/codeql.yml`)

**Возможности:**
- Стандартные запросы безопасности
- Кастомные запросы для графических API
- Обнаружение утечек ресурсов
- Анализ небезопасных операций с памятью

## 🔧 Конфигурационные файлы

### Анализаторы кода

| Файл | Назначение |
|------|-----------|
| `.clang-format` | Форматирование кода |
| `.clang-tidy` | Статический анализ |
| `cppcheck.cfg` | Настройки cppcheck |
| `.pre-commit-config.yaml` | Pre-commit hooks |

### CI/CD конфигурация

| Файл | Назначение |
|------|-----------|
| `.github/dependabot.yml` | Автообновление зависимостей |
| `.github/codeql/codeql-config.yml` | Настройки CodeQL |
| `.github/workflows/cache-config.yml` | Конфигурация кэширования |

### Скрипты проверки

| Скрипт | Назначение |
|--------|-----------|
| `scripts/check_copyright.sh` | Проверка авторских прав |
| `scripts/check_include_guards.sh` | Проверка include guards |
| `scripts/check_namespaces.sh` | Проверка namespace |

## 📊 Метрики и мониторинг

### Покрытие кода
- Автоматический сбор метрик покрытия на Linux Debug
- Загрузка в Codecov
- Фильтрация системных файлов и тестов

### Производительность
- Бенчмарки с Google Benchmark
- Автоматическое сравнение с предыдущими результатами
- Предупреждения при ухудшении на 150%

### Безопасность
- Сканирование уязвимостей с Trivy
- Загрузка результатов в GitHub Security
- Кастомные CodeQL запросы для графических API

## 🔐 Секреты и переменные

### Необходимые секреты

```yaml
GITHUB_TOKEN: # Автоматически предоставляется
DOCKER_USERNAME: # Для публикации Docker образов
DOCKER_PASSWORD: # Пароль Docker Hub
DISCORD_WEBHOOK_URL: # Для уведомлений (опционально)
```

### Переменные окружения

```yaml
BUILD_TYPE: Release
VCPKG_BINARY_SOURCES: "clear;x-gha,readwrite"
CPPCHECK_VERSION: "2.12"
CLANG_VERSION: "16"
```

## 🚀 Использование

### Настройка для нового проекта

1. **Копирование файлов:**
   ```bash
   cp -r .github/ /path/to/new/project/
   cp .clang-* .pre-commit-config.yaml cppcheck.cfg /path/to/new/project/
   cp -r scripts/ /path/to/new/project/
   ```

2. **Настройка секретов в GitHub:**
   - Перейти в Settings → Secrets and variables → Actions
   - Добавить необходимые секреты

3. **Активация pre-commit hooks:**
   ```bash
   pip install pre-commit
   pre-commit install
   ```

### Создание релиза

1. **Автоматический релиз:**
   - Создать tag: `git tag v1.0.0`
   - Push tag: `git push origin v1.0.0`
   - Создать GitHub Release

2. **Ручной релиз:**
   - Перейти в Actions → Release Build and Deploy
   - Нажать "Run workflow"
   - Указать версию и тип релиза

### Мониторинг качества

1. **Просмотр результатов:**
   - GitHub Actions для статуса сборки
   - Security tab для уязвимостей
   - Pull requests для проверок качества

2. **Настройка уведомлений:**
   - Включить email уведомления в настройках
   - Настроить Discord webhook (опционально)

## 🔄 Процесс разработки

### Workflow разработчика

1. **Создание ветки:**
   ```bash
   git checkout -b feature/my-feature
   ```

2. **Разработка с автопроверками:**
   - Pre-commit hooks проверяют код автоматически
   - Локальное тестирование перед push

3. **Создание Pull Request:**
   - Автоматически запускаются все проверки
   - Обязательные проверки должны пройти
   - Code review от команды

4. **Мерж в main:**
   - Автоматическая сборка и тестирование
   - Обновление метрик покрытия
   - Уведомления команде

### Обновление зависимостей

Dependabot автоматически создает PR для обновления:
- GitHub Actions (понедельник)
- vcpkg пакеты (вторник) 
- Docker образы (среда)
- Git submodules (четверг)

## 📈 Оптимизация производительности

### Кэширование

Настроено кэширование для:
- vcpkg зависимостей
- CMake конфигурации
- Инструментов анализа
- Компилятора

### Параллелизация

- Матричные сборки для разных платформ
- Параллельная компиляция (`--parallel`)
- Параллельное тестирование (`--parallel`)

## 🐛 Отладка проблем

### Частые проблемы

1. **Ошибки формирования:**
   ```bash
   # Локальная проверка
   clang-format --dry-run --Werror --style=file src/**/*.cpp
   ```

2. **Проблемы с зависимостями:**
   ```bash
   # Очистка кэша vcpkg
   rm -rf vcpkg_installed/
   ```

3. **Ошибки CodeQL:**
   - Проверить синтаксис кастомных запросов
   - Обновить qlpack.yml версии

### Логи и диагностика

- Подробные логи в GitHub Actions
- Артефакты с результатами анализа
- Метрики производительности компиляции

## 📚 Дополнительные ресурсы

- [GitHub Actions Documentation](https://docs.github.com/en/actions)
- [CodeQL Documentation](https://codeql.github.com/docs/)
- [vcpkg Documentation](https://vcpkg.io/en/getting-started.html)
- [clang-tidy Checks](https://clang.llvm.org/extra/clang-tidy/checks/list.html)

---

*Этот CI/CD пайплайн обеспечивает высокое качество кода, безопасность и надежность разработки SpectraForge.*
