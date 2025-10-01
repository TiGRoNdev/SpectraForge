/**
 * @file CuFFTBackend.cpp
 */

#include "HyperEngine/Rendering/FreqVox/Backends/CuFFTBackend.h"
#include "HyperEngine/Core/SafeConsole.h"
#include <stdexcept>

namespace HyperEngine::Rendering::FreqVox::Backends {

CuFFTBackend::CuFFTBackend() {
#ifdef HYPERENGINE_CUDA_AVAILABLE
    SAFE_PRINT_LINE("[CuFFTBackend] Конструктор вызван (CUDA доступна)");
#else
    SAFE_WARNING("[CuFFTBackend] Создан без поддержки CUDA");
#endif
}

CuFFTBackend::~CuFFTBackend() {
    shutdown();
}

bool CuFFTBackend::initialize(const DctBlockConfig& config) {
#ifndef HYPERENGINE_CUDA_AVAILABLE
    SAFE_ERROR("[CuFFTBackend] CUDA недоступна - сборка без BUILD_WITH_CUDA");
    return false;
#else
    if (initialized_) {
        SAFE_WARNING("[CuFFTBackend] Уже инициализирован");
        return true;
    }

    cfg_ = config;

    // Создаём CUDA stream
    cudaError_t err = cudaStreamCreate(&stream_);
    if (err != cudaSuccess) {
        SAFE_ERROR("[CuFFTBackend] Ошибка создания CUDA stream: " + std::string(cudaGetErrorString(err)));
        return false;
    }

    if (!createPlans()) {
        shutdown();
        return false;
    }

    size_t required_size = cfg_.batch_count * cfg_.block_width * cfg_.block_height * sizeof(float);
    if (!setupDeviceMemory(required_size)) {
        shutdown();
        return false;
    }

    initialized_ = true;
    SAFE_PRINT_LINE("[CuFFTBackend] Инициализирован: блоки " << cfg_.block_width << "x" << cfg_.block_height
                    << ", батчей=" << cfg_.batch_count);
    return true;
#endif
}

void CuFFTBackend::shutdown() {
#ifdef HYPERENGINE_CUDA_AVAILABLE
    if (!initialized_) return;

    destroyPlans();

    if (d_buffer_) {
        cudaFree(d_buffer_);
        d_buffer_ = nullptr;
        buffer_size_ = 0;
    }

    if (stream_) {
        cudaStreamDestroy(stream_);
        stream_ = nullptr;
    }

    initialized_ = false;
    SAFE_PRINT_LINE("[CuFFTBackend] Завершение работы");
#endif
}

bool CuFFTBackend::transform_forward(std::vector<float>& io_block_batched) {
#ifndef HYPERENGINE_CUDA_AVAILABLE
    (void)io_block_batched;
    return false;
#else
    if (!initialized_) {
        SAFE_ERROR("[CuFFTBackend] Не инициализирован");
        return false;
    }

    size_t data_size = io_block_batched.size() * sizeof(float);
    if (data_size > buffer_size_) {
        SAFE_ERROR("[CuFFTBackend] Размер данных превышает буфер");
        return false;
    }

    // Копируем на GPU
    cudaError_t err = cudaMemcpyAsync(d_buffer_, io_block_batched.data(), data_size,
                                      cudaMemcpyHostToDevice, stream_);
    if (err != cudaSuccess) {
        SAFE_ERROR("[CuFFTBackend] Ошибка копирования H->D: " + std::string(cudaGetErrorString(err)));
        return false;
    }

    // Выполняем FFT (используем как основу для DCT)
    cufftResult res = cufftExecR2C(plan_forward_, d_buffer_, (cufftComplex*)d_buffer_);
    if (res != CUFFT_SUCCESS) {
        SAFE_ERROR("[CuFFTBackend] Ошибка cufftExecR2C: код " + std::to_string(res));
        return false;
    }

    // Копируем обратно
    err = cudaMemcpyAsync(io_block_batched.data(), d_buffer_, data_size,
                          cudaMemcpyDeviceToHost, stream_);
    if (err != cudaSuccess) {
        SAFE_ERROR("[CuFFTBackend] Ошибка копирования D->H: " + std::string(cudaGetErrorString(err)));
        return false;
    }

    cudaStreamSynchronize(stream_);
    return true;
#endif
}

bool CuFFTBackend::transform_inverse(std::vector<float>& io_block_batched) {
#ifndef HYPERENGINE_CUDA_AVAILABLE
    (void)io_block_batched;
    return false;
#else
    if (!initialized_) {
        SAFE_ERROR("[CuFFTBackend] Не инициализирован");
        return false;
    }

    size_t data_size = io_block_batched.size() * sizeof(float);
    
    cudaError_t err = cudaMemcpyAsync(d_buffer_, io_block_batched.data(), data_size,
                                      cudaMemcpyHostToDevice, stream_);
    if (err != cudaSuccess) {
        SAFE_ERROR("[CuFFTBackend] Ошибка копирования H->D: " + std::string(cudaGetErrorString(err)));
        return false;
    }

    cufftResult res = cufftExecC2R(plan_inverse_, (cufftComplex*)d_buffer_, d_buffer_);
    if (res != CUFFT_SUCCESS) {
        SAFE_ERROR("[CuFFTBackend] Ошибка cufftExecC2R: код " + std::to_string(res));
        return false;
    }

    err = cudaMemcpyAsync(io_block_batched.data(), d_buffer_, data_size,
                          cudaMemcpyDeviceToHost, stream_);
    if (err != cudaSuccess) {
        SAFE_ERROR("[CuFFTBackend] Ошибка копирования D->H: " + std::string(cudaGetErrorString(err)));
        return false;
    }

    cudaStreamSynchronize(stream_);
    return true;
#endif
}

#ifdef HYPERENGINE_CUDA_AVAILABLE
bool CuFFTBackend::createPlans() {
    // Создаём план для батчированного 2D FFT
    int n[2] = {static_cast<int>(cfg_.block_height), static_cast<int>(cfg_.block_width)};
    
    cufftResult res = cufftPlanMany(&plan_forward_, 2, n,
                                    nullptr, 1, 0,
                                    nullptr, 1, 0,
                                    CUFFT_R2C, cfg_.batch_count);
    if (res != CUFFT_SUCCESS) {
        SAFE_ERROR("[CuFFTBackend] Ошибка создания forward плана: " + std::to_string(res));
        return false;
    }

    res = cufftPlanMany(&plan_inverse_, 2, n,
                       nullptr, 1, 0,
                       nullptr, 1, 0,
                       CUFFT_C2R, cfg_.batch_count);
    if (res != CUFFT_SUCCESS) {
        SAFE_ERROR("[CuFFTBackend] Ошибка создания inverse плана: " + std::to_string(res));
        cufftDestroy(plan_forward_);
        return false;
    }

    cufftSetStream(plan_forward_, stream_);
    cufftSetStream(plan_inverse_, stream_);

    SAFE_PRINT_LINE("[CuFFTBackend] cuFFT планы созданы успешно");
    return true;
}

void CuFFTBackend::destroyPlans() {
    if (plan_forward_) {
        cufftDestroy(plan_forward_);
        plan_forward_ = 0;
    }
    if (plan_inverse_) {
        cufftDestroy(plan_inverse_);
        plan_inverse_ = 0;
    }
}

bool CuFFTBackend::setupDeviceMemory(size_t required_size) {
    // Выделяем буфер на GPU (с запасом для комплексных чисел)
    size_t alloc_size = required_size * 2; // Запас для complex
    
    cudaError_t err = cudaMalloc(&d_buffer_, alloc_size);
    if (err != cudaSuccess) {
        SAFE_ERROR("[CuFFTBackend] Ошибка cudaMalloc: " + std::string(cudaGetErrorString(err)));
        return false;
    }

    buffer_size_ = alloc_size;
    SAFE_PRINT_LINE("[CuFFTBackend] Выделено GPU памяти: " << alloc_size << " байт");
    return true;
}
#endif

} // namespace HyperEngine::Rendering::FreqVox::Backends

