#!/bin/bash
# 🔍 COMPLETE REFACTORING CHECK
# Запускает ВСЕ проверки для рефакторинга

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"

GREEN='\033[0;32m'
RED='\033[0;31m'
BLUE='\033[0;34m'
NC='\033[0m'

echo -e "${BLUE}"
echo "╔═══════════════════════════════════════════╗"
echo "║  SpectraForge Refactoring Check Suite    ║"
echo "╚═══════════════════════════════════════════╝"
echo -e "${NC}"

FAILED_CHECKS=0

# 1. SOLID Audit
echo ""
echo "═══════════════════════════════════════════"
echo "1️⃣  SOLID COMPLIANCE AUDIT"
echo "═══════════════════════════════════════════"
if bash "$SCRIPT_DIR/solid_audit.sh"; then
    echo -e "${GREEN}✅ SOLID Audit: PASSED${NC}"
else
    echo -e "${RED}❌ SOLID Audit: FAILED${NC}"
    FAILED_CHECKS=$((FAILED_CHECKS + 1))
fi

# 2. SAFE_TO_STRING Check
echo ""
echo "═══════════════════════════════════════════"
echo "2️⃣  SAFE_TO_STRING COMPLIANCE"
echo "═══════════════════════════════════════════"
if bash "$SCRIPT_DIR/check_safe_to_string.sh"; then
    echo -e "${GREEN}✅ SAFE_TO_STRING: PASSED${NC}"
else
    echo -e "${RED}❌ SAFE_TO_STRING: FAILED${NC}"
    FAILED_CHECKS=$((FAILED_CHECKS + 1))
fi

# 3. Test Coverage
echo ""
echo "═══════════════════════════════════════════"
echo "3️⃣  TEST COVERAGE"
echo "═══════════════════════════════════════════"
if bash "$SCRIPT_DIR/check_test_coverage.sh"; then
    echo -e "${GREEN}✅ Test Coverage: PASSED${NC}"
else
    echo -e "${RED}❌ Test Coverage: FAILED${NC}"
    FAILED_CHECKS=$((FAILED_CHECKS + 1))
fi

# 4. Performance Baseline
echo ""
echo "═══════════════════════════════════════════"
echo "4️⃣  PERFORMANCE BASELINE"
echo "═══════════════════════════════════════════"
if bash "$SCRIPT_DIR/performance_baseline.sh"; then
    echo -e "${GREEN}✅ Performance: PASSED${NC}"
else
    echo -e "${RED}❌ Performance: FAILED${NC}"
    FAILED_CHECKS=$((FAILED_CHECKS + 1))
fi

# ИТОГОВЫЙ РЕЗУЛЬТАТ
echo ""
echo "═══════════════════════════════════════════"
echo "📊 ИТОГОВЫЙ РЕЗУЛЬТАТ"
echo "═══════════════════════════════════════════"

if [ "$FAILED_CHECKS" -eq 0 ]; then
    echo -e "${GREEN}"
    echo "╔═══════════════════════════════════════════╗"
    echo "║  ✅ ALL CHECKS PASSED                     ║"
    echo "║  Код готов к commit/merge!               ║"
    echo "╚═══════════════════════════════════════════╝"
    echo -e "${NC}"
    exit 0
else
    echo -e "${RED}"
    echo "╔═══════════════════════════════════════════╗"
    echo "║  ❌ CHECKS FAILED: $FAILED_CHECKS                       ║"
    echo "║  Исправьте проблемы перед commit!        ║"
    echo "╚═══════════════════════════════════════════╝"
    echo -e "${NC}"
    exit 1
fi

