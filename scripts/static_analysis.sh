#!/bin/bash
# Комплексный статический анализ кода HyperEngine

echo "🔍 Запуск статического анализа кода HyperEngine..."

# Создать директорию для отчетов
mkdir -p build/static-analysis

# Определение операционной системы
OS_TYPE="unknown"
if [[ "$OSTYPE" == "linux-gnu"* ]]; then
    OS_TYPE="linux"
elif [[ "$OSTYPE" == "msys" ]] || [[ "$OSTYPE" == "cygwin" ]]; then
    OS_TYPE="windows"
elif [[ "$OSTYPE" == "darwin"* ]]; then
    OS_TYPE="macos"
fi

echo "🖥️ Обнаружена ОС: $OS_TYPE"

# 1. Clang Static Analyzer
echo "📊 Запуск Clang Static Analyzer..."
if command -v scan-build &> /dev/null; then
    echo "✅ scan-build найден"
    # Попробуем использовать Ninja для лучшей поддержки compile_commands.json
    if command -v ninja &> /dev/null; then
        echo "🥷 Используем Ninja генератор"
        scan-build cmake -B build/static-analysis \
            -G Ninja \
            -DCMAKE_BUILD_TYPE=Debug \
            -DCMAKE_EXPORT_COMPILE_COMMANDS=ON \
            -DCMAKE_TOOLCHAIN_FILE=vcpkg/scripts/buildsystems/vcpkg.cmake
    else
        echo "📦 Используем стандартный генератор"
        scan-build cmake -B build/static-analysis \
            -DCMAKE_BUILD_TYPE=Debug \
            -DCMAKE_EXPORT_COMPILE_COMMANDS=ON \
            -DCMAKE_TOOLCHAIN_FILE=vcpkg/scripts/buildsystems/vcpkg.cmake
    fi

    scan-build --status-bugs cmake --build build/static-analysis
else
    echo "⚠️ scan-build не найден, пропускаем Clang Static Analyzer"
    # Альтернативная генерация compile_commands.json
    if command -v ninja &> /dev/null; then
        cmake -B build/static-analysis \
            -G Ninja \
            -DCMAKE_BUILD_TYPE=Debug \
            -DCMAKE_EXPORT_COMPILE_COMMANDS=ON \
            -DCMAKE_TOOLCHAIN_FILE=vcpkg/scripts/buildsystems/vcpkg.cmake
    else
        cmake -B build/static-analysis \
            -DCMAKE_BUILD_TYPE=Debug \
            -DCMAKE_EXPORT_COMPILE_COMMANDS=ON \
            -DCMAKE_TOOLCHAIN_FILE=vcpkg/scripts/buildsystems/vcpkg.cmake
    fi
fi

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

# 6. Простой анализ паттернов для поиска потенциальных проблем
echo "🔍 Запуск анализа паттернов..."
{
    echo "=== Поиск потенциальных утечек памяти ==="
    grep -rn "new\|malloc\|calloc" src/ include/ 2>/dev/null || echo "Не найдено"
    echo ""

    echo "=== Поиск использования raw pointers ==="
    grep -rn "\*.*=" src/ include/ 2>/dev/null | head -20 || echo "Не найдено"
    echo ""

    echo "=== Поиск TODO и FIXME ==="
    grep -rn "TODO\|FIXME\|HACK\|XXX" src/ include/ 2>/dev/null || echo "Не найдено"
    echo ""

    echo "=== Поиск потенциально небезопасных функций ==="
    grep -rn "strcpy\|strcat\|sprintf\|gets" src/ include/ 2>/dev/null || echo "Не найдено"
} > build/static-analysis/pattern-analysis.txt

# 7. Создание сводного отчета
echo "📋 Создание сводного отчета..."
{
    echo "# Отчет статического анализа кода HyperEngine"
    echo ""
    echo "Дата анализа: $(date)"
    echo "ОС: $OS_TYPE"
    echo ""
    echo "## Выполненные проверки"
    echo ""

    if command -v scan-build &> /dev/null; then
        echo "### Clang Static Analyzer"
        echo "- Статус: ✅ Выполнен"
        echo "- Отчет: Смотрите вывод scan-build"
        echo ""
    else
        echo "### Clang Static Analyzer"
        echo "- Статус: ❌ Не выполнен (scan-build не найден)"
        echo "- Установка: sudo apt install clang-tools (Ubuntu/Debian)"
        echo ""
    fi

    if [ -f build/static-analysis/clang-tidy-fixes.yaml ]; then
        echo "### Clang-Tidy"
        echo "- Статус: ✅ Выполнен"
        echo "- Отчет: build/static-analysis/clang-tidy-fixes.yaml"
        echo ""
    else
        echo "### Clang-Tidy"
        echo "- Статус: ❌ Не выполнен (compile_commands.json не найден или clang-tidy недоступен)"
        echo ""
    fi

    if [ -f build/static-analysis/cppcheck-report.xml ]; then
        echo "### Cppcheck"
        echo "- Статус: ✅ Выполнен"
        echo "- Отчет: build/static-analysis/cppcheck-report.xml"
        echo ""
    else
        echo "### Cppcheck"
        echo "- Статус: ❌ Не выполнен (cppcheck не установлен)"
        echo "- Установка: sudo apt install cppcheck (Ubuntu/Debian)"
        echo ""
    fi

    if [ -f build/static-analysis/iwyu-report.txt ]; then
        echo "### Include What You Use"
        echo "- Статус: ✅ Выполнен"
        echo "- Отчет: build/static-analysis/iwyu-report.txt"
        echo ""
    else
        echo "### Include What You Use"
        echo "- Статус: ❌ Не выполнен (iwyu не установлен)"
        echo "- Установка: sudo apt install iwyu (Ubuntu/Debian)"
        echo ""
    fi

    echo "### Анализ паттернов"
    echo "- Статус: ✅ Выполнен"
    echo "- Отчет: build/static-analysis/pattern-analysis.txt"
    echo ""

    echo "## Рекомендации по установке инструментов"
    echo ""
    case $OS_TYPE in
        "linux")
            echo "### Ubuntu/Debian:"
            echo "```bash"
            echo "sudo apt update"
            echo "sudo apt install clang-tools cppcheck iwyu ninja-build"
            echo "```"
            echo ""
            ;;
        "windows")
            echo "### Windows:"
            echo "```cmd"
            echo "winget install LLVM.LLVM"
            echo "winget install Cppcheck.Cppcheck"
            echo "```"
            echo ""
            ;;
        "macos")
            echo "### macOS:"
            echo "```bash"
            echo "brew install llvm cppcheck include-what-you-use ninja"
            echo "```"
            echo ""
            ;;
    esac

    echo "## Следующие шаги"
    echo ""
    echo "1. Просмотрите все найденные предупреждения"
    echo "2. Исправьте критические ошибки"
    echo "3. Рассмотрите предложения по улучшению"
    echo "4. Установите недостающие инструменты для более полного анализа"
} > build/static-analysis/summary.md

echo "✅ Статический анализ завершен. Отчеты в build/static-analysis/"
echo "📋 Сводный отчет: build/static-analysis/summary.md"
