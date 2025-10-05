/**
 * @file cuda_kernels.cu
 * @brief CUDA kernels для демонстрации
 */

#include <cuda_runtime.h>

// CUDA kernel для обработки данных
__global__ void processBufferKernel(float* data, size_t size, float multiplier) {
    int idx = blockIdx.x * blockDim.x + threadIdx.x;
    if (idx < size) {
        data[idx] *= multiplier;
    }
}

// C wrapper функция
extern "C" void launchProcessBufferKernel(float* data, size_t size, float multiplier) {
    const int blockSize = 256;
    const int gridSize = (size + blockSize - 1) / blockSize;

    processBufferKernel<<<gridSize, blockSize>>>(data, size, multiplier);
}
