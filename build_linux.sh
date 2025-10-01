#!/bin/bash
# Скрипт сборки HyperEngine для Linux
# Автоматически определяет доступные SDK и настраивает сборку

set -e

echo "🚀 HyperEngine - Скрипт сборки для Linux"
echo "=========================================="
echo ""

# Цвета для вывода
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

# Директория проекта
PROJECT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
cd "$PROJECT_DIR"

# Параметры по умолчанию
BUILD_DIR="${PROJECT_DIR}/build"
BUILD_TYPE="${1:-Release}"
VCPKG_ROOT="${VCPKG_ROOT:-$HOME/vcpkg}"

echo "📁 Директория проекта: $PROJECT_DIR"
echo "🔧 Тип сборки: $BUILD_TYPE"
echo ""

# Проверка vcpkg
if [ ! -f "$VCPKG_ROOT/scripts/buildsystems/vcpkg.cmake" ]; then
    echo -e "${RED}❌ Ошибка: vcpkg не найден в $VCPKG_ROOT${NC}"
    echo "Установите vcpkg или задайте переменную VCPKG_ROOT"
    exit 1
fi

echo -e "${GREEN}✓${NC} vcpkg найден: $VCPKG_ROOT"

# Проверка CUDA
BUILD_WITH_CUDA=OFF
if command -v nvcc &> /dev/null; then
    CUDA_VERSION=$(nvcc --version | grep "release" | awk '{print $5}' | sed 's/,//')
    echo -e "${GREEN}✓${NC} CUDA найден: версия $CUDA_VERSION"
    BUILD_WITH_CUDA=ON
else
    echo -e "${YELLOW}⚠${NC} CUDA не найден, сборка без CUDA"
fi

# Проверка OptiX
BUILD_WITH_OPTIX=OFF
if [ -n "$OptiX_ROOT_DIR" ] && [ -d "$OptiX_ROOT_DIR" ]; then
    echo -e "${GREEN}✓${NC} OptiX найден: $OptiX_ROOT_DIR"
    BUILD_WITH_OPTIX=ON
elif [ -n "$OPTIX_ROOT" ] && [ -d "$OPTIX_ROOT" ]; then
    echo -e "${GREEN}✓${NC} OptiX найден: $OPTIX_ROOT"
    export OptiX_ROOT_DIR="$OPTIX_ROOT"
    BUILD_WITH_OPTIX=ON
else
    echo -e "${YELLOW}⚠${NC} OptiX не найден, сборка без OptiX"
fi

# DLSS на Linux не поддерживается
BUILD_WITH_DLSS=OFF
echo -e "${YELLOW}ℹ${NC} DLSS не поддерживается на Linux (только Windows)"

# Проверка FSR
BUILD_WITH_FSR=OFF
if [ -n "$FSR_ROOT_DIR" ] && [ -d "$FSR_ROOT_DIR" ]; then
    echo -e "${GREEN}✓${NC} FSR найден: $FSR_ROOT_DIR"
    BUILD_WITH_FSR=ON
elif [ -n "$FIDELITYFX_ROOT" ] && [ -d "$FIDELITYFX_ROOT" ]; then
    echo -e "${GREEN}✓${NC} FSR найден: $FIDELITYFX_ROOT"
    export FSR_ROOT_DIR="$FIDELITYFX_ROOT"
    BUILD_WITH_FSR=ON
else
    echo -e "${YELLOW}⚠${NC} FSR не найден, сборка без FSR"
fi

echo ""
echo "📋 Конфигурация сборки:"
echo "  • CUDA:  $BUILD_WITH_CUDA"
echo "  • OptiX: $BUILD_WITH_OPTIX"
echo "  • DLSS:  $BUILD_WITH_DLSS (не поддерживается на Linux)"
echo "  • FSR:   $BUILD_WITH_FSR"
echo ""

# Очистка предыдущей сборки (опционально)
if [ "$2" == "clean" ]; then
    echo "🧹 Очистка предыдущей сборки..."
    rm -rf "$BUILD_DIR"
fi

# Создание директории сборки
mkdir -p "$BUILD_DIR"

# Конфигурация CMake
echo "⚙️  Конфигурация CMake..."
cmake -B "$BUILD_DIR" -G Ninja \
    -DCMAKE_TOOLCHAIN_FILE="$VCPKG_ROOT/scripts/buildsystems/vcpkg.cmake" \
    -DCMAKE_EXPORT_COMPILE_COMMANDS=ON \
    -DCMAKE_BUILD_TYPE="$BUILD_TYPE" \
    -DBUILD_WITH_CUDA="$BUILD_WITH_CUDA" \
    -DBUILD_WITH_OPTIX="$BUILD_WITH_OPTIX" \
    -DBUILD_WITH_DLSS=OFF \
    -DBUILD_WITH_FSR="$BUILD_WITH_FSR"

if [ $? -ne 0 ]; then
    echo -e "${RED}❌ Ошибка конфигурации CMake${NC}"
    exit 1
fi

# Сборка
echo ""
echo "🔨 Сборка проекта..."
cmake --build "$BUILD_DIR" --config "$BUILD_TYPE" -j$(nproc)

if [ $? -ne 0 ]; then
    echo -e "${RED}❌ Ошибка сборки${NC}"
    exit 1
fi

echo ""
echo -e "${GREEN}✅ Сборка успешно завершена!${NC}"
echo ""
echo "📦 Результаты сборки:"
ls -lh "$BUILD_DIR"/*.a "$BUILD_DIR"/*_Demo 2>/dev/null || true

echo ""
echo "🚀 Для запуска демо:"
echo "  cd $BUILD_DIR"
echo "  ./VulkanRenderer_Demo"
echo ""

