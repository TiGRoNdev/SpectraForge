#!/bin/bash
# Быстрая проверка перед коммитом

echo "🚀 Быстрая проверка качества кода..."

# Форматирование
echo "🎨 Проверка форматирования..."
if ! git diff --cached --name-only | grep -E '\.(cpp|h|hpp)$' | xargs clang-format --dry-run --Werror; then
    echo "❌ Ошибки форматирования. Запустите: git diff --cached --name-only | grep -E '\.(cpp|h|hpp)$' | xargs clang-format -i"
    exit 1
fi

# Базовые проверки
echo "🔍 Базовые проверки..."
git diff --cached --name-only | grep -E '\.(cpp|h|hpp)$' | while read file; do
    # Проверка на небезопасные функции
    if grep -n "strcpy\|strcat\|sprintf\|gets" "$file"; then
        echo "❌ Найдены небезопасные функции в $file"
        exit 1
    fi
    
    # Проверка namespace
    if [ "${file##*.}" = "h" ] || [ "${file##*.}" = "hpp" ]; then
        if ! grep -q "namespace HyperEngine" "$file"; then
            echo "⚠️ Файл $file может не использовать namespace HyperEngine"
        fi
    fi
done

echo "✅ Быстрая проверка завершена успешно"
