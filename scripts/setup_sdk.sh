#!/bin/bash
# setup_sdk.sh - Автоматическая настройка SDK для HyperEngine
# Этот скрипт проверяет наличие необходимых SDK и помогает с их настройкой

echo "========================================"
echo "HyperEngine SDK Setup Script"
echo "========================================"
echo

# Функция для проверки команды
check_command() {
    if command -v "$1" >/dev/null 2>&1; then
        return 0
    else
        return 1
    fi
}

# Проверка CUDA Toolkit
echo "[1/4] Проверка CUDA Toolkit..."
if check_command nvcc; then
    echo "✓ CUDA Toolkit найден"
    nvcc --version | grep "release"
else
    echo "✗ CUDA Toolkit не найден"
    echo "  Скачайте с: https://developer.nvidia.com/cuda-downloads"
    echo "  Рекомендуемая версия: 11.8 или новее"
fi
echo

# Проверка OptiX SDK
echo "[2/4] Проверка OptiX SDK..."
if [ -f "$OPTIX_ROOT/include/optix.h" ]; then
    echo "✓ OptiX SDK найден: $OPTIX_ROOT"
else
    echo "✗ OptiX SDK не найден"
    echo "  Установите переменную окружения OPTIX_ROOT"
    echo "  Пример: export OPTIX_ROOT=/opt/optix"
    echo "  Скачайте с: https://developer.nvidia.com/optix"
fi
echo

# Проверка DLSS SDK (Streamline)
echo "[3/4] Проверка DLSS SDK (опционально)..."
if [ -f "$STREAMLINE_ROOT/include/sl.h" ]; then
    echo "✓ DLSS SDK найден: $STREAMLINE_ROOT"
else
    echo "⚠ DLSS SDK не найден (опционально)"
    echo "  Установите переменную окружения STREAMLINE_ROOT"
    echo "  Пример: export STREAMLINE_ROOT=/opt/streamline"
    echo "  Скачайте с: https://developer.nvidia.com/rtx/streamline"
fi
echo

# Проверка FidelityFX SDK
echo "[4/4] Проверка FidelityFX SDK..."
if [ -f "$FIDELITYFX_ROOT/sdk/include/FidelityFX/host/ffx_fsr2.h" ]; then
    echo "✓ FidelityFX SDK найден: $FIDELITYFX_ROOT"
else
    echo "⚠ FidelityFX SDK не найден"
    echo "  Установите переменную окружения FIDELITYFX_ROOT"
    echo "  Или клонируйте: git clone https://github.com/GPUOpen-Effects/FidelityFX-FSR2.git"
fi
echo

# Проверка Vulkan SDK
echo "Дополнительно: Проверка Vulkan SDK..."
if check_command vulkaninfo; then
    echo "✓ Vulkan SDK найден"
    vulkaninfo --summary 2>/dev/null | grep "Vulkan Instance Version" || echo "  (версия не определена)"
else
    echo "✗ Vulkan SDK не найден"
    echo "  Скачайте с: https://vulkan.lunarg.com/"
fi
echo

# Проверка vcpkg
echo "Дополнительно: Проверка vcpkg..."
if [ -f "vcpkg/vcpkg" ]; then
    echo "✓ vcpkg найден"
else
    echo "⚠ vcpkg не найден в текущей директории"
    echo "  Выполните: git clone https://github.com/Microsoft/vcpkg.git"
    echo "  Затем: ./vcpkg/bootstrap-vcpkg.sh"
fi
echo

echo "========================================"
echo "Рекомендации по сборке:"
echo "========================================"
echo

echo "Минимальная конфигурация (без дополнительных SDK):"
echo "cmake .. -DCMAKE_TOOLCHAIN_FILE=vcpkg/scripts/buildsystems/vcpkg.cmake \\"
echo "         -DBUILD_VULKAN_RENDERER=ON \\"
echo "         -DBUILD_WITH_CUDA=OFF \\"
echo "         -DBUILD_WITH_OPTIX=OFF \\"
echo "         -DBUILD_WITH_DLSS=OFF \\"
echo "         -DBUILD_WITH_FSR=OFF"
echo

echo "Полная конфигурация (все SDK):"
echo "cmake .. -DCMAKE_TOOLCHAIN_FILE=vcpkg/scripts/buildsystems/vcpkg.cmake \\"
echo "         -DBUILD_VULKAN_RENDERER=ON \\"
echo "         -DBUILD_WITH_CUDA=ON \\"
echo "         -DBUILD_WITH_OPTIX=ON \\"
echo "         -DBUILD_WITH_DLSS=ON \\"
echo "         -DBUILD_WITH_FSR=ON"
echo

echo "Для получения подробной информации см. docs/SDK_SETUP_GUIDE.md"
echo
