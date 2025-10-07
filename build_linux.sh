#!/bin/bash

# SpectraForge Build Script for Linux
# Pure C++ 3D Engine with Vulkan and Hybrid TriangleSplatting + FreGS rendering

set -e  # Exit on any error

echo "=================================="
echo "SpectraForge Linux Build Script"
echo "Pure C++ with Vulkan only"
echo "Hybrid TriangleSplatting + FreGS"
echo "=================================="

# Проверяем наличие CMake
if ! command -v cmake &> /dev/null; then
    echo "❌ CMake не найден. Установите CMake 3.16 или выше."
    exit 1
fi

# Проверяем наличие компилятора C++
if ! command -v g++ &> /dev/null; then
    echo "❌ Компилятор C++ не найден. Установите g++."
    exit 1
fi

# Проверяем наличие системного Vulkan SDK
if ! pkg-config --exists vulkan; then
    echo "❌ Vulkan SDK не найден в системе."
    echo "Установите Vulkan SDK: sudo apt install libvulkan-dev"
    exit 1
fi

echo "✅ Vulkan SDK найден в системе"

echo "✅ Проверки зависимостей пройдены"

# Создаем директорию сборки если её нет
BUILD_DIR="build"
if [ ! -d "$BUILD_DIR" ]; then
    echo "📁 Создаём директорию сборки: $BUILD_DIR"
    mkdir -p "$BUILD_DIR"
else
    echo "📁 Директория сборки уже существует: $BUILD_DIR"
fi

cd "$BUILD_DIR"

# Настройка проекта с CMake
echo "🔧 Настройка проекта с CMake..."
cmake .. \
    -DCMAKE_BUILD_TYPE=Release \
    -DCMAKE_CXX_STANDARD=17 \
    -DVULKAN_SDK_PATH="$VULKAN_SDK_PATH" \
    -DCMAKE_PREFIX_PATH="$VULKAN_SDK_PATH" \
    -DBUILD_EXAMPLES=ON \
    -DBUILD_TESTS=ON \
    -DENABLE_VULKAN_BACKEND=ON \

# Сборка проекта
echo "🔨 Сборка проекта..."
cmake --build . --config Release -j$(nproc)
