#!/bin/bash
# 🔍 SDF Debug Test Script with Vulkan Debug Printf
# Запуск демо с включенным Vulkan validation и debug printf

set -e

echo "═══════════════════════════════════════════════════════════"
echo "🔍 SDF ALGORITHM DEBUG TEST"
echo "═══════════════════════════════════════════════════════════"
echo ""

# Убедимся, что билд актуален
if [ ! -f "build/SpectraForge_Example_Demo" ]; then
    echo "❌ Демо не найдено! Запустите сборку сначала."
    exit 1
fi

echo "✅ Демо найдено: build/SpectraForge_Example_Demo"
echo ""

# КРИТИЧНО: Включаем Vulkan validation layer для debug printf
export VK_INSTANCE_LAYERS=VK_LAYER_KHRONOS_validation
export VK_LOADER_DEBUG=all

# Включаем debug printf в Vulkan validation
# Это позволит debugPrintfEXT в шейдере выводить в stderr
export VK_LAYER_ENABLES=VK_VALIDATION_FEATURE_ENABLE_DEBUG_PRINTF_EXT

echo "🔧 Vulkan Debug Printf ENABLED"
echo "   VK_INSTANCE_LAYERS=$VK_INSTANCE_LAYERS"
echo "   VK_LAYER_ENABLES=$VK_LAYER_ENABLES"
echo ""

echo "═══════════════════════════════════════════════════════════"
echo "🚀 ЗАПУСК ДЕМО (первый кадр)"
echo "═══════════════════════════════════════════════════════════"
echo ""

# Запускаем демо и сохраняем debug output
# Ограничиваем время выполнения 10 секундами (достаточно для первого кадра)
cd build

timeout 10s ./SpectraForge_Example_Demo 2>&1 | tee ../sdf_debug_output.txt || true

cd ..

echo ""
echo "═══════════════════════════════════════════════════════════"
echo "📊 Debug output сохранен в: sdf_debug_output.txt"
echo "═══════════════════════════════════════════════════════════"
echo ""

# Анализ ключевых строк debug output
echo "🔍 Анализ SDF debug output:"
echo ""

if grep -q "TRIANGLE #0 DEBUG" sdf_debug_output.txt; then
    echo "✅ Debug logging работает!"
    echo ""
    echo "Ключевые значения из шейдера:"
    grep -A 20 "TRIANGLE #0 DEBUG" sdf_debug_output.txt | head -25
else
    echo "❌ Debug logging НЕ найден в output!"
    echo "Возможные причины:"
    echo "  1. Vulkan validation layer не активирован"
    echo "  2. debugPrintfEXT не поддерживается GPU"
    echo "  3. Первый треугольник за пределами frustum"
fi

echo ""
echo "═══════════════════════════════════════════════════════════"
echo "✅ SDF DEBUG TEST ЗАВЕРШЕН"
echo "═══════════════════════════════════════════════════════════"

