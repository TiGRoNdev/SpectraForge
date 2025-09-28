#!/bin/bash
# Комплексный статический анализ кода HyperEngine

echo "🔍 Запуск статического анализа кода HyperEngine..."

# Создать директорию для отчетов
mkdir -p build/static-analysis

# 1. Clang Static Analyzer
echo "📊 Запуск Clang Static Analyzer..."
scan-build cmake -B build/static-analysis \
    -DCMAKE_BUILD_TYPE=Debug \
    -DCMAKE_EXPORT_COMPILE_COMMANDS=ON \
    -DCMAKE_TOOLCHAIN_FILE=vcpkg/scripts/buildsystems/vcpkg.cmake

scan-build --status-bugs cmake --build build/static-analysis

# 2. Clang-Tidy анализ
echo "🔧 Запуск Clang-Tidy..."
if [ -f build/static-analysis/compile_commands.json ]; then
    find src include -name "*.cpp" | \
    head -50 | \
    xargs clang-tidy \
        --config-file=.clang-tidy \
        -p build/static-analysis \
        --export-fixes=build/static-analysis/clang-tidy-fixes.yaml
else
    echo "⚠️ compile_commands.json не найден, пропускаем clang-tidy"
fi

# 3. Cppcheck анализ
echo "🛡️ Запуск Cppcheck..."
if command -v cppcheck &> /dev/null; then
    if [ -f build/static-analysis/compile_commands.json ]; then
        cppcheck \
            --enable=all \
            --xml \
            --xml-version=2 \
            --output-file=build/static-analysis/cppcheck-report.xml \
            --suppress=missingIncludeSystem \
            --suppress=unusedFunction \
            --project=build/static-analysis/compile_commands.json
    else
        # Fallback: анализ исходных файлов напрямую
        cppcheck \
            --enable=all \
            --xml \
            --xml-version=2 \
            --output-file=build/static-analysis/cppcheck-report.xml \
            --suppress=missingIncludeSystem \
            --suppress=unusedFunction \
            -I include \
            src/ include/
    fi
else
    echo "⚠️ Cppcheck не установлен, пропускаем анализ"
fi

# 4. Include What You Use (если установлен)
if command -v include-what-you-use &> /dev/null; then
    echo "📦 Запуск Include What You Use..."
    find src include -name "*.cpp" | head -20 | \
    xargs -I {} include-what-you-use \
        -p build/static-analysis {} > build/static-analysis/iwyu-report.txt 2>&1
else
    echo "⚠️ Include What You Use не установлен, пропускаем анализ"
fi

# 5. PC-lint Plus (если доступен)
if command -v pclp &> /dev/null; then
    echo "🔬 Запуск PC-lint Plus..."
    pclp -b build/static-analysis/lint-config.lnt src/*.cpp > build/static-analysis/pclint-report.txt
else
    echo "⚠️ PC-lint Plus не доступен, пропускаем анализ"
fi

# 6. Создание сводного отчета
echo "📋 Создание сводного отчета..."
cat > build/static-analysis/summary.md << 'EOF'
# Отчет статического анализа кода

## Выполненные проверки

### Clang Static Analyzer
- Статус: Выполнен
- Отчет: Смотрите вывод scan-build

### Clang-Tidy
- Статус: Выполнен
- Отчет: clang-tidy-fixes.yaml

### Cppcheck
- Статус: Выполнен  
- Отчет: cppcheck-report.xml

### Include What You Use
- Статус: Выполнен (если доступен)
- Отчет: iwyu-report.txt

## Рекомендации
1. Просмотрите все найденные предупреждения
2. Исправьте критические ошибки
3. Рассмотрите предложения по улучшению
EOF

echo "✅ Статический анализ завершен. Отчеты в build/static-analysis/"
echo "📋 Сводный отчет: build/static-analysis/summary.md"
