/**
 * @file cuda_stubs.cpp
 * @brief CUDA stubs для случаев когда CUDA недоступна
 * @author HyperEngine Team
 * @date 2024
 */

#include "cuda_stubs.h"

namespace HyperEngine {
namespace CUDA {

// Заглушки для CUDA функций
cudaError_t cudaMalloc(void** devPtr, size_t size) {
    (void)devPtr;
    (void)size;
    return cudaErrorNoDevice;
}

cudaError_t cudaFree(void* devPtr) {
    (void)devPtr;
    return cudaErrorNoDevice;
}

cudaError_t cudaMemcpy(void* dst, const void* src, size_t count, cudaMemcpyKind kind) {
    (void)dst;
    (void)src;
    (void)count;
    (void)kind;
    return cudaErrorNoDevice;
}

cudaError_t cudaDeviceSynchronize() {
    return cudaErrorNoDevice;
}

cudaError_t cudaGetDeviceCount(int* count) {
    if (count)
        *count = 0;
    return cudaErrorNoDevice;
}

cudaError_t cudaSetDevice(int device) {
    (void)device;
    return cudaErrorNoDevice;
}

cudaError_t cudaGetDeviceProperties(cudaDeviceProp* prop, int device) {
    (void)prop;
    (void)device;
    return cudaErrorNoDevice;
}

const char* cudaGetErrorString(cudaError_t error) {
    (void)error;
    return "CUDA not available";
}

}  // namespace CUDA
}  // namespace HyperEngine
