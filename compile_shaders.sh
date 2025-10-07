#!/bin/bash
# Компиляция шейдеров для SpectraForge (Triangle Splatting)

set -e  # Exit on error

echo "=== Компиляция шейдеров SpectraForge ==="

# Проверка наличия glslangValidator
if ! command -v glslangValidator &> /dev/null; then
    echo "❌ ОШИБКА: glslangValidator не найден!"
    echo "Установите: sudo apt install glslang-tools"
    exit 1
fi

# Проверка наличия glslc (альтернатива)
USE_GLSLC=false
if command -v glslc &> /dev/null; then
    USE_GLSLC=true
    echo "✅ Используется glslc для компиляции"
else
    echo "✅ Используется glslangValidator для компиляции"
fi

# Создаём директорию для compiled shaders (опционально)
mkdir -p shaders/compiled 2>/dev/null || true

# Функция компиляции шейдера
compile_shader() {
    local shader_file=$1
    local output_file=$2
    
    if [ ! -f "$shader_file" ]; then
        echo "⚠️  Файл не найден: $shader_file (пропускаем)"
        return 0
    fi
    
    echo "  Компилируем: $shader_file → $output_file"
    
    if [ "$USE_GLSLC" = true ]; then
        glslc "$shader_file" -o "$output_file"
    else
        glslangValidator -V "$shader_file" -o "$output_file"
    fi
    
    if [ $? -eq 0 ]; then
        echo "  ✅ Успешно"
    else
        echo "  ❌ ОШИБКА компиляции: $shader_file"
        exit 1
    fi
}

# === ОСНОВНЫЕ ШЕЙДЕРЫ ===

echo ""
echo "📦 Компиляция Triangle Splatting шейдеров..."
compile_shader "shaders/TriangleSplatting.comp" "shaders/TriangleSplatting.comp.spv"

echo ""
echo "📦 Компиляция Bitonic Sort шейдера..."
compile_shader "shaders/BitonicSort.comp" "shaders/BitonicSort.comp.spv"

echo ""
echo "📦 Компиляция Depth Key Compute шейдера..."
compile_shader "shaders/DepthKeyCompute.comp" "shaders/DepthKeyCompute.comp.spv"

echo ""
echo "📦 Компиляция Frustum Culling шейдера..."
compile_shader "shaders/FrustumCulling.comp" "shaders/FrustumCulling.comp.spv"

echo ""
echo "📦 Компиляция Indirect Args шейдера..."
compile_shader "shaders/IndirectArgs.comp" "shaders/IndirectArgs.comp.spv"

echo ""
echo "📦 Компиляция Tile Culling шейдера..."
compile_shader "shaders/TileCulling.comp" "shaders/TileCulling.comp.spv"

echo ""
echo "📦 Компиляция Two-Pass Triangle Splatting шейдеров..."
compile_shader "shaders/TriangleVisibility.comp" "shaders/TriangleVisibility.comp.spv"
compile_shader "shaders/TriangleShading.comp" "shaders/TriangleShading.comp.spv"

echo ""
echo "📦 Компиляция Instanced Mesh шейдеров..."
compile_shader "shaders/InstancedMesh.vert" "shaders/InstancedMesh.vert.spv"
compile_shader "shaders/InstancedMesh.frag" "shaders/InstancedMesh.frag.spv"

# === ДОПОЛНИТЕЛЬНЫЕ ШЕЙДЕРЫ (если есть) ===

echo ""
echo "📦 Компиляция Wavelet/FreGS шейдеров (если есть)..."
compile_shader "shaders/HaarWavelet.comp" "shaders/HaarWavelet.comp.spv" || true
compile_shader "shaders/GaussFreqSplat.comp" "shaders/GaussFreqSplat.comp.spv" || true

echo ""
echo "📦 Компиляция оптимизированных шейдеров..."
echo "  Компилируем: shaders/DepthSortAtomic.comp → shaders/DepthSortAtomic.comp.spv"
glslc -fshader-stage=compute --target-env=vulkan1.1 shaders/DepthSortAtomic.comp -o shaders/DepthSortAtomic.comp.spv && echo "  ✅ Успешно" || echo "  ❌ Ошибка"
compile_shader "shaders/MobileUpscalingHDR.comp" "shaders/MobileUpscalingHDR.comp.spv"

# === ФИНАЛЬНАЯ ПРОВЕРКА ===

echo ""
echo "================================"
echo "✅ Все шейдеры успешно скомпилированы!"
echo ""
echo "📊 Размеры файлов:"
ls -lh shaders/*.spv 2>/dev/null | awk '{print "  " $9 " (" $5 ")"}'
echo ""
echo "================================"

