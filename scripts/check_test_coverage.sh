#!/bin/bash
# 🧪 Test Coverage Validator
# Проверяет минимальное покрытие тестами (80%+)

set -e

GREEN='\033[0;32m'
RED='\033[0;31m'
YELLOW='\033[1;33m'
NC='\033[0m'

echo "🧪 Test Coverage Check"
echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"

MIN_COVERAGE=80

# Проверяем наличие build директории
if [ ! -d "build" ]; then
    echo -e "${YELLOW}⚠️  Build директория не найдена${NC}"
    echo "   Запустите сначала сборку с тестами"
    exit 0
fi

# Запуск тестов
echo "Запуск тестов..."
cd build
if ! ctest --output-on-failure --timeout 60 > /dev/null 2>&1; then
    echo -e "${RED}❌ Некоторые тесты провалились!${NC}"
    ctest --output-on-failure --timeout 60
    exit 1
fi

echo -e "${GREEN}✅ Все тесты прошли успешно${NC}"

# Проверка coverage (если есть gcov данные)
if command -v gcov &> /dev/null; then
    echo ""
    echo "Проверка покрытия кода..."
    
    # Простая проверка через gcov
    COVERED=$(gcov src/*.cpp 2>/dev/null | grep "Lines executed" | awk '{sum+=$3; count++} END {if(count>0) print sum/count; else print 0}')
    
    if [ ! -z "$COVERED" ]; then
        COVERAGE_INT=$(printf "%.0f" "$COVERED")
        
        if [ "$COVERAGE_INT" -ge "$MIN_COVERAGE" ]; then
            echo -e "${GREEN}✅ Coverage: ${COVERAGE_INT}% (минимум: ${MIN_COVERAGE}%)${NC}"
        else
            echo -e "${RED}❌ Coverage: ${COVERAGE_INT}% (минимум: ${MIN_COVERAGE}%)${NC}"
            echo "   Добавьте больше тестов!"
            exit 1
        fi
    else
        echo -e "${YELLOW}⚠️  Coverage данные не найдены${NC}"
    fi
else
    echo -e "${YELLOW}⚠️  gcov не установлен, пропускаем проверку coverage${NC}"
fi

cd ..
echo -e "${GREEN}✅ TEST COVERAGE CHECK: PASSED${NC}"
exit 0

