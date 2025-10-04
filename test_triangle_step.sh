#!/bin/bash
# Автоматическое тестирование различных значений triangleStep для поиска оптимума

set -e

echo "=== Triangle Step Performance Test ==="
echo "Testing different triangle densities for optimal FPS/quality balance"
echo ""

# Массив значений step для тестирования
STEPS=(100 200 300 400 500 1000)

for STEP in "${STEPS[@]}"; do
    echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
    echo "Testing triangleStep = $STEP"
    echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
    
    # Изменяем step в demo файле
    sed -i "s/scene.triangleStep = [0-9]*;/scene.triangleStep = $STEP;/" examples/SpectraForge_Example_Demo.cpp
    
    # Пересобираем только demo
    cmake --build build --target SpectraForge_Example_Demo -j$(nproc) > /dev/null 2>&1
    
    # Запускаем тест на 15 секунд
    timeout 15s ./run_with_system_vulkan.sh ./build/SpectraForge_Example_Demo > /tmp/test_step_${STEP}.log 2>&1 || true
    
    # Извлекаем результаты
    TRIANGLES=$(grep "Загружено.*треугольников для Triangle" /tmp/test_step_${STEP}.log | grep -oP '\d+(?= треугольников)' | head -1)
    AVG_FPS=$(grep "📊 FPS:" /tmp/test_step_${STEP}.log | tail -n 10 | grep -oP '\d+' | awk '{sum+=$1; count++} END {if(count>0) print int(sum/count); else print 0}')
    
    echo "  Треугольников: $TRIANGLES"
    echo "  Средний FPS:   $AVG_FPS"
    echo ""
    
    # Сохраняем результат в таблицу
    echo "$STEP,$TRIANGLES,$AVG_FPS" >> /tmp/triangle_step_results.csv
done

echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
echo "RESULTS SUMMARY"
echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
echo ""
echo "Step | Triangles | Avg FPS"
echo "-----|-----------|--------"
cat /tmp/triangle_step_results.csv | column -t -s ','
echo ""
echo "Results saved to: /tmp/triangle_step_results.csv"

