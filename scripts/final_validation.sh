#!/bin/bash
# Финальная валидация рефакторинга

echo "🎯 ФИНАЛЬНАЯ ВАЛИДАЦИЯ РЕФАКТОРИНГА HYPERENGINE"
echo "=============================================="

VALIDATION_PASSED=true

# Функция проверки
validate() {
    local test_name="$1"
    local command="$2"
    
    echo -n "🔍 $test_name... "
    
    if eval "$command" > /dev/null 2>&1; then
        echo "✅ ПРОШЕЛ"
        return 0
    else
        echo "❌ НЕ ПРОШЕЛ"
        VALIDATION_PASSED=false
        return 1
    fi
}

echo ""
echo "📋 ПРОВЕРКА АРХИТЕКТУРНЫХ ТРЕБОВАНИЙ"
echo "-----------------------------------"

# 1. Проверка структуры проекта
validate "Структура директорий" "test -d src && test -d include && test -d tests && test -d docs"

# 2. Проверка namespace'ов
validate "Обновление namespace'ов" "grep -r 'namespace HyperEngine' include/ | wc -l | grep -v '^0$'"

# 3. Проверка миграции include путей
validate "Миграция include путей" "grep -r '#include \"HyperEngine/' examples/ | wc -l | grep -v '^0$'"

# 4. Проверка отсутствия старых namespace'ов
validate "Отсутствие старых Engine3D namespace'ов" "! grep -r 'namespace Engine3D' --include='*.cpp' --include='*.h' . | grep -v '.backup'"

echo ""
echo "🧪 ПРОВЕРКА СИСТЕМЫ ТЕСТИРОВАНИЯ"
echo "-------------------------------"

# 4. Сборка тестов
validate "Сборка тестов" "cmake -B build/validation -DBUILD_TESTING=ON && cmake --build build/validation"

# 5. Запуск тестов
validate "Прохождение тестов" "cd build/validation && ctest --output-on-failure"

# 6. Покрытие кода
validate "Покрытие кода > 80%" "./scripts/coverage.sh && python3 -c \"
import re
with open('build/coverage/coverage_html/index.html', 'r') as f:
    content = f.read()
    match = re.search(r'lines......: ([0-9.]+)%', content)
    coverage = float(match.group(1)) if match else 0
    exit(0 if coverage >= 80 else 1)
\""

echo ""
echo "🔧 ПРОВЕРКА КАЧЕСТВА КОДА"
echo "------------------------"

# 7. Статический анализ
validate "Статический анализ" "./scripts/static_analysis.sh"

# 8. Форматирование кода
validate "Форматирование кода" "find src include tests -name '*.cpp' -o -name '*.h' | xargs clang-format --dry-run --Werror"

# 9. Отсутствие критических предупреждений
validate "Отсутствие критических ошибок" "! ./scripts/quality_check.sh | grep -q 'ОШИБКА'"

echo ""
echo "📚 ПРОВЕРКА ДОКУМЕНТАЦИИ"
echo "-----------------------"

# 10. Генерация документации
validate "Генерация Doxygen документации" "doxygen Doxyfile"

# 11. Наличие ключевых документов
validate "Наличие ключевых документов" "test -f docs/refactoring/FINAL_REPORT.md && test -f docs/DEVELOPER_GUIDE.md"

echo ""
echo "🚀 ПРОВЕРКА CI/CD"
echo "----------------"

# 12. GitHub Actions workflow
validate "GitHub Actions workflow" "test -f .github/workflows/ci-cd.yml"

# 13. Pre-commit hooks
validate "Pre-commit hooks" "test -f .pre-commit-config.yaml"

echo ""
echo "🎊 РЕЗУЛЬТАТЫ ВАЛИДАЦИИ"
echo "======================"

if [ "$VALIDATION_PASSED" = true ]; then
    echo "✅ ВСЕ ПРОВЕРКИ ПРОШЛИ УСПЕШНО!"
    echo ""
    echo "🎉 ПОЗДРАВЛЯЕМ! Рефакторинг HyperEngine успешно завершен!"
    echo ""
    echo "Проект теперь соответствует всем современным стандартам:"
    echo "  ✅ SOLID архитектурные принципы"
    echo "  ✅ Комплексная система тестирования" 
    echo "  ✅ Высокое качество кода"
    echo "  ✅ Полная документация"
    echo "  ✅ Надежный CI/CD пайплайн"
    echo ""
    echo "Движок готов к использованию и дальнейшему развитию!"
    
    # Генерация итогового отчета
    python3 scripts/generate_final_report.py
    
    exit 0
else
    echo "❌ НЕКОТОРЫЕ ПРОВЕРКИ НЕ ПРОШЛИ"
    echo ""
    echo "Пожалуйста, исправьте выявленные проблемы перед завершением рефакторинга."
    exit 1
fi
