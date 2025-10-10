#!/bin/bash

# ==============================================================================
# SpectraForge Optimized Build Script
# 60+ FPS на мобильных GPU, 4K HDR рендеринг
# ==============================================================================

set -e  # Exit on error

echo "🚀 SpectraForge Optimized Build"
echo "================================"
echo ""

# Цвета для вывода
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

# Проверка зависимостей
echo -e "${BLUE}📋 Проверка зависимостей...${NC}"

# Vulkan SDK
USE_SYSTEM_VULKAN=0
if [ -z "$VULKAN_SDK" ]; then
    # Проверяем известный путь
    if [ -d "/home/tigron/vulkan/1.4.321.1" ]; then
        export VULKAN_SDK=/home/tigron/vulkan/1.4.321.1/x86_64
        export PATH=$VULKAN_SDK/bin:$PATH
        export LD_LIBRARY_PATH=$VULKAN_SDK/lib:$LD_LIBRARY_PATH
        export VK_ICD_FILENAMES=$VULKAN_SDK/etc/vulkan/icd.d
        export VK_LAYER_PATH=$VULKAN_SDK/etc/vulkan/explicit_layer.d
        echo -e "${GREEN}✅ Vulkan SDK найден: $VULKAN_SDK${NC}"
    elif command -v pkg-config &> /dev/null && pkg-config --exists vulkan; then
        USE_SYSTEM_VULKAN=1
        VULKAN_INCLUDE_DIR=$(pkg-config --cflags-only-I vulkan | sed 's/-I//g' | tr -d '\n')
        VULKAN_LIBRARY_DIR=$(pkg-config --libs-only-L vulkan | sed 's/-L//g' | tr -d '\n')
        echo -e "${GREEN}✅ Найдена системная установка Vulkan через pkg-config${NC}"
        echo "   include: ${VULKAN_INCLUDE_DIR:-unknown}"
        echo "   lib: ${VULKAN_LIBRARY_DIR:-unknown}"
    else
        echo -e "${RED}❌ Vulkan SDK не найден!${NC}"
        echo "Пожалуйста, установите Vulkan SDK или пакет libvulkan-dev"
        exit 1
    fi
else
    echo -e "${GREEN}✅ Vulkan SDK: $VULKAN_SDK${NC}"
fi

if [ "$USE_SYSTEM_VULKAN" -eq 1 ] && [ -n "$VULKAN_LIBRARY_DIR" ]; then
    export LD_LIBRARY_PATH=$VULKAN_LIBRARY_DIR:$LD_LIBRARY_PATH
fi

# glslc для компиляции шейдеров
if ! command -v glslc &> /dev/null; then
    echo -e "${RED}❌ glslc не найден!${NC}"
    echo "Установите: sudo apt install glslc"
    exit 1
else
    echo -e "${GREEN}✅ glslc найден${NC}"
fi

# CMake
if ! command -v cmake &> /dev/null; then
    echo -e "${RED}❌ CMake не найден!${NC}"
    exit 1
else
    CMAKE_VERSION=$(cmake --version | head -n1 | cut -d' ' -f3)
    echo -e "${GREEN}✅ CMake: $CMAKE_VERSION${NC}"
fi

echo ""

# Выбор конфигурации сборки
BUILD_TYPE="Release"
if [ "$1" == "debug" ]; then
    BUILD_TYPE="Debug"
    echo -e "${YELLOW}⚠️  Debug сборка (медленнее)${NC}"
else
    echo -e "${GREEN}🚀 Release сборка (оптимизировано)${NC}"
fi

# Создание директории сборки
BUILD_DIR="build"
mkdir -p $BUILD_DIR
cd $BUILD_DIR

# Конфигурация CMake
echo ""
echo -e "${BLUE}⚙️  Конфигурация CMake...${NC}"
cmake .. \
    -DCMAKE_BUILD_TYPE=$BUILD_TYPE \
    -DBUILD_EXAMPLES=ON \
    -DCMAKE_CXX_FLAGS="-O3 -march=native -mtune=native -flto -ffast-math" \
    -DCMAKE_EXE_LINKER_FLAGS="-flto"

# Компиляция
echo ""
echo -e "${BLUE}🔨 Компиляция...${NC}"
CORES=$(nproc)
echo "Используется $CORES ядер"
make -j$CORES

# Компиляция шейдеров
echo ""
echo -e "${BLUE}🎨 Компиляция шейдеров...${NC}"
cd ..
./compile_shaders.sh

# Проверка успешности сборки
cd $BUILD_DIR
if [ ! -f "SpectraForge_Optimized_Demo" ]; then
    echo -e "${RED}❌ Ошибка: исполняемый файл не создан${NC}"
    exit 1
fi

echo ""
echo -e "${GREEN}✅ Сборка завершена успешно!${NC}"
echo ""

# Запуск демо
echo "================================"
echo -e "${BLUE}🎮 Запуск оптимизированной демки${NC}"
echo "================================"
echo ""
echo "Доступные режимы:"
echo "  1) Mobile 60+ FPS (1080p -> 4K upscale)"
echo "  2) Balanced (1440p -> 4K upscale)"
echo "  3) Quality (Native 4K)"
echo ""

# Выбор режима
read -p "Выберите режим (1-3) [1]: " mode
mode=${mode:-1}

case $mode in
    1)
        MODE_FLAG="--mobile"
        echo -e "${GREEN}Запуск в режиме Mobile 60+ FPS${NC}"
        ;;
    2)
        MODE_FLAG="--balanced"
        echo -e "${YELLOW}Запуск в режиме Balanced${NC}"
        ;;
    3)
        MODE_FLAG="--quality"
        echo -e "${RED}Запуск в режиме Quality (требовательно к GPU)${NC}"
        ;;
    *)
        MODE_FLAG="--mobile"
        echo -e "${GREEN}По умолчанию: Mobile 60+ FPS${NC}"
        ;;
esac

echo ""
echo "Управление в демо:"
echo "  WASD - движение"
echo "  Мышь - обзор"
echo "  Q/E - вверх/вниз"
echo "  1-3 - переключение качества"
echo "  H - HDR вкл/выкл"
echo "  ESC - выход"
echo ""
echo -e "${YELLOW}Запуск...${NC}"
echo ""

# Установка переменных окружения для оптимальной производительности
export VK_ICD_FILENAMES="/usr/share/vulkan/icd.d/nvidia_icd.json:/usr/share/vulkan/icd.d/radeon_icd.x86_64.json:/usr/share/vulkan/icd.d/intel_icd.x86_64.json"
export MESA_VK_WSI_PRESENT_MODE=immediate  # Disable VSync для максимального FPS
export __GL_SYNC_TO_VBLANK=0

# Запуск с приоритетом реального времени для стабильного FPS
if command -v nice &> /dev/null; then
    nice -n -10 ./SpectraForge_Optimized_Demo $MODE_FLAG
else
    ./SpectraForge_Optimized_Demo $MODE_FLAG
fi

# Результаты
echo ""
echo "================================"
echo -e "${GREEN}✅ Демо завершено${NC}"
echo "================================"
