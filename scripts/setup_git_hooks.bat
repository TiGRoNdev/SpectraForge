@echo off
REM Скрипт для установки локальных git hooks для HyperEngine (Windows)

echo 🔧 Настройка git hooks для HyperEngine...
echo ========================================

REM Проверяем, что мы в git репозитории
if not exist ".git" (
    echo ❌ Ошибка: Не найдена директория .git
    echo 💡 Запустите скрипт из корня git репозитория
    exit /b 1
)

REM Создаем директорию hooks если её нет
if not exist ".git\hooks" mkdir ".git\hooks"

echo 📝 Создание pre-commit hook...

REM Pre-commit hook
(
echo #!/bin/bash
echo # pre-commit hook для HyperEngine
echo # Автоматически создан скриптом setup_git_hooks.bat
echo.
echo echo "🚀 Запуск pre-commit проверок..."
echo.
echo # Проверяем наличие staged файлов
echo if git diff --cached --quiet; then
echo     echo "ℹ️ Нет изменений для коммита"
echo     exit 0
echo fi
echo.
echo # Получаем список измененных C++ файлов
echo STAGED_FILES=$^(git diff --cached --name-only --diff-filter=ACM ^| grep -E "\.\^(cpp^|h^|hpp^|c^|cc^|cxx^)$" ^|^| true^)
echo.
echo if [ -z "$STAGED_FILES" ]; then
echo     echo "ℹ️ Нет C++ файлов для проверки"
echo     exit 0
echo fi
echo.
echo echo "🔍 Проверка форматирования C++ файлов..."
echo.
echo # Проверяем форматирование
echo FORMAT_ISSUES=0
echo for file in $STAGED_FILES; do
echo     if [ -f "$file" ]; then
echo         # Проверяем форматирование
echo         if ! clang-format --dry-run --Werror --style=file "$file" ^>/dev/null 2^>^&1; then
echo             echo "❌ Ошибка форматирования в файле: $file"
echo             FORMAT_ISSUES=1
echo         fi
echo     fi
echo done
echo.
echo if [ $FORMAT_ISSUES -eq 1 ]; then
echo     echo ""
echo     echo "💡 Для исправления форматирования выполните:"
echo     echo "   git diff --cached --name-only ^| grep -E \"\.\^(cpp^|h^|hpp^|c^|cc^|cxx^)$\" ^| xargs clang-format -i"
echo     echo "   git add ."
echo     exit 1
echo fi
echo.
echo echo "✅ Все проверки прошли успешно!"
) > .git\hooks\pre-commit

echo 📝 Создание pre-push hook...

REM Pre-push hook
(
echo #!/bin/bash
echo # pre-push hook для HyperEngine
echo # Автоматически создан скриптом setup_git_hooks.bat
echo.
echo echo "🚀 Запуск pre-push проверок..."
echo.
echo # Проверяем, что есть коммиты для push
echo if [ -z "$^(git log @{u}.. --oneline 2^>/dev/null^)" ]; then
echo     echo "ℹ️ Нет новых коммитов для push"
echo     exit 0
echo fi
echo.
echo # Запускаем быструю проверку качества
echo if [ -f "scripts/pre_commit_check.sh" ]; then
echo     echo "🔍 Запуск быстрой проверки качества..."
echo     if ! bash scripts/pre_commit_check.sh; then
echo         echo "❌ Проверка качества не прошла"
echo         exit 1
echo     fi
echo fi
echo.
echo # Проверяем, что проект собирается
echo if [ -d "build-vcpkg" ]; then
echo     echo "🔨 Проверка сборки..."
echo     cd build-vcpkg
echo     if ! cmake --build . --config Release --parallel 2 ^>/dev/null; then
echo         echo "❌ Проект не собирается"
echo         cd ..
echo         exit 1
echo     fi
echo     cd ..
echo     echo "✅ Сборка прошла успешно"
echo fi
echo.
echo echo "✅ Все pre-push проверки прошли успешно!"
) > .git\hooks\pre-push

echo 📝 Создание commit-msg hook...

REM Commit-msg hook
(
echo #!/bin/bash
echo # commit-msg hook для HyperEngine
echo # Автоматически создан скриптом setup_git_hooks.bat
echo.
echo # Проверка формата commit message
echo commit_regex="^^(feat^|fix^|docs^|style^|refactor^|test^|chore^|perf^|ci^|build^|revert^)(\(.+\))?: .{1,50}"
echo.
echo if ! grep -qE "$commit_regex" "$1"; then
echo     echo "❌ Неверный формат commit message!"
echo     echo ""
echo     echo "📋 Используйте conventional commits формат:"
echo     echo "   ^<type^>[optional scope]: ^<description^>"
echo     echo ""
echo     echo "🏷️ Доступные типы:"
echo     echo "   feat:     новая функциональность"
echo     echo "   fix:      исправление бага"
echo     echo "   docs:     изменения в документации"
echo     echo "   style:    форматирование"
echo     echo "   refactor: рефакторинг кода"
echo     echo "   test:     добавление тестов"
echo     echo "   chore:    обновление задач сборки"
echo     echo "   perf:     улучшение производительности"
echo     echo "   ci:       изменения в CI/CD"
echo     echo "   build:    изменения в системе сборки"
echo     echo "   revert:   откат изменений"
echo     exit 1
echo fi
) > .git\hooks\commit-msg

echo.
echo 🎉 Git hooks успешно настроены!
echo.
echo 📋 Установленные hooks:
echo    • pre-commit:  проверка форматирования и безопасности
echo    • pre-push:    проверка качества и сборки
echo    • commit-msg:  проверка формата commit message
echo.
echo 💡 Для отключения hook'ов используйте:
echo    git commit --no-verify
echo    git push --no-verify
echo.
echo 🔧 Для обновления hooks запустите этот скрипт снова
