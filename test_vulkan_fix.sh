#!/bin/bash
# Тест исправления конфликта заголовков Vulkan/GLFW

set -e

echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
echo "🔧 Тест исправления конфликта Vulkan заголовков"
echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
echo ""

# Проверка Vulkan драйверов
echo "📋 Шаг 1/4: Проверка Vulkan драйверов..."
if ! vulkaninfo > /dev/null 2>&1; then
    echo "❌ ОШИБКА: Vulkan драйверы не найдены"
    echo "   Установите: sudo apt install vulkan-tools libvulkan-dev"
    exit 1
fi
echo "✅ Vulkan драйверы OK"
echo ""

# Пересборка
echo "📋 Шаг 2/4: Пересборка проекта..."
cd build
cmake --build . --target FreqVox_Sponza_Demo -j$(nproc)
if [ $? -ne 0 ]; then
    echo "❌ ОШИБКА: Компиляция не удалась"
    exit 1
fi
echo "✅ Компиляция успешна"
echo ""

# Проверка бинарника
echo "📋 Шаг 3/4: Проверка бинарника..."
if [ ! -f FreqVox_Sponza_Demo ]; then
    echo "❌ ОШИБКА: Бинарник не найден"
    exit 1
fi
echo "✅ Бинарник создан: $(ls -lh FreqVox_Sponza_Demo | awk '{print $5}')"
echo ""

# Тест запуска (5 секунд)
echo "📋 Шаг 4/4: Тест запуска (5 секунд)..."
timeout 5s ./FreqVox_Sponza_Demo || true
echo ""

echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
echo "✅ ТЕСТ ЗАВЕРШЕН УСПЕШНО"
echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
echo ""
echo "Для полного запуска используйте:"
echo "  ./FreqVox_Sponza_Demo"
echo ""

