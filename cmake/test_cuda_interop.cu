// test_cuda_interop.cu - тест поддержки CUDA-Vulkan interop
#include <cuda_runtime.h>
#include <cuda.h>

// Проверяем наличие необходимых функций для external memory
int main() {
    // Проверка поддержки external memory
    #ifdef __CUDA_RUNTIME_H__
    cudaExternalMemoryHandleDesc memHandleDesc = {};
    cudaExternalMemory_t extMem;
    
    // Эти функции должны быть доступны для interop
    cudaError_t result = cudaImportExternalMemory(&extMem, &memHandleDesc);
    
    // Проверка поддержки external semaphores
    cudaExternalSemaphoreHandleDesc semHandleDesc = {};
    cudaExternalSemaphore_t extSem;
    result = cudaImportExternalSemaphore(&extSem, &semHandleDesc);
    
    return 0;
    #else
    return -1;
    #endif
}
