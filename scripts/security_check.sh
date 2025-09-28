#!/bin/bash
# Проверка безопасности кода HyperEngine

echo "🔒 Запуск проверки безопасности HyperEngine..."

# Создать директорию для отчетов
mkdir -p build/security-reports

# Счетчики
SECURITY_ISSUES=0
WARNINGS=0

# Функция для логирования
log_security_issue() {
    echo "🚨 ПРОБЛЕМА БЕЗОПАСНОСТИ: $1"
    ((SECURITY_ISSUES++))
}

log_warning() {
    echo "⚠️ ПРЕДУПРЕЖДЕНИЕ: $1" 
    ((WARNINGS++))
}

log_success() {
    echo "✅ $1"
}

log_info() {
    echo "ℹ️ $1"
}

# 1. Проверка на небезопасные функции C
echo ""
echo "🔍 Проверка небезопасных функций C..."
UNSAFE_FUNCTIONS=("strcpy" "strcat" "sprintf" "gets" "scanf" "strncpy" "strncat")
UNSAFE_FOUND=false

for func in "${UNSAFE_FUNCTIONS[@]}"; do
    if grep -r "\b$func\s*(" src/ include/ 2>/dev/null | grep -v "// SAFE:" > build/security-reports/unsafe-functions.log; then
        log_security_issue "Найдена небезопасная функция: $func"
        UNSAFE_FOUND=true
    fi
done

if [ "$UNSAFE_FOUND" = false ]; then
    log_success "Небезопасные функции C не обнаружены"
else
    echo "📋 Детали в build/security-reports/unsafe-functions.log"
fi

# 2. Проверка на переполнение буфера
echo ""
echo "🛡️ Проверка потенциальных переполнений буфера..."
if grep -r "char\s*\w*\[" src/ include/ 2>/dev/null > build/security-reports/buffer-arrays.log; then
    log_warning "Найдены массивы фиксированного размера. Проверьте на переполнение буфера"
    echo "📋 Детали в build/security-reports/buffer-arrays.log"
else
    log_success "Массивы фиксированного размера не найдены"
fi

# 3. Проверка на утечки памяти (поиск new без delete)
echo ""
echo "💾 Проверка управления памятью..."
NEW_COUNT=$(grep -r "\bnew\s" src/ include/ 2>/dev/null | wc -l)
DELETE_COUNT=$(grep -r "\bdelete\s" src/ include/ 2>/dev/null | wc -l)

if [ "$NEW_COUNT" -gt 0 ]; then
    if [ "$NEW_COUNT" -ne "$DELETE_COUNT" ]; then
        log_warning "Несоответствие количества new ($NEW_COUNT) и delete ($DELETE_COUNT)"
    fi
    
    # Проверка на использование умных указателей
    SMART_PTR_COUNT=$(grep -r "std::\(unique_ptr\|shared_ptr\|weak_ptr\)" src/ include/ 2>/dev/null | wc -l)
    if [ "$SMART_PTR_COUNT" -lt "$NEW_COUNT" ]; then
        log_warning "Рекомендуется использовать умные указатели вместо raw new/delete"
    else
        log_success "Используются умные указатели для управления памятью"
    fi
else
    log_success "Прямое использование new/delete не обнаружено"
fi

# 4. Проверка на SQL инъекции (если есть SQL код)
echo ""
echo "💉 Проверка на SQL инъекции..."
if grep -r "SELECT\|INSERT\|UPDATE\|DELETE" src/ include/ 2>/dev/null | grep -v "prepared\|bind" > build/security-reports/sql-queries.log; then
    log_warning "Найдены SQL запросы. Убедитесь в использовании prepared statements"
    echo "📋 Детали в build/security-reports/sql-queries.log"
else
    log_success "Потенциальные SQL инъекции не обнаружены"
fi

# 5. Проверка на небезопасные приведения типов
echo ""
echo "🔄 Проверка небезопасных приведений типов..."
if grep -r "reinterpret_cast\|const_cast\|(.*\*)" src/ include/ 2>/dev/null > build/security-reports/unsafe-casts.log; then
    log_warning "Найдены потенциально небезопасные приведения типов"
    echo "📋 Детали в build/security-reports/unsafe-casts.log"
else
    log_success "Небезопасные приведения типов не обнаружены"
fi

# 6. Проверка на использование временных файлов
echo ""
echo "📁 Проверка безопасности временных файлов..."
if grep -r "tmpnam\|tempnam\|mktemp\|/tmp/" src/ include/ 2>/dev/null > build/security-reports/temp-files.log; then
    log_warning "Найдено использование временных файлов. Проверьте безопасность"
    echo "📋 Детали в build/security-reports/temp-files.log"
else
    log_success "Небезопасное использование временных файлов не обнаружено"
fi

# 7. Проверка на жестко закодированные пароли/ключи
echo ""
echo "🔑 Проверка на жестко закодированные секреты..."
SECRET_PATTERNS=("password\s*=" "key\s*=" "secret\s*=" "token\s*=" "api_key")
SECRETS_FOUND=false

for pattern in "${SECRET_PATTERNS[@]}"; do
    if grep -ri "$pattern" src/ include/ 2>/dev/null | grep -v "// TODO:\|// FIXME:" > build/security-reports/hardcoded-secrets.log; then
        log_security_issue "Найдены потенциально жестко закодированные секреты"
        SECRETS_FOUND=true
        break
    fi
done

if [ "$SECRETS_FOUND" = false ]; then
    log_success "Жестко закодированные секреты не обнаружены"
else
    echo "📋 Детали в build/security-reports/hardcoded-secrets.log"
