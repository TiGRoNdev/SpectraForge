#!/bin/bash
# 🔍 SOLID Compliance Audit Script
# Проверяет соответствие кодовой базы принципам SOLID

set -e

echo "🔍 SOLID COMPLIANCE AUDIT"
echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"

VIOLATIONS=0

# Цвета для вывода
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

# 1. SRP: Проверка размера файлов (Single Responsibility)
echo ""
echo "📏 1. SRP: Проверка размера файлов (лимит 500 строк)"
echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"

LARGE_FILES=$(find src/ include/ -type f \( -name "*.cpp" -o -name "*.h" -o -name "*.hpp" \) -exec wc -l {} \; | awk '$1 > 500 {print}' | sort -nr)

if [ -z "$LARGE_FILES" ]; then
    echo -e "${GREEN}✅ SRP: Нет файлов больше 500 строк${NC}"
else
    echo -e "${RED}❌ SRP VIOLATION: Найдены файлы больше 500 строк:${NC}"
    echo "$LARGE_FILES"
    VIOLATIONS=$((VIOLATIONS + 1))
fi

# 2. DIP: Проверка использования raw pointers
echo ""
echo "🔌 2. DIP: Проверка raw pointers (должны использоваться smart pointers)"
echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"

RAW_NEW=$(grep -rn "new " src/ --include="*.cpp" | grep -v "//" | wc -l)
RAW_DELETE=$(grep -rn "delete " src/ --include="*.cpp" | grep -v "//" | wc -l)

if [ "$RAW_NEW" -eq 0 ] && [ "$RAW_DELETE" -eq 0 ]; then
    echo -e "${GREEN}✅ DIP: Нет использования raw pointers (new/delete)${NC}"
else
    echo -e "${YELLOW}⚠️  DIP WARNING: Найдено $RAW_NEW 'new' и $RAW_DELETE 'delete'${NC}"
    echo "    Используйте std::make_unique<> / std::make_shared<>"
    VIOLATIONS=$((VIOLATIONS + 1))
fi

# 3. Проверка использования SAFE_TO_STRING
echo ""
echo "🛡️  3. SAFE_TO_STRING: Проверка безопасности консольного вывода"
echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"

UNSAFE_COUT=$(grep -rn "std::cout" src/ --include="*.cpp" | grep -v "SAFE_TO_STRING" | grep -v "<<.*\"" | wc -l)

if [ "$UNSAFE_COUT" -eq 0 ]; then
    echo -e "${GREEN}✅ Все std::cout используют SAFE_TO_STRING${NC}"
else
    echo -e "${RED}❌ VIOLATION: Найдено $UNSAFE_COUT небезопасных std::cout${NC}"
    echo "    Используйте SAFE_TO_STRING() для переменных"
    VIOLATIONS=$((VIOLATIONS + 1))
fi

# 4. Проверка naming conventions
echo ""
echo "📝 4. Naming Conventions: Проверка соглашений об именовании"
echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"

# Проверка snake_case для функций (простая эвристика)
BAD_NAMING=$(grep -rn "^[A-Z].*(" src/ --include="*.cpp" | grep -v "class\|struct\|template" | wc -l)

if [ "$BAD_NAMING" -eq 0 ]; then
    echo -e "${GREEN}✅ Naming: Соблюдены соглашения (snake_case для функций)${NC}"
else
    echo -e "${YELLOW}⚠️  Naming: Возможно нарушение snake_case ($BAD_NAMING случаев)${NC}"
fi

# 5. Проверка использования 'using namespace std' в заголовках
echo ""
echo "🚫 5. Namespace Pollution: Проверка 'using namespace std' в headers"
echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"

USING_NS_STD=$(grep -rn "using namespace std" include/ --include="*.h" --include="*.hpp" | wc -l)

if [ "$USING_NS_STD" -eq 0 ]; then
    echo -e "${GREEN}✅ Нет 'using namespace std' в заголовках${NC}"
else
    echo -e "${RED}❌ CRITICAL: Найдено $USING_NS_STD 'using namespace std' в headers${NC}"
    VIOLATIONS=$((VIOLATIONS + 1))
fi

# 6. Проверка include guards
echo ""
echo "🛡️  6. Include Guards: Проверка #pragma once"
echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"

HEADERS_WITHOUT_PRAGMA=$(find include/ -name "*.h" -o -name "*.hpp" | while read file; do
    if ! grep -q "#pragma once" "$file"; then
        echo "$file"
    fi
done)

if [ -z "$HEADERS_WITHOUT_PRAGMA" ]; then
    echo -e "${GREEN}✅ Все заголовки имеют #pragma once${NC}"
else
    echo -e "${YELLOW}⚠️  Заголовки без #pragma once:${NC}"
    echo "$HEADERS_WITHOUT_PRAGMA"
fi

# ИТОГОВАЯ СТАТИСТИКА
echo ""
echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
echo "📊 ИТОГОВАЯ СТАТИСТИКА"
echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"

if [ "$VIOLATIONS" -eq 0 ]; then
    echo -e "${GREEN}✅ SOLID COMPLIANCE: PASSED${NC}"
    echo "   Все проверки пройдены успешно!"
    exit 0
else
    echo -e "${RED}❌ SOLID COMPLIANCE: FAILED${NC}"
    echo "   Найдено нарушений: $VIOLATIONS"
    echo ""
    echo "Пожалуйста, исправьте нарушения перед commit/merge"
    exit 1
fi

