#!/bin/bash
# ⚡ Performance Baseline & Regression Test
# Создает baseline производительности и проверяет regression

set -e

GREEN='\033[0;32m'
RED='\033[0;31m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m'

BASELINE_FILE="build/benchmark_baseline.txt"
RESULTS_FILE="build/benchmark_results/latest.txt"
REGRESSION_THRESHOLD=5  # 5% допустимая деградация

echo -e "${BLUE}⚡ PERFORMANCE BENCHMARK${NC}"
echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"

# Проверяем наличие benchmark executable
if [ ! -f "build/TriangleSplattingBenchmark" ]; then
    echo -e "${YELLOW}⚠️  Benchmark executable не найден${NC}"
    echo "   Соберите проект с бенчмарками: cmake -DENABLE_BENCHMARKS=ON"
    exit 0
fi

# Создаем директорию для результатов
mkdir -p build/benchmark_results

# Запускаем benchmark
echo "Запуск бенчмарков..."
./build/TriangleSplattingBenchmark --benchmark_out="$RESULTS_FILE" --benchmark_out_format=console

if [ ! -f "$RESULTS_FILE" ]; then
    echo -e "${RED}❌ Не удалось создать файл результатов${NC}"
    exit 1
fi

echo -e "${GREEN}✅ Бенчмарки выполнены${NC}"
echo ""

# Показываем результаты
echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
echo "📊 РЕЗУЛЬТАТЫ БЕНЧМАРКОВ:"
echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
cat "$RESULTS_FILE"
echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"

# Сохранение baseline если его нет
if [ ! -f "$BASELINE_FILE" ]; then
    echo ""
    echo -e "${BLUE}📌 Создание baseline для будущих сравнений...${NC}"
    cp "$RESULTS_FILE" "$BASELINE_FILE"
    echo -e "${GREEN}✅ Baseline сохранен в $BASELINE_FILE${NC}"
    exit 0
fi

# Сравнение с baseline
echo ""
echo "🔍 Сравнение с baseline..."

# Простое сравнение времени выполнения
BASELINE_TIME=$(grep -oP 'Time: \K[0-9.]+' "$BASELINE_FILE" | head -1)
CURRENT_TIME=$(grep -oP 'Time: \K[0-9.]+' "$RESULTS_FILE" | head -1)

if [ -z "$BASELINE_TIME" ] || [ -z "$CURRENT_TIME" ]; then
    echo -e "${YELLOW}⚠️  Не удалось извлечь время из результатов${NC}"
    exit 0
fi

# Вычисляем изменение в процентах
CHANGE=$(awk "BEGIN {print (($CURRENT_TIME - $BASELINE_TIME) / $BASELINE_TIME) * 100}")

echo "   Baseline:     ${BASELINE_TIME}ms"
echo "   Current:      ${CURRENT_TIME}ms"
echo "   Change:       ${CHANGE}%"

# Проверяем regression
if (( $(echo "$CHANGE > $REGRESSION_THRESHOLD" | bc -l) )); then
    echo -e "${RED}❌ PERFORMANCE REGRESSION DETECTED!${NC}"
    echo "   Производительность снизилась на ${CHANGE}%"
    echo "   Допустимый порог: ${REGRESSION_THRESHOLD}%"
    echo ""
    echo "Пожалуйста, оптимизируйте код или обновите baseline"
    exit 1
elif (( $(echo "$CHANGE < -5" | bc -l) )); then
    echo -e "${GREEN}🎉 PERFORMANCE IMPROVEMENT!${NC}"
    echo "   Производительность улучшилась на ${CHANGE}%"
else
    echo -e "${GREEN}✅ Performance в пределах допустимого (${CHANGE}%)${NC}"
fi

exit 0

