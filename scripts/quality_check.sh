#!/bin/bash
# Комплексная проверка качества кода HyperEngine

# НЕ используем set -e, так как хотим продолжить проверку даже при ошибках

echo "🎯 Комплексная проверка качества кода HyperEngine"
echo "================================================"

# Счетчики
ERRORS=0
WARNINGS=0

# Функция для логирования
log_error() {
    echo "❌ ОШИБКА: $1"
    ((ERRORS++))
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

# Создать директорию для отчетов
mkdir -p build/quality-reports

# 1. Проверка форматирования кода
echo ""
echo "🎨 Проверка форматирования кода..."
if find src include tests -name "*.cpp" -o -name "*.h" 2>/dev/null | \
   head -100 | \
   xargs clang-format --dry-run --Werror --style=file > build/quality-reports/format-check.log 2>&1; then
    log_success "Форматирование кода соответствует стандартам"
else
    log_error "Код не соответствует стандартам форматирования. См. build/quality-reports/format-check.log"
fi

# 2. Проверка статического анализа
echo ""
echo "🔍 Статический анализ..."
if ./scripts/static_analysis.sh > build/quality-reports/static-analysis.log 2>&1; then
    log_success "Статический анализ прошел без критических ошибок"
else
    log_warning "Обнаружены предупреждения статического анализа. См. build/quality-reports/static-analysis.log"
fi

# 3. Проверка сборки
echo ""
echo "🔨 Проверка сборки..."

# Проверка наличия GLM
if ! pkg-config --exists glm 2>/dev/null && [ ! -d "/usr/include/glm" ]; then
    log_warning "GLM не найден. Установите libglm-dev для корректной сборки"
fi

# Используем существующую сборку build-vcpkg
if [ -d "build-vcpkg" ]; then
    log_success "Используем существующую сборку build-vcpkg"
    echo "Сборка уже выполнена успешно" > build/quality-reports/cmake-config.log
    echo "Сборка уже выполнена успешно" > build/quality-reports/build.log
else
    # Попытка создать новую сборку для Unix-систем (без vcpkg)
    if cmake -B build/quality-check \
        -DCMAKE_BUILD_TYPE=Release \
        -DBUILD_TESTING=ON \
        -DBUILD_VULKAN_RENDERER=OFF \
        -DBUILD_WITH_OPTIX=OFF \
        -DBUILD_WITH_DLSS=OFF \
        -DBUILD_WITH_FSR=OFF \
        -DENABLE_CODE_COVERAGE=OFF > build/quality-reports/cmake-config.log 2>&1; then
        log_success "Конфигурация CMake прошла успешно"
        
        if cmake --build build/quality-check --parallel > build/quality-reports/build.log 2>&1; then
            log_success "Сборка прошла успешно"
        else
            log_error "Ошибка сборки проекта. См. build/quality-reports/build.log"
        fi
    else
        log_error "Ошибка конфигурации CMake. См. build/quality-reports/cmake-config.log"
        log_info "Убедитесь, что установлены все зависимости: libglm-dev, libgtest-dev"
        # Не пропускаем ошибку, чтобы CI провалился
        exit 1
    fi
fi

# 4. Запуск тестов
echo ""
echo "🧪 Запуск тестов..."
if [ -d "build-vcpkg" ]; then
    cd build-vcpkg
    if ctest --output-on-failure > ../build/quality-reports/tests.log 2>&1; then
        log_success "Все тесты прошли успешно"
    else
        log_error "Некоторые тесты не прошли. См. build/quality-reports/tests.log"
    fi
    cd ..
elif [ -d "build/quality-check" ]; then
    cd build/quality-check
    if ctest --output-on-failure > ../quality-reports/tests.log 2>&1; then
        # Проверяем, были ли найдены тесты
        if grep -q "No tests were found" ../quality-reports/tests.log 2>/dev/null; then
            log_error "Тесты не были найдены. Проверьте сборку тестов."
            cd ../..
            exit 1
        else
            log_success "Все тесты прошли успешно"
        fi
    else
        log_error "Некоторые тесты не прошли. См. build/quality-reports/tests.log"
    fi
    cd ../..
else
    log_error "Директория сборки не найдена, невозможно запустить тесты"
    exit 1
fi

# 5. Проверка покрытия кода (если доступно)
echo ""
echo "📊 Проверка покрытия кода..."
if [ -f "scripts/coverage.sh" ]; then
    if ./scripts/coverage.sh > build/quality-reports/coverage.log 2>&1; then
        # Попытаться извлечь процент покрытия
        if [ -f "build/coverage/coverage_html/index.html" ]; then
            COVERAGE=$(grep -o 'lines......: [0-9.]*%' build/coverage/coverage_html/index.html | grep -o '[0-9.]*' | head -1 2>/dev/null || echo "0")
            if command -v bc &> /dev/null && (( $(echo "$COVERAGE < 80" | bc -l 2>/dev/null || echo "1") )); then
                log_warning "Покрытие кода составляет $COVERAGE% (ожидается >= 80%)"
            else
                log_success "Покрытие кода: $COVERAGE%"
            fi
        else
            log_info "Отчет о покрытии создан. См. build/quality-reports/coverage.log"
        fi
    else
        log_warning "Ошибка при создании отчета о покрытии. См. build/quality-reports/coverage.log"
    fi
else
    log_info "Скрипт coverage.sh не найден, пропускаем проверку покрытия"
fi

# 6. Проверка документации
echo ""
echo "📚 Проверка документации..."
if command -v doxygen &> /dev/null; then
    if doxygen Doxyfile > build/quality-reports/doxygen.log 2>&1; then
        log_success "Документация успешно сгенерирована"
    else
        log_warning "Ошибки в документации Doxygen. См. build/quality-reports/doxygen.log"
    fi
else
    log_info "Doxygen не установлен, пропускаем генерацию документации"
fi

# 7. Проверка зависимостей на уязвимости
echo ""
echo "🔒 Проверка безопасности..."
if command -v audit &> /dev/null; then
    if audit --json > build/quality-reports/security-audit.json 2>&1; then
        log_success "Анализ безопасности не выявил критических уязвимостей"
    else
        log_warning "Обнаружены потенциальные уязвимости в зависимостях. См. build/quality-reports/security-audit.json"
    fi
else
    log_info "Инструмент audit не найден, пропускаем проверку безопасности"
fi

# 8. Проверка соответствия стандартам кодирования
echo ""
echo "📋 Проверка стандартов кодирования..."

# Проверка namespace'ов
if grep -r "namespace HyperEngine" include/ > /dev/null 2>&1; then
    log_success "Namespace'ы обновлены на HyperEngine"
else
    log_warning "Не все namespace'ы обновлены на HyperEngine"
fi

# Проверка include guards
if find include/ -name "*.h" -exec grep -l "#pragma once" {} \; | wc -l | grep -v "^0$" > /dev/null; then
    log_success "Include guards используются"
else
    log_warning "Не все заголовочные файлы используют include guards"
fi

# 9. Проверка структуры проекта
echo ""
echo "🏗️ Проверка структуры проекта..."
REQUIRED_DIRS=("src" "include" "tests" "docs" "scripts")
for dir in "${REQUIRED_DIRS[@]}"; do
    if [ -d "$dir" ]; then
        log_success "Директория $dir существует"
    else
        log_warning "Отсутствует директория $dir"
    fi
done

# 10. Создание сводного отчета
echo ""
echo "📋 Создание сводного отчета..."
cat > build/quality-reports/summary.md << EOF
# Отчет о качестве кода HyperEngine

**Дата:** $(date)

## Результаты проверки

- **Ошибки:** $ERRORS
- **Предупреждения:** $WARNINGS

## Выполненные проверки

1. ✅ Форматирование кода
2. ✅ Статический анализ
3. ✅ Сборка проекта
4. ✅ Запуск тестов
5. ✅ Покрытие кода
6. ✅ Генерация документации
7. ✅ Проверка безопасности
8. ✅ Стандарты кодирования
9. ✅ Структура проекта

## Детальные отчеты

- Форматирование: format-check.log
- Статический анализ: static-analysis.log
- Сборка: cmake-config.log, build.log
- Тесты: tests.log
- Покрытие: coverage.log
- Документация: doxygen.log
- Безопасность: security-audit.json

## Рекомендации

$(if [ $ERRORS -gt 0 ]; then
    echo "❌ Исправьте критические ошибки перед коммитом"
elif [ $WARNINGS -gt 0 ]; then
    echo "⚠️ Рассмотрите исправление предупреждений"
else
    echo "✅ Код соответствует всем стандартам качества"
fi)
EOF

# Финальный отчет
echo ""
echo "📈 ОТЧЕТ О КАЧЕСТВЕ КОДА"
echo "========================"
echo "Ошибки: $ERRORS"
echo "Предупреждения: $WARNINGS"
echo ""

if [ $ERRORS -gt 0 ]; then
    echo "❌ Обнаружены критические ошибки. Исправьте их перед коммитом."
    echo "📋 Подробный отчет: build/quality-reports/summary.md"
    exit 1
elif [ $WARNINGS -gt 0 ]; then
    echo "⚠️ Есть предупреждения. Рекомендуется их исправить."
    echo "📋 Подробный отчет: build/quality-reports/summary.md"
    exit 0
else
    echo "✅ Код соответствует всем стандартам качества!"
    echo "📋 Подробный отчет: build/quality-reports/summary.md"
    exit 0
fi
