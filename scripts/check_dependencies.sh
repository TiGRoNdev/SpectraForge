#!/bin/bash

echo "========================================"
echo "Проверка зависимостей SpectraForge"
echo "========================================"

# Проверка системных пакетов
echo "[1/6] Проверка системных библиотек..."
MISSING_PKGS=()
for pkg in glfw3 glm vulkan glew; do
    if pkg-config --exists $pkg 2>/dev/null; then
        VERSION=$(pkg-config --modversion $pkg 2>/dev/null || echo "N/A")
        echo "✓ $pkg установлен (версия: $VERSION)"
    else
        echo "✗ $pkg не найден"
        MISSING_PKGS+=($pkg)
    fi
done

if [ ${#MISSING_PKGS[@]} -gt 0 ]; then
    echo "⚠️  Установите недостающие пакеты:"
    echo "  sudo apt-get install $(printf 'lib%s-dev ' "${MISSING_PKGS[@]}")"
fi

# Проверка CUDA
echo -e "\n[2/6] Проверка CUDA Toolkit..."
if command -v nvcc &> /dev/null; then
    echo "✓ CUDA Toolkit установлен"
    nvcc --version | head -1
else
    echo "✗ CUDA Toolkit не найден"
fi

# Проверка Vulkan
echo -e "\n[3/6] Проверка Vulkan SDK..."
if command -v vulkaninfo &> /dev/null; then
    echo "✓ Vulkan SDK установлен"
    vulkaninfo --version 2>/dev/null || echo "  (GPU недоступен в контейнере)"
else
    echo "✗ Vulkan SDK не найден"
fi

# Проверка OptiX
echo -e "\n[4/6] Проверка OptiX SDK..."
if [ -d "$OPTIX_ROOT" ] && [ -f "$OPTIX_ROOT/README.txt" ]; then
    echo "✓ OptiX SDK настроен (заглушка)"
    echo "  Путь: $OPTIX_ROOT"
else
    echo "✗ OptiX SDK не настроен"
fi

# Проверка FidelityFX
echo -e "\n[5/6] Проверка FidelityFX SDK..."
if [ -d "$FIDELITYFX_ROOT" ]; then
    echo "✓ FidelityFX SDK установлен"
    echo "  Путь: $FIDELITYFX_ROOT"
else
    echo "✗ FidelityFX SDK не найден"
fi

# Проверка DLSS
echo -e "\n[6/6] Проверка DLSS SDK..."
if [ -d "$STREAMLINE_ROOT" ] && [ -f "$STREAMLINE_ROOT/README.txt" ]; then
    echo "✓ DLSS SDK настроен (заглушка)"
    echo "  Путь: $STREAMLINE_ROOT"
else
    echo "✗ DLSS SDK не настроен"
fi

echo -e "\n========================================"
echo "Проверка завершена"
echo "========================================"
