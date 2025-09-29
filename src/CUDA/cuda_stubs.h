/**
 * @file cuda_stubs.h
 * @brief Заголовочный файл для CUDA stubs
 * @author HyperEngine Team
 * @date 2024
 */

#pragma once

#include <cstddef>

namespace HyperEngine {
namespace CUDA {

// Типы данных CUDA
typedef int cudaError_t;
typedef int cudaMemcpyKind;

// Константы ошибок
constexpr cudaError_t cudaSuccess = 0;
constexpr cudaError_t cudaErrorNoDevice = 8;

// Константы для cudaMemcpy
constexpr cudaMemcpyKind cudaMemcpyHostToDevice = 1;
constexpr cudaMemcpyKind cudaMemcpyDeviceToHost = 2;
constexpr cudaMemcpyKind cudaMemcpyDeviceToDevice = 3;

// Структура свойств устройства
struct cudaDeviceProp {
    char name[256];
    int major;
    int minor;
    int multiProcessorCount;
    size_t totalGlobalMem;
    size_t sharedMemPerBlock;
    int maxThreadsPerBlock;
    int maxThreadsDim[3];
    int maxGridSize[3];
    int clockRate;
    size_t totalConstMem;
    int textureAlignment;
    int deviceOverlap;
    int multiprocessorCount;
    int kernelExecTimeoutEnabled;
    int integrated;
    int canMapHostMemory;
    int computeMode;
    int maxTexture1D;
    int maxTexture2D[2];
    int maxTexture3D[3];
    int maxTexture1DLayered[2];
    int maxTexture2DLayered[3];
    int maxSurface1D;
    int maxSurface2D[2];
    int maxSurface3D[3];
    int maxSurface1DLayered[2];
    int maxSurface2DLayered[3];
    size_t surfaceAlignment;
    int concurrentKernels;
    int ECCEnabled;
    int pciBusID;
    int pciDeviceID;
    int pciDomainID;
    int tccDriver;
    int asyncEngineCount;
    int unifiedAddressing;
    int memoryClockRate;
    int memoryBusWidth;
    int l2CacheSize;
    int maxThreadsPerMultiProcessor;
    int globalL1CacheSupported;
    int localL1CacheSupported;
    size_t sharedMemPerMultiprocessor;
    int regsPerMultiprocessor;
    int managedMemory;
    int isMultiGpuBoard;
    int multiGpuBoardGroupID;
    int singleToDoublePrecisionPerfRatio;
    int pageableMemoryAccess;
    int concurrentManagedAccess;
    int computePreemptionSupported;
    int canUseHostPointerForRegisteredMem;
    int cooperativeLaunch;
    int cooperativeMultiDeviceLaunch;
    int sharedMemPerBlockOptin;
    int pageableMemoryAccessUsesHostPageTables;
    int directManagedMemAccessFromHost;
    int maxBlocksPerMultiProcessor;
    int accessPolicyMaxWindowSize;
    int reservedSharedMemPerBlock;
};

// Объявления функций
cudaError_t cudaMalloc(void** devPtr, size_t size);
cudaError_t cudaFree(void* devPtr);
cudaError_t cudaMemcpy(void* dst, const void* src, size_t count, cudaMemcpyKind kind);
cudaError_t cudaDeviceSynchronize();
cudaError_t cudaGetDeviceCount(int* count);
cudaError_t cudaSetDevice(int device);
cudaError_t cudaGetDeviceProperties(cudaDeviceProp* prop, int device);
const char* cudaGetErrorString(cudaError_t error);

}  // namespace CUDA
}  // namespace HyperEngine
