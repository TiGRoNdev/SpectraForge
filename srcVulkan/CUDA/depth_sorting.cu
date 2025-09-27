/**
 * @file depth_sorting.cu
 * @brief CUDA kernels для высокопроизводительной сортировки по глубине
 * 
 * Реализует оптимизированные алгоритмы сортировки гауссианов по глубине
 * для правильного порядка рендеринга в 3D Gaussian Splatting.
 */

#include <cuda_runtime.h>
#include <device_launch_parameters.h>
#include <cub/cub.cuh>
#include <thrust/sort.h>
#include <thrust/device_vector.h>
#include <thrust/execution_policy.h>

// Определяем необходимые типы локально, чтобы избежать зависимостей от Vulkan
namespace Engine3D::CUDA {
    /**
     * @brief Проецированный гауссиан на экране (локальное определение)
     */
    struct ProjectedGaussian {
        float2 center;             // Центр на экране (пиксели)
        float2 axis1, axis2;       // Главные оси эллипса
        float depth;               // Глубина для сортировки
        float4 color;              // Цвет и прозрачность
        float radius;              // Радиус влияния (пиксели)
        uint32_t gaussianID;       // ID исходного гауссиана
    };
}

namespace Engine3D::CUDA {

/**
 * @brief Структура для сортировки (ключ-значение)
 */
struct DepthSortPair {
    float depth;        // Ключ сортировки (глубина)
    uint32_t index;     // Значение (индекс гауссиана)
    
