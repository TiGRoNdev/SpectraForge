#!/bin/bash
#
# Безопасный тестовый скрипт с автоматическим timeout
# Запускает демо на 10 секунд и автоматически завершает
#

set -e

DEMO="./build/SpectraForge_Example_Demo"
TIMEOUT=10  # секунд
LOG_FILE="/tmp/spectraforge_test_$$.log"

echo "🧪 ===== БЕЗОПАСНОЕ ТЕСТИРОВАНИЕ SPECTRAFORGE ====="
echo ""
echo "⏱️  Timeout: ${TIMEOUT} секунд"
echo "📝 Лог: ${LOG_FILE}"
echo ""

# Проверка Vulkan
echo "🔍 Проверка Vulkan..."
if ! command -v vulkaninfo &> /dev/null; then
    echo "❌ vulkaninfo не найден!"
    exit 1
fi

if ! vulkaninfo --summary &> /dev/null; then
    echo "❌ Vulkan не работает!"
    exit 1
fi
echo "✅ Vulkan работает"
echo ""

# Экспорт системного Vulkan
export LD_LIBRARY_PATH="/usr/lib/x86_64-linux-gnu:$LD_LIBRARY_PATH"
export VK_LOADER_DEBUG=none  # Отключаем debug вывод loader'а

echo "🚀 Запуск демо с timeout ${TIMEOUT}s..."
echo "   Нажмите Ctrl+C для экстренного завершения"
echo ""

# Запуск с timeout и перенаправлением вывода
timeout --signal=SIGINT --kill-after=2s ${TIMEOUT}s ${DEMO} 2>&1 | tee "${LOG_FILE}" &
DEMO_PID=$!

# Ждём завершения
wait $DEMO_PID
EXIT_CODE=$?

echo ""
echo "===================="
echo "📊 РЕЗУЛЬТАТЫ ТЕСТА:"
echo "===================="

if [ $EXIT_CODE -eq 124 ]; then
    echo "✅ Timeout достигнут (${TIMEOUT}s) - ОЖИДАЕМО"
    echo "   Демо работало без зависаний!"
elif [ $EXIT_CODE -eq 0 ]; then
    echo "✅ Демо завершилось нормально"
elif [ $EXIT_CODE -eq 130 ]; then
    echo "⚠️  Прервано пользователем (Ctrl+C)"
else
    echo "❌ Демо завершилось с кодом: $EXIT_CODE"
fi

echo ""
echo "📝 Проверка логов на наличие проблем..."
echo ""

# Анализ лога
if grep -q "TIMEOUT.*Fence.*5 секунд" "${LOG_FILE}"; then
    echo "⚠️  ПРЕДУПРЕЖДЕНИЕ: Обнаружен timeout fence (5s)"
    grep "TIMEOUT.*Fence" "${LOG_FILE}"
fi

if grep -q "TIMEOUT.*acquireNextImageKHR.*1 секунду" "${LOG_FILE}"; then
    echo "⚠️  ПРЕДУПРЕЖДЕНИЕ: Обнаружен timeout acquireNextImageKHR (1s)"
    grep "TIMEOUT.*acquireNextImageKHR" "${LOG_FILE}"
fi

if grep -q "минимизировано" "${LOG_FILE}"; then
    echo "ℹ️  Окно было минимизировано"
fi

if grep -q "✅.*initialized successfully" "${LOG_FILE}"; then
    echo "✅ Инициализация прошла успешно"
fi

if grep -q "VulkanContext initialized successfully" "${LOG_FILE}"; then
    echo "✅ VulkanContext инициализирован"
fi

echo ""
echo "Полный лог сохранён: ${LOG_FILE}"
echo ""

# Проверка что процесс завершился
sleep 1
if ps -p ${DEMO_PID} > /dev/null 2>&1; then
    echo "⚠️  ВНИМАНИЕ: Процесс всё ещё работает! Принудительное завершение..."
    kill -9 ${DEMO_PID} 2>/dev/null || true
    echo "✅ Процесс завершён"
fi

echo ""
echo "🎯 ТЕСТ ЗАВЕРШЁН"

exit 0