fi

# 8. Проверка на использование небезопасных рандомных функций
echo ""
echo "🎲 Проверка генерации случайных чисел..."
if grep -r "\brand\s*(\|srand\s*(" src/ include/ 2>/dev/null > build/security-reports/weak-random.log; then
    log_warning "Найдено использование слабых генераторов случайных чисел (rand/srand)"
    echo "📋 Рекомендуется использовать std::random_device и std::mt19937"
    echo "📋 Детали в build/security-reports/weak-random.log"
else
    log_success "Слабые генераторы случайных чисел не обнаружены"
fi

# 9. Проверка на race conditions (базовая проверка)
echo ""
echo "🏃 Проверка на потенциальные race conditions..."
if grep -r "static\s.*=" src/ include/ 2>/dev/null | grep -v "const\|constexpr" > build/security-reports/static-variables.log; then
    log_warning "Найдены статические переменные. Проверьте thread safety"
    echo "📋 Детали в build/security-reports/static-variables.log"
else
    log_success "Потенциальные race conditions не обнаружены"
fi

# 10. Проверка на использование assert в production коде
echo ""
echo "🐛 Проверка использования assert..."
if grep -r "\bassert\s*(" src/ include/ 2>/dev/null | grep -v "#ifdef DEBUG\|#ifndef NDEBUG" > build/security-reports/assert-usage.log; then
    log_warning "Найдено использование assert. В production коде может быть отключен"
    echo "📋 Детали в build/security-reports/assert-usage.log"
else
    log_success "Небезопасное использование assert не обнаружено"
fi

# 11. Запуск дополнительных инструментов безопасности (если доступны)
echo ""
echo "🔧 Дополнительные инструменты безопасности..."

# Flawfinder
if command -v flawfinder &> /dev/null; then
    log_info "Запуск Flawfinder..."
    flawfinder --html --context src/ include/ > build/security-reports/flawfinder-report.html 2>&1
    log_success "Отчет Flawfinder создан: build/security-reports/flawfinder-report.html"
else
    log_info "Flawfinder не установлен"
fi

# Bandit (для Python скриптов, если есть)
if command -v bandit &> /dev/null && find . -name "*.py" | head -1 > /dev/null; then
    log_info "Запуск Bandit для Python скриптов..."
    bandit -r . -f json -o build/security-reports/bandit-report.json 2>/dev/null || true
    log_success "Отчет Bandit создан: build/security-reports/bandit-report.json"
fi

# Semgrep (если установлен)
if command -v semgrep &> /dev/null; then
    log_info "Запуск Semgrep..."
    semgrep --config=auto --json --output=build/security-reports/semgrep-report.json src/ include/ 2>/dev/null || true
    log_success "Отчет Semgrep создан: build/security-reports/semgrep-report.json"
fi

# 12. Создание сводного отчета
echo ""
echo "📋 Создание сводного отчета безопасности..."
cat > build/security-reports/security-summary.md << EOF
# Отчет о безопасности HyperEngine

**Дата:** $(date)

## Результаты проверки

- **Проблемы безопасности:** $SECURITY_ISSUES
- **Предупреждения:** $WARNINGS

## Выполненные проверки

1. ✅ Небезопасные функции C
2. ✅ Переполнение буфера
3. ✅ Управление памятью
4. ✅ SQL инъекции
5. ✅ Небезопасные приведения типов
6. ✅ Временные файлы
7. ✅ Жестко закодированные секреты
8. ✅ Генерация случайных чисел
9. ✅ Race conditions
10. ✅ Использование assert
11. ✅ Дополнительные инструменты

## Детальные отчеты

- Небезопасные функции: unsafe-functions.log
- Массивы: buffer-arrays.log
- SQL запросы: sql-queries.log
- Приведения типов: unsafe-casts.log
- Временные файлы: temp-files.log
- Секреты: hardcoded-secrets.log
- Случайные числа: weak-random.log
- Статические переменные: static-variables.log
- Assert: assert-usage.log

## Рекомендации

$(if [ $SECURITY_ISSUES -gt 0 ]; then
    echo "🚨 Критические проблемы безопасности требуют немедленного исправления"
elif [ $WARNINGS -gt 0 ]; then
    echo "⚠️ Рассмотрите исправление предупреждений безопасности"
else
    echo "✅ Критические проблемы безопасности не обнаружены"
fi)

## Общие рекомендации по безопасности

1. Используйте умные указатели вместо raw pointers
2. Применяйте RAII для управления ресурсами
3. Избегайте небезопасных функций C
4. Используйте современные генераторы случайных чисел
5. Проверяйте границы массивов
6. Применяйте статический анализ регулярно
7. Используйте sanitizers при тестировании
EOF

# Финальный отчет
echo ""
echo "🔒 ОТЧЕТ О БЕЗОПАСНОСТИ"
echo "======================"
echo "Проблемы безопасности: $SECURITY_ISSUES"
echo "Предупреждения: $WARNINGS"
echo ""

if [ $SECURITY_ISSUES -gt 0 ]; then
    echo "🚨 Обнаружены критические проблемы безопасности!"
    echo "📋 Подробный отчет: build/security-reports/security-summary.md"
    exit 1
elif [ $WARNINGS -gt 0 ]; then
    echo "⚠️ Есть предупреждения безопасности. Рекомендуется их рассмотреть."
    echo "📋 Подробный отчет: build/security-reports/security-summary.md"
    exit 0
else
    echo "✅ Критические проблемы безопасности не обнаружены!"
    echo "📋 Подробный отчет: build/security-reports/security-summary.md"
    exit 0
fi
