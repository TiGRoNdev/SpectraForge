#!/bin/bash
# 🛡️ SAFE_TO_STRING Checker
# Проверяет использование SAFE_TO_STRING для всех console outputs

set -e

RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m'

echo "🛡️  SAFE_TO_STRING Compliance Check"
echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"

VIOLATIONS=0

# Проверка std::cout без SAFE_TO_STRING
echo "Проверка std::cout..."
UNSAFE_COUT=$(find src/ include/ -type f \( -name "*.cpp" -o -name "*.h" \) -exec grep -Hn "std::cout.*<<" {} \; | \
    grep -v "SAFE_TO_STRING\|SAFE_INT\|SAFE_FLOAT\|SAFE_DOUBLE" | \
    grep -v "<<.*\".*<<" | \
    grep -v "// NOLINT")

if [ ! -z "$UNSAFE_COUT" ]; then
    echo -e "${RED}❌ Найдены небезопасные std::cout:${NC}"
    echo "$UNSAFE_COUT"
    VIOLATIONS=$((VIOLATIONS + 1))
fi

# Проверка std::cerr
echo "Проверка std::cerr..."
UNSAFE_CERR=$(find src/ include/ -type f \( -name "*.cpp" -o -name "*.h" \) -exec grep -Hn "std::cerr.*<<" {} \; | \
    grep -v "SAFE_TO_STRING\|SAFE_INT\|SAFE_FLOAT\|SAFE_DOUBLE" | \
    grep -v "<<.*\".*<<" | \
    grep -v "// NOLINT")

if [ ! -z "$UNSAFE_CERR" ]; then
    echo -e "${RED}❌ Найдены небезопасные std::cerr:${NC}"
    echo "$UNSAFE_CERR"
    VIOLATIONS=$((VIOLATIONS + 1))
fi

# Проверка printf (не должно использоваться вообще)
echo "Проверка printf..."
PRINTF_USAGE=$(find src/ include/ -type f \( -name "*.cpp" -o -name "*.h" \) -exec grep -Hn "printf\|sprintf" {} \; | \
    grep -v "// NOLINT")

if [ ! -z "$PRINTF_USAGE" ]; then
    echo -e "${YELLOW}⚠️  Найдено использование printf/sprintf (используйте std::cout + SAFE_TO_STRING):${NC}"
    echo "$PRINTF_USAGE"
fi

# Итог
if [ "$VIOLATIONS" -eq 0 ]; then
    echo -e "${GREEN}✅ SAFE_TO_STRING: PASSED${NC}"
    exit 0
else
    echo -e "${RED}❌ SAFE_TO_STRING: FAILED (нарушений: $VIOLATIONS)${NC}"
    echo ""
    echo "Исправьте, обернув переменные в SAFE_TO_STRING():"
    echo "  ❌ std::cout << value << std::endl;"
    echo "  ✅ std::cout << SAFE_TO_STRING(value) << std::endl;"
    exit 1
fi