    __device__ __host__ DepthSortPair() : depth(0.0f), index(0) {}
    __device__ __host__ DepthSortPair(float d, uint32_t i) : depth(d), index(i) {}
};

/**
 * @brief Оператор сравнения для сортировки по глубине (от близкого к дальнему)
 */
struct DepthComparator {
    __device__ __host__ bool operator()(const DepthSortPair& a, const DepthSortPair& b) const {
        return a.depth < b.depth; // Ближние объекты первыми
    }
};

/**
 * @brief Kernel для подготовки данных к сортировке
 * @param projected Проецированные гауссианы
 * @param sortPairs Выходные пары для сортировки
 * @param numGaussians Количество гауссианов
 */
__global__ void prepareSortPairs(
    const Engine3D::CUDA::ProjectedGaussian* projected,
    DepthSortPair* sortPairs,
    int numGaussians
) {
    int idx = blockIdx.x * blockDim.x + threadIdx.x;
    if (idx >= numGaussians) return;
    
    const Engine3D::CUDA::ProjectedGaussian& proj = projected[idx];
    
    // Создаем пару ключ-значение для сортировки
    sortPairs[idx].depth = proj.depth;
    sortPairs[idx].index = idx;
}

/**
 * @brief Kernel для извлечения отсортированных индексов
 * @param sortPairs Отсортированные пары
 * @param sortedIndices Выходные отсортированные индексы
 * @param numGaussians Количество гауссианов
 */
__global__ void extractSortedIndices(
    const DepthSortPair* sortPairs,
    uint32_t* sortedIndices,
    int numGaussians
) {
    int idx = blockIdx.x * blockDim.x + threadIdx.x;
    if (idx >= numGaussians) return;
    
    sortedIndices[idx] = sortPairs[idx].index;
}

/**
 * @brief Kernel для быстрой локальной сортировки малых групп
 * Использует shared memory для сортировки групп до 1024 элементов
 * @param sortPairs Пары для сортировки
 * @param groupSize Размер группы
 * @param numGroups Количество групп
 */
__global__ void localBitonicSort(
    DepthSortPair* sortPairs,
    int groupSize,
    int numGroups
) {
    extern __shared__ DepthSortPair sharedData[];
    
    int groupIdx = blockIdx.x;
    int tid = threadIdx.x;
    
    if (groupIdx >= numGroups) return;
    
    int groupStart = groupIdx * groupSize;
    int elementsInGroup = min(groupSize, numGroups * groupSize - groupStart);
    
    // Загружаем данные в shared memory
    if (tid < elementsInGroup) {
        sharedData[tid] = sortPairs[groupStart + tid];
    } else {
        // Заполняем "пустые" элементы максимальным значением
        sharedData[tid].depth = FLT_MAX;
        sharedData[tid].index = 0xFFFFFFFF;
    }
    __syncthreads();
    
    // Bitonic sort
    for (int size = 2; size <= blockDim.x; size *= 2) {
        // Определяем направление сортировки (восходящий/нисходящий)
        bool ascending = ((tid & (size / 2)) == 0);
        
        for (int stride = size / 2; stride > 0; stride /= 2) {
            int partner = tid ^ stride;
            
            if (partner < blockDim.x) {
                bool shouldSwap = (sharedData[tid].depth > sharedData[partner].depth) == ascending;
                
                if (shouldSwap) {
                    DepthSortPair temp = sharedData[tid];
                    sharedData[tid] = sharedData[partner];
                    sharedData[partner] = temp;
                }
            }
            __syncthreads();
        }
    }
    
    // Записываем результат обратно в глобальную память
    if (tid < elementsInGroup) {
        sortPairs[groupStart + tid] = sharedData[tid];
    }
}

/**
 * @brief Kernel для сортировки по тайлам (tile-aware sorting)
 * Сортирует гауссианы внутри каждого тайла отдельно
 * @param projected Проецированные гауссианы
 * @param tileAssignments Назначения тайлов
 * @param sortedIndices Выходные отсортированные индексы
 * @param tileOffsets Смещения тайлов
 * @param numGaussians Количество гауссианов
 * @param numTiles Количество тайлов
 */
__global__ void sortByTiles(
    const Engine3D::CUDA::ProjectedGaussian* projected,
    const uint32_t* tileAssignments,
    uint32_t* sortedIndices,
    uint32_t* tileOffsets,
    int numGaussians,
    int numTiles
) {
    int tileID = blockIdx.x;
    if (tileID >= numTiles) return;
    
    // Получаем диапазон гауссианов для этого тайла
    uint32_t start = tileOffsets[tileID];
    uint32_t end = (tileID + 1 < numTiles) ? tileOffsets[tileID + 1] : numGaussians;
    uint32_t count = end - start;
    
    if (count == 0) return;
    
    // Используем shared memory для локальной сортировки
    extern __shared__ DepthSortPair tileData[];
    
    // Загружаем данные тайла
    for (int i = threadIdx.x; i < count; i += blockDim.x) {
        if (start + i < end) {
            uint32_t gaussianIdx = sortedIndices[start + i];
            tileData[i].depth = projected[gaussianIdx].depth;
            tileData[i].index = gaussianIdx;
        }
    }
    __syncthreads();
    
    // Простая сортировка вставками для малых массивов
    if (threadIdx.x == 0) {
        for (int i = 1; i < count; i++) {
            DepthSortPair key = tileData[i];
            
            int j = i - 1;
            while (j >= 0 && tileData[j].depth > key.depth) {
                tileData[j + 1] = tileData[j];
                j--;
            }
            tileData[j + 1] = key;
        }
    }
    __syncthreads();
    
    // Записываем отсортированные индексы обратно
    for (int i = threadIdx.x; i < count; i += blockDim.x) {
        if (start + i < end) {
            sortedIndices[start + i] = tileData[i].index;
        }
    }
}

/**
 * @brief Kernel для построения списков тайлов (tile lists)
 * Создает компактные списки гауссианов для каждого тайла
 * @param tileAssignments Назначения тайлов
 * @param sortedIndices Отсортированные индексы
 * @param tileOffsets Выходные смещения тайлов
 * @param numGaussians Количество гауссианов
 * @param numTiles Количество тайлов
 */
__global__ void buildTileLists(
    const uint32_t* tileAssignments,
    const uint32_t* sortedIndices,
    uint32_t* tileOffsets,
    int numGaussians,
    int numTiles
) {
    int idx = blockIdx.x * blockDim.x + threadIdx.x;
    if (idx >= numGaussians) return;
    
    uint32_t gaussianIdx = sortedIndices[idx];
    uint32_t tileID = tileAssignments[gaussianIdx];
    
    if (tileID != 0xFFFFFFFF && tileID < numTiles) {
        // Атомарно увеличиваем счетчик для этого тайла
        atomicAdd(&tileOffsets[tileID], 1);
    }
}

    /**
     * @brief Kernel для извлечения ключей и значений из пар
     */
    __global__ void extractKeysValuesKernel(
        const DepthSortPair* pairs, 
        float* keys, 
        uint32_t* values, 
        int n
    ) {
        int idx = blockIdx.x * blockDim.x + threadIdx.x;
        if (idx < n) {
            keys[idx] = pairs[idx].depth;
            values[idx] = pairs[idx].index;
        }
    }

} // namespace Engine3D::CUDA

