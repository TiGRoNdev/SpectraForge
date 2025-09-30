#!/bin/bash
# Скрипт для установки локальных git hooks для HyperEngine

set -e

echo "🔧 Настройка git hooks для HyperEngine..."
echo "========================================"

# Проверяем, что мы в git репозитории
if [ ! -d ".git" ]; then
    echo "❌ Ошибка: Не найдена директория .git"
    echo "💡 Запустите скрипт из корня git репозитория"
    exit 1
fi

# Создаем директорию hooks если её нет
mkdir -p .git/hooks

# Функция для создания hook'а
create_hook() {
    local hook_name="$1"
    local hook_content="$2"
    local hook_path=".git/hooks/$hook_name"
    
    echo "📝 Создание $hook_name..."
    
    cat > "$hook_path" << EOF
#!/bin/bash
# $hook_name hook для HyperEngine
# Автоматически создан скриптом setup_git_hooks.sh

$hook_content
EOF
    
    chmod +x "$hook_path"
    echo "✅ $hook_name создан и настроен"
}

# Pre-commit hook
PRE_COMMIT_CONTENT='
echo "🚀 Запуск pre-commit проверок..."

# Проверяем наличие staged файлов
if git diff --cached --quiet; then
    echo "ℹ️ Нет изменений для коммита"
    exit 0
fi

# Получаем список измененных C++ файлов
STAGED_FILES=$(git diff --cached --name-only --diff-filter=ACM | grep -E "\.(cpp|h|hpp|c|cc|cxx)$" || true)

if [ -z "$STAGED_FILES" ]; then
    echo "ℹ️ Нет C++ файлов для проверки"
    exit 0
fi

echo "🔍 Проверка форматирования C++ файлов..."

# Проверяем форматирование
FORMAT_ISSUES=0
for file in $STAGED_FILES; do
    if [ -f "$file" ]; then
        # Проверяем форматирование
        if ! clang-format --dry-run --Werror --style=file "$file" >/dev/null 2>&1; then
            echo "❌ Ошибка форматирования в файле: $file"
            FORMAT_ISSUES=1
        fi
    fi
done

if [ $FORMAT_ISSUES -eq 1 ]; then
    echo ""
    echo "💡 Для исправления форматирования выполните:"
    echo "   git diff --cached --name-only | grep -E \"\.(cpp|h|hpp|c|cc|cxx)$\" | xargs clang-format -i"
    echo "   git add ."
    echo ""
    echo "🔧 Или используйте автоматическое исправление:"
    echo "   pre-commit run clang-format --files $STAGED_FILES"
    exit 1
fi

# Быстрые проверки безопасности
echo "🔒 Проверка безопасности..."
SECURITY_ISSUES=0

for file in $STAGED_FILES; do
    if [ -f "$file" ]; then
        # Проверка на небезопасные функции
        if grep -n "strcpy\|strcat\|sprintf\|gets" "$file"; then
            echo "❌ Найдены небезопасные функции в $file"
            SECURITY_ISSUES=1
        fi
        
        # Проверка на TODO без номеров issue
        if grep -n "TODO\|FIXME" "$file" | grep -v "#[0-9]"; then
            echo "⚠️ TODO/FIXME без номера issue в $file"
        fi
    fi
done

if [ $SECURITY_ISSUES -eq 1 ]; then
    echo "❌ Обнаружены проблемы безопасности. Исправьте их перед коммитом."
    exit 1
fi

echo "✅ Все проверки прошли успешно!"
'

# Pre-push hook
PRE_PUSH_CONTENT='
echo "🚀 Запуск pre-push проверок..."

# Проверяем, что есть коммиты для push
if [ -z "$(git log @{u}.. --oneline 2>/dev/null)" ]; then
    echo "ℹ️ Нет новых коммитов для push"
    exit 0
fi

# Запускаем быструю проверку качества
if [ -f "scripts/pre_commit_check.sh" ]; then
    echo "🔍 Запуск быстрой проверки качества..."
    if ! bash scripts/pre_commit_check.sh; then
        echo "❌ Проверка качества не прошла"
        exit 1
    fi
fi

# Проверяем, что проект собирается
if [ -d "build" ]; then
    echo "🔨 Проверка сборки..."
    cd build
    if ! cmake --build . --config Release --parallel 2 >/dev/null; then
        echo "❌ Проект не собирается"
        cd ..
        exit 1
    fi
    cd ..
    echo "✅ Сборка прошла успешно"
fi

echo "✅ Все pre-push проверки прошли успешно!"
'

# Commit-msg hook
COMMIT_MSG_CONTENT='
# Проверка формата commit message
commit_regex="^(feat|fix|docs|style|refactor|test|chore|perf|ci|build|revert)(\(.+\))?: .{1,50}"

if ! grep -qE "$commit_regex" "$1"; then
    echo "❌ Неверный формат commit message!"
    echo ""
    echo "📋 Используйте conventional commits формат:"
    echo "   <type>[optional scope]: <description>"
    echo ""
    echo "🏷️ Доступные типы:"
    echo "   feat:     новая функциональность"
    echo "   fix:      исправление бага"
    echo "   docs:     изменения в документации"
    echo "   style:    форматирование, отсутствующие точки с запятой и т.д."
    echo "   refactor: рефакторинг кода"
    echo "   test:     добавление тестов"
    echo "   chore:    обновление задач сборки, настроек и т.д."
    echo "   perf:     улучшение производительности"
    echo "   ci:       изменения в CI/CD"
    echo "   build:    изменения в системе сборки"
    echo "   revert:   откат изменений"
    echo ""
    echo "📝 Примеры:"
    echo "   feat: добавить поддержку 4D рендеринга"
    echo "   fix(vulkan): исправить утечку памяти в буферах"
    echo "   docs: обновить README с инструкциями по сборке"
    exit 1
fi
'

# Создаем hooks
create_hook "pre-commit" "$PRE_COMMIT_CONTENT"
create_hook "pre-push" "$PRE_PUSH_CONTENT"
create_hook "commit-msg" "$COMMIT_MSG_CONTENT"

echo ""
echo "🎉 Git hooks успешно настроены!"
echo ""
echo "📋 Установленные hooks:"
echo "   • pre-commit:  проверка форматирования и безопасности"
echo "   • pre-push:    проверка качества и сборки"
echo "   • commit-msg:  проверка формата commit message"
echo ""
echo "💡 Для отключения hook'ов используйте:"
echo "   git commit --no-verify"
echo "   git push --no-verify"
echo ""
echo "🔧 Для обновления hooks запустите этот скрипт снова"
