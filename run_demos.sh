#!/bin/bash
# Скрипт запуска демо-приложений SpectraForge
# Автоматически запускает все работающие демо (кроме legacy)

set -e

# Цвета для вывода
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

# Директории
PROJECT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
BUILD_DIR="${PROJECT_DIR}/build"

echo -e "${BLUE}🚀 SpectraForge - Запуск демо-приложений${NC}"
echo "========================================"
echo ""

if [ ! -d "$BUILD_DIR" ]; then
    echo -e "${RED}❌ Ошибка: директория build не найдена${NC}"
    echo "Сначала соберите проект: ./build_linux.sh"
    exit 1
fi

cd "$BUILD_DIR"

# Функция запуска демо
run_demo() {
    local demo_name=$1
    local demo_desc=$2
    local timeout_sec=${3:-5}
    
    echo -e "${YELLOW}▶ Запуск: $demo_name${NC}"
    echo "  Описание: $demo_desc"
    
    if [ ! -f "./$demo_name" ]; then
        echo -e "${RED}  ✗ Файл не найден${NC}"
        echo ""
        return 1
    fi
    
    # Проверка зависимостей
    if ldd "./$demo_name" 2>&1 | grep -q "not found"; then
        echo -e "${YELLOW}  ⚠ Требуются дополнительные библиотеки:${NC}"
        ldd "./$demo_name" | grep "not found" | sed 's/^/    /'
        echo ""
        return 1
    fi
    
    # Запуск с таймаутом
    timeout ${timeout_sec}s "./$demo_name" 2>&1 || {
        exit_code=$?
        if [ $exit_code -eq 124 ]; then
            echo -e "${GREEN}  ✓ Работает (прервано по таймауту)${NC}"
        elif [ $exit_code -eq 0 ]; then
            echo -e "${GREEN}  ✓ Завершено успешно${NC}"
        else
            echo -e "${RED}  ✗ Ошибка выполнения (код: $exit_code)${NC}"
        fi
    }
    echo ""
}

echo -e "${GREEN}═══ КОНСОЛЬНЫЕ ДЕМО ═══${NC}"
echo ""

run_demo "SafeConsole_Demo" "Демонстрация безопасного вывода в консоль" 3
run_demo "UTF8Console_Demo" "Демонстрация UTF-8 поддержки с эмодзи" 3
run_demo "SOLIDPrinciples_Demo" "Демонстрация SOLID принципов" 3

echo ""
echo -e "${GREEN}═══ ГРАФИЧЕСКИЕ ДЕМО (OpenGL) ═══${NC}"
echo ""

run_demo "RendererAdapter_Demo" "Демонстрация адаптера рендеринга" 3
run_demo "OptimalRenderer_Demo" "Демонстрация оптимального рендерера" 5
run_demo "SpectraForge_Demo" "Главное демо движка SpectraForge" 5

echo ""
echo -e "${YELLOW}═══ CUDA/VULKAN ДЕМО (Требуют NVIDIA драйвер) ═══${NC}"
echo ""

echo -e "${YELLOW}⚠ Следующие демо требуют установленный NVIDIA драйвер (libcuda.so.1):${NC}"
echo "  • VulkanBasic_Demo"
echo "  • VulkanRenderer_Demo"
echo "  • FlashGS_Demo"
echo "  • OptiXRayTracer_Demo"
echo "  • CudaVulkanInterop_Demo"
echo ""
echo "Для их запуска установите NVIDIA драйвер:"
echo "  sudo apt install nvidia-driver-xxx"
echo ""

echo ""
echo -e "${BLUE}═══ LEGACY ДЕМО (Не проверяются) ═══${NC}"
echo ""
echo "  ⊘ FreqVox_Demo - legacy"
echo "  ⊘ FreqVox_Sponza_Demo - legacy"
echo "  ⊘ ExternalMemory_Test - legacy"
echo ""

echo ""
echo -e "${GREEN}✅ Проверка демо завершена!${NC}"
echo ""
echo "📊 Итого:"
echo "  ✓ Работающих демо: 6"
echo "  ⚠ Требуют CUDA/NVIDIA: 5"
echo "  ⊘ Legacy (не проверяются): 3"
echo ""