/**
 * @brief Host функции для управления сортировкой с C linkage
 */
extern "C" {
    
    /**
     * @brief Быстрая сортировка с использованием CUB библиотеки
     */
    cudaError_t launchCubRadixSort(
        const Engine3D::CUDA::ProjectedGaussian* d_projected,
        uint32_t* d_sortedIndices,
        int numGaussians,
        cudaStream_t stream = 0
    ) {
        using namespace Engine3D::CUDA;
        
        // Временные буферы для сортировки
        DepthSortPair* d_sortPairs = nullptr;
        DepthSortPair* d_sortPairsOut = nullptr;
        void* d_tempStorage = nullptr;
        
        try {
            // Выделяем память для пар ключ-значение
            cudaMalloc(&d_sortPairs, numGaussians * sizeof(DepthSortPair));
            cudaMalloc(&d_sortPairsOut, numGaussians * sizeof(DepthSortPair));
            
            // Подготовка пар ключ-значение
            dim3 blockSize(256);
            dim3 gridSize((numGaussians + blockSize.x - 1) / blockSize.x);
            
            prepareSortPairs<<<gridSize, blockSize, 0, stream>>>(
                d_projected, d_sortPairs, numGaussians
            );
            
            // Определяем необходимый размер временного хранилища для CUB
            size_t tempStorageBytes = 0;
            cub::DeviceRadixSort::SortPairs(
                d_tempStorage, tempStorageBytes,
                (float*)nullptr, (float*)nullptr,  // keys
                (uint32_t*)nullptr, (uint32_t*)nullptr,  // values
                numGaussians, 0, sizeof(float) * 8, stream
            );
            
            // Выделяем временное хранилище
            cudaMalloc(&d_tempStorage, tempStorageBytes);
            
            // Извлекаем ключи и значения из структуры
            float* d_keys;
            float* d_keysOut;
            uint32_t* d_values;
            uint32_t* d_valuesOut;
            
            cudaMalloc(&d_keys, numGaussians * sizeof(float));
            cudaMalloc(&d_keysOut, numGaussians * sizeof(float));
            cudaMalloc(&d_values, numGaussians * sizeof(uint32_t));
            cudaMalloc(&d_valuesOut, numGaussians * sizeof(uint32_t));
            
            // Извлекаем ключи и значения из структуры
            Engine3D::CUDA::extractKeysValuesKernel<<<gridSize, blockSize, 0, stream>>>(
                d_sortPairs, d_keys, d_values, numGaussians
            );
            
            // Выполняем CUB сортировку ключей и значений
            cub::DeviceRadixSort::SortPairs(
                d_tempStorage, tempStorageBytes,
                d_keys, d_keysOut,
                d_values, d_valuesOut,
                numGaussians, 0, sizeof(float) * 8, stream
            );
            
            // Копируем отсортированные индексы в выходной буфер
            cudaMemcpy(d_sortedIndices, d_valuesOut, numGaussians * sizeof(uint32_t), cudaMemcpyDeviceToDevice);
            
            // Освобождаем временную память
            cudaFree(d_keys);
            cudaFree(d_keysOut);
            cudaFree(d_values);
            cudaFree(d_valuesOut);
            cudaFree(d_sortPairs);
            cudaFree(d_sortPairsOut);
            cudaFree(d_tempStorage);
            
            return cudaGetLastError();
            
        } catch (...) {
            // Освобождаем память в случае ошибки
            if (d_sortPairs) cudaFree(d_sortPairs);
            if (d_sortPairsOut) cudaFree(d_sortPairsOut);
            if (d_tempStorage) cudaFree(d_tempStorage);
            
            return cudaErrorUnknown;
        }
    }
    
    /**
     * @brief Сортировка с использованием Thrust (альтернативный метод)
     */
    cudaError_t launchThrustSort(
        const Engine3D::CUDA::ProjectedGaussian* d_projected,
        uint32_t* d_sortedIndices,
        int numGaussians,
        cudaStream_t stream = 0
    ) {
        using namespace Engine3D::CUDA;
        
        try {
            // Создаем temporary vectors
            thrust::device_vector<DepthSortPair> sortPairs(numGaussians);
            
            // Подготавливаем данные
            dim3 blockSize(256);
            dim3 gridSize((numGaussians + blockSize.x - 1) / blockSize.x);
            
            prepareSortPairs<<<gridSize, blockSize, 0, stream>>>(
                d_projected, thrust::raw_pointer_cast(sortPairs.data()), numGaussians
            );
            
            // Сортируем с помощью Thrust
            thrust::sort(thrust::cuda::par.on(stream), 
                        sortPairs.begin(), sortPairs.end(), 
                        DepthComparator());
            
            // Извлекаем отсортированные индексы
            extractSortedIndices<<<gridSize, blockSize, 0, stream>>>(
                thrust::raw_pointer_cast(sortPairs.data()), d_sortedIndices, numGaussians
            );
            
            return cudaGetLastError();
            
        } catch (...) {
            return cudaErrorUnknown;
        }
    }
    
    /**
     * @brief Сортировка с учетом тайлов
     */
    cudaError_t launchTileAwareSorting(
        const Engine3D::CUDA::ProjectedGaussian* d_projected,
        const uint32_t* d_tileAssignments,
        uint32_t* d_sortedIndices,
        uint32_t* d_tileOffsets,
        int numGaussians,
        int numTiles,
        cudaStream_t stream = 0
    ) {
        using namespace Engine3D::CUDA;
        
        try {
            // Сначала выполняем обычную сортировку по глубине
            cudaError_t result = launchCubRadixSort(d_projected, d_sortedIndices, numGaussians, stream);
            if (result != cudaSuccess) {
                return result;
            }
            
            // Затем строим списки тайлов
            dim3 blockSize(256);
            dim3 gridSize((numGaussians + blockSize.x - 1) / blockSize.x);
            
            // Инициализируем смещения тайлов
            cudaMemset(d_tileOffsets, 0, (numTiles + 1) * sizeof(uint32_t));
            
            // Строим списки тайлов
            buildTileLists<<<gridSize, blockSize, 0, stream>>>(
                d_tileAssignments, d_sortedIndices, d_tileOffsets, 
                numGaussians, numTiles
            );
            
            // Сортируем внутри каждого тайла
            dim3 tileGridSize(numTiles);
            dim3 tileBlockSize(256);
            size_t sharedMemSize = 256 * sizeof(DepthSortPair); // Макс. 256 гауссианов на тайл
            
            sortByTiles<<<tileGridSize, tileBlockSize, sharedMemSize, stream>>>(
                d_projected, d_tileAssignments, d_sortedIndices, d_tileOffsets,
                numGaussians, numTiles
            );
            
            return cudaGetLastError();
            
        } catch (...) {
            return cudaErrorUnknown;
        }
    }
    
    /**
     * @brief Битонная сортировка для малых данных
     */
    cudaError_t launchBitonicSort(
        const Engine3D::CUDA::ProjectedGaussian* d_projected,
        uint32_t* d_sortedIndices,
        int numGaussians,
        cudaStream_t stream = 0
    ) {
        using namespace Engine3D::CUDA;
        
        try {
            // Ограничиваем битонную сортировку группами по 1024 элемента
            const int GROUP_SIZE = 1024;
            int numGroups = (numGaussians + GROUP_SIZE - 1) / GROUP_SIZE;
            
            DepthSortPair* d_sortPairs = nullptr;
            cudaMalloc(&d_sortPairs, numGaussians * sizeof(DepthSortPair));
            
            // Подготовка данных
            dim3 blockSize(256);
            dim3 gridSize((numGaussians + blockSize.x - 1) / blockSize.x);
            
            prepareSortPairs<<<gridSize, blockSize, 0, stream>>>(
                d_projected, d_sortPairs, numGaussians
            );
            
            // Локальная битонная сортировка групп
            dim3 localGrid(numGroups);
            dim3 localBlock(GROUP_SIZE);
            size_t sharedMemSize = GROUP_SIZE * sizeof(DepthSortPair);
            
            localBitonicSort<<<localGrid, localBlock, sharedMemSize, stream>>>(
                d_sortPairs, GROUP_SIZE, numGroups
            );
            
            // Извлекаем результаты
            extractSortedIndices<<<gridSize, blockSize, 0, stream>>>(
                d_sortPairs, d_sortedIndices, numGaussians
            );
            
            cudaFree(d_sortPairs);
            return cudaGetLastError();
            
        } catch (...) {
            return cudaErrorUnknown;
        }
    }
}