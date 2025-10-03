#!/bin/bash
#
# Скрипт запуска FreqVox Sponza Demo
# Автоматически проверяет наличие сборки и запускает демо из правильной директории
#

set -e

SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
cd "$SCRIPT_DIR"

# Цвета для вывода
GREEN='\033[0;32m'
RED='\033[0;31m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

echo -e "${GREEN}========================================${NC}"
echo -e "${GREEN}  FreqVox Sponza Demo Launcher${NC}"
echo -e "${GREEN}========================================${NC}"
echo ""

# Проверка наличия сцены Sponza
if [ ! -f "examples/scenes/sponza/sponza.obj" ]; then
    echo -e "${RED}❌ Ошибка: Сцена Sponza не найдена!${NC}"
    echo -e "${YELLOW}Ожидается: examples/scenes/sponza/sponza.obj${NC}"
    echo ""
    echo "Пожалуйста, убедитесь что сцена Sponza находится в правильной директории."
    exit 1
fi

echo -e "${GREEN}✓ Сцена Sponza найдена${NC}"

# Проверка наличия сборки
if [ ! -f "build/FreqVox_Sponza_Demo" ]; then
    echo -e "${RED}❌ Демо не собрано!${NC}"
    echo ""
    echo "Запустите сборку:"
    echo "  mkdir -p build && cd build"
    echo "  cmake .. -DENABLE_FREQVOX=ON -DBUILD_EXAMPLES=ON"
    echo "  cmake --build . --target FreqVox_Sponza_Demo"
    exit 1
fi

echo -e "${GREEN}✓ Демо собрано${NC}"
echo ""

# Вывод информации о системе
echo -e "${YELLOW}Информация о системе:${NC}"
echo "  Рабочая директория: $(pwd)"
echo "  Исполняемый файл: build/FreqVox_Sponza_Demo"

# Проверка наличия GLFW
if ldconfig -p | grep -q libglfw; then
    echo -e "${GREEN}  ✓ GLFW установлен${NC}"
else
    echo -e "${YELLOW}  ⚠ GLFW может быть не установлен${NC}"
fi

# Проверка наличия Vulkan
if ldconfig -p | grep -q libvulkan; then
    echo -e "${GREEN}  ✓ Vulkan установлен${NC}"
else
    echo -e "${YELLOW}  ⚠ Vulkan может быть не установлен${NC}"
fi

# Проверка CUDA (опционально)
if which nvcc > /dev/null 2>&1; then
    CUDA_VERSION=$(nvcc --version | grep "release" | awk '{print $5}' | cut -d',' -f1)
    echo -e "${GREEN}  ✓ CUDA ${CUDA_VERSION} установлен${NC}"
else
    echo -e "${YELLOW}  ⚠ CUDA не найдена (опционально)${NC}"
fi

echo ""
echo -e "${GREEN}Запуск демо...${NC}"
echo ""
echo -e "${YELLOW}Управление:${NC}"
echo "  WASD - движение камеры"
echo "  Мышь - поворот камеры"
echo "  Scroll - изменение скорости"
echo "  F - фовеация вкл/выкл"
echo "  T - temporal reprojection вкл/выкл"
echo "  U - upscaling вкл/выкл"
echo "  ESC - выход"
echo ""
echo -e "${GREEN}========================================${NC}"
echo ""

# Запуск демо
exec ./build/FreqVox_Sponza_Demo "$@"

