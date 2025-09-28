/**
 * @file cuda_stubs.cpp
 * @brief Заглушки для CUDA функций когда CUDA не доступна
 * 
 * Эти функции предоставляют пустые реализации CUDA функций
 * для случаев когда CUDA код не скомпилирован но вызывается.
 */

#include <iostream>
#include "HyperEngine/Core/Console.h"

// Определяем базовые CUDA типы если не доступны
#ifndef __CUDA_RUNTIME_H__
struct float4 {
    float x, y, z, w;
};

struct float3 {
    float x, y, z;
};
#endif

// Заглушки для CUDA функций из FlashGSSplatter

extern "C" {

void launchGaussianInitialization(
    const float4* positions,
    int numPoints,
    float4* gaussians,
    float3* colors,
    float* opacities,
    float* scales,
    int* activeMask
) {
    SAFE_PRINT_LINE("[CUDA Stub] launchGaussianInitialization called (CUDA not available)");
    (void)positions; (void)numPoints; (void)gaussians; (void)colors;
    (void)opacities; (void)scales; (void)activeMask;
}

void launchGradientComputation(
    const float4* positions,
    const float4* colors,
    float4* positionGrads,
    float4* colorGrads,
    float* opacityGrads,
    float* scaleGrads,
    int numGaussians
) {
    SAFE_PRINT_LINE("[CUDA Stub] launchGradientComputation called (CUDA not available)");
    (void)positions; (void)colors; (void)positionGrads; (void)colorGrads;
    (void)opacityGrads; (void)scaleGrads; (void)numGaussians;
}

void launchParameterUpdate(
    float4* positions,
    float3* colors,
    float* opacities,
    float* scales,
    const float4* positionGrads,
    const float4* colorGrads,
    const float* opacityGrads,
    const float* scaleGrads,
    float learningRate,
    int numGaussians
) {
    SAFE_PRINT_LINE("[CUDA Stub] launchParameterUpdate called (CUDA not available)");
    (void)positions; (void)colors; (void)opacities; (void)scales;
    (void)positionGrads; (void)colorGrads; (void)opacityGrads; (void)scaleGrads;
    (void)learningRate; (void)numGaussians;
}

void launchGaussianProjection(
    const float4* gaussians,
    const float* scales,
    const float* viewMatrix,
    const float* projMatrix,
    float4* projectedGaussians,
    float* depths,
    int* tileAssignments,
    int numGaussians,
    int screenWidth,
    int screenHeight
) {
    SAFE_PRINT_LINE("[CUDA Stub] launchGaussianProjection called (CUDA not available)");
    (void)gaussians; (void)scales; (void)viewMatrix; (void)projMatrix;
    (void)projectedGaussians; (void)depths; (void)tileAssignments;
    (void)numGaussians; (void)screenWidth; (void)screenHeight;
}

void launchTileAssignment(
    const float4* projectedGaussians,
    const float* depths,
    int* tileAssignments,
    int* gaussianIndices,
    int numGaussians,
    int numTiles
) {
    SAFE_PRINT_LINE("[CUDA Stub] launchTileAssignment called (CUDA not available)");
    (void)projectedGaussians; (void)depths; (void)tileAssignments;
    (void)gaussianIndices; (void)numGaussians; (void)numTiles;
}

void launchTileRasterization(
    const float4* projectedGaussians,
    const float3* colors,
    const float* opacities,
    const int* gaussianIndices,
    const int* tileAssignments,
    float4* outputBuffer,
    float* depthBuffer,
    int numTiles,
    int tileSize,
    int screenWidth,
    int screenHeight
) {
    SAFE_PRINT_LINE("[CUDA Stub] launchTileRasterization called (CUDA not available)");
    (void)projectedGaussians; (void)colors; (void)opacities; (void)gaussianIndices;
    (void)tileAssignments; (void)outputBuffer; (void)depthBuffer;
    (void)numTiles; (void)tileSize; (void)screenWidth; (void)screenHeight;
}

void launchCubRadixSort(
    float* keys,
    int* values,
    int numElements,
    void* tempStorage,
    size_t tempStorageSize
) {
    SAFE_PRINT_LINE("[CUDA Stub] launchCubRadixSort called (CUDA not available)");
    (void)keys; (void)values; (void)numElements; (void)tempStorage; (void)tempStorageSize;
}

} // extern "C"

