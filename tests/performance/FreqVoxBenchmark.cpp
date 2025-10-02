/**
 * @file FreqVoxBenchmark.cpp
 * @brief Бенчмарки производительности FreqVox Renderer
 */

#include "SpectraForge/Rendering/FreqVox/BackendFactory.h"
#include "SpectraForge/Rendering/FreqVox/FoveatedSelector.h"
#include "SpectraForge/Rendering/FreqVox/TemporalReprojection.h"
#include "SpectraForge/Rendering/FreqVox/NeuralUpscaling.h"
#include "SpectraForge/Core/SafeConsole.h"
#include <chrono>
#include <vector>
#include <iostream>
#include <iomanip>

using namespace SpectraForge::Rendering::FreqVox;
using namespace SpectraForge::Math;
using namespace std::chrono;

struct BenchmarkResult {
    std::string name;
    double avg_time_ms;
    double min_time_ms;
    double max_time_ms;
    size_t iterations;
};

// Утилита для замера времени
template<typename Func>
BenchmarkResult benchmark(const std::string& name, Func func, size_t iterations = 100) {
    std::vector<double> times;
    times.reserve(iterations);

    // Прогрев
    for (size_t i = 0; i < 10; ++i) {
        func();
    }

    // Основные измерения
    for (size_t i = 0; i < iterations; ++i) {
        auto start = high_resolution_clock::now();
        func();
        auto end = high_resolution_clock::now();
        
        double time_ms = duration<double, std::milli>(end - start).count();
        times.push_back(time_ms);
    }

    // Статистика
    double sum = 0.0;
    double min_val = times[0];
    double max_val = times[0];
    
    for (double t : times) {
        sum += t;
        if (t < min_val) min_val = t;
        if (t > max_val) max_val = t;
    }

    return {
        name,
        sum / iterations,
        min_val,
        max_val,
        iterations
    };
}

void printResults(const std::vector<BenchmarkResult>& results) {
    std::cout << "\n=== FreqVox Performance Benchmarks ===\n\n";
    std::cout << std::left << std::setw(40) << "Test" 
              << std::right << std::setw(12) << "Avg (ms)" 
              << std::setw(12) << "Min (ms)" 
              << std::setw(12) << "Max (ms)" 
              << std::setw(10) << "Iters" << "\n";
    std::cout << std::string(86, '-') << "\n";

    for (const auto& r : results) {
        std::cout << std::left << std::setw(40) << r.name
                  << std::right << std::setw(12) << std::fixed << std::setprecision(3) << r.avg_time_ms
                  << std::setw(12) << r.min_time_ms
                  << std::setw(12) << r.max_time_ms
                  << std::setw(10) << r.iterations << "\n";
    }
    std::cout << std::string(86, '=') << "\n\n";
}

int main() {
    std::vector<BenchmarkResult> results;

    // Benchmark 1: Backend Factory Creation
    results.push_back(benchmark("Backend Factory - Auto Create", []() {
        auto backend = BackendFactory::create(BackendFactory::BackendType::Auto);
    }, 1000));

    results.push_back(benchmark("Backend Factory - Simple Create", []() {
        auto backend = BackendFactory::create(BackendFactory::BackendType::Simple);
    }, 1000));

    // Benchmark 2: Backend Initialize
    {
        auto backend = BackendFactory::create(BackendFactory::BackendType::Simple);
        DctBlockConfig config{8, 8, 4};
        
        results.push_back(benchmark("SimpleDCT - Initialize", [&]() {
            backend->initialize(config);
        }, 100));
    }

    // Benchmark 3: DCT Transform (8x8 blocks, 4 batches)
    {
        auto backend = BackendFactory::create(BackendFactory::BackendType::Simple);
        DctBlockConfig config{8, 8, 4};
        backend->initialize(config);
        
        std::vector<float> data(8 * 8 * 4, 1.0f);
        
        results.push_back(benchmark("SimpleDCT - Forward 8x8x4", [&]() {
            backend->transform_forward(data);
        }, 500));

        results.push_back(benchmark("SimpleDCT - Inverse 8x8x4", [&]() {
            backend->transform_inverse(data);
        }, 500));
    }

    // Benchmark 4: DCT Transform (16x16 blocks, 16 batches)
    {
        auto backend = BackendFactory::create(BackendFactory::BackendType::Simple);
        DctBlockConfig config{16, 16, 16};
        backend->initialize(config);
        
        std::vector<float> data(16 * 16 * 16, 1.0f);
        
        results.push_back(benchmark("SimpleDCT - Forward 16x16x16", [&]() {
            backend->transform_forward(data);
        }, 500));
    }

    // Benchmark 5: Foveated Selection
    {
        FoveatedSelector selector;
        std::vector<Voxel> voxels(10000);
        for (size_t i = 0; i < voxels.size(); ++i) {
            voxels[i].position = Vector3(
                static_cast<float>(i % 100),
                static_cast<float>((i / 100) % 100),
                static_cast<float>(i / 10000)
            );
        }

        std::vector<Voxel> selected;
        float v_eff = 0.0f;
        FoveatedParams params{};

        results.push_back(benchmark("Foveated Selection - 10K voxels", [&]() {
            selector.select(voxels, params, selected, v_eff);
        }, 100));
    }

    // Benchmark 6: Temporal Reprojection
    {
        TemporalReprojection temporal;
        TemporalReprojectionParams params;
        temporal.initialize(1920, 1080, params);

        std::vector<float> current_frame(1920 * 1080 * 3, 0.5f);
        std::vector<Vector3> motion_vectors(1920 * 1080, Vector3(1.0f, 0.5f, 0.0f));
        std::vector<float> depth(1920 * 1080, 10.0f);
        std::vector<float> output(1920 * 1080 * 3);

        results.push_back(benchmark("Temporal Reprojection - 1920x1080", [&]() {
            temporal.reproject(current_frame, motion_vectors, depth, output);
        }, 50));
    }

    // Benchmark 7: Neural Upscaling (Bilinear)
    {
        NeuralUpscaler upscaler;
        NeuralUpscalingParams params{2.0f};
        upscaler.initialize(UpscalerType::Bilinear, 960, 540, params);

        std::vector<float> input(960 * 540 * 3, 0.5f);
        std::vector<float> output;

        results.push_back(benchmark("Neural Upscale 2x - 960x540->1920x1080", [&]() {
            upscaler.upscale(input, output);
        }, 50));
    }

    // Benchmark 8: Full Pipeline Simulation (без GPU)
    {
        auto backend = BackendFactory::create(BackendFactory::BackendType::Simple);
        DctBlockConfig config{8, 8, 64};
        backend->initialize(config);

        FoveatedSelector selector;
        std::vector<Voxel> voxels(5000);
        std::vector<Voxel> selected;
        float v_eff = 0.0f;
        FoveatedParams fov_params{};

        TemporalReprojection temporal;
        TemporalReprojectionParams temp_params;
        temporal.initialize(1920, 1080, temp_params);

        NeuralUpscaler upscaler;
        NeuralUpscalingParams up_params{2.0f};
        upscaler.initialize(UpscalerType::Bilinear, 960, 540, up_params);

        std::vector<float> dct_data(8 * 8 * 64, 1.0f);
        std::vector<float> frame(960 * 540 * 3, 0.5f);
        std::vector<Vector3> mvs(1920 * 1080);
        std::vector<float> depth(1920 * 1080, 10.0f);
        std::vector<float> temp_out(1920 * 1080 * 3);
        std::vector<float> final_out;

        results.push_back(benchmark("Full Pipeline (no GPU)", [&]() {
            // 1. Foveated selection
            selector.select(voxels, fov_params, selected, v_eff);
            
            // 2. DCT forward
            backend->transform_forward(dct_data);
            
            // 3. DCT inverse
            backend->transform_inverse(dct_data);
            
            // 4. Temporal reprojection
            temporal.reproject(frame, mvs, depth, temp_out);
            
            // 5. Neural upscaling
            upscaler.upscale(frame, final_out);
        }, 20));
    }

    // Вывод результатов
    printResults(results);

    // Анализ производительности
    std::cout << "Performance Analysis:\n";
    std::cout << "---------------------\n";
    
    double total_frame_time = 0.0;
    for (const auto& r : results) {
        if (r.name.find("Full Pipeline") != std::string::npos) {
            total_frame_time = r.avg_time_ms;
            double fps = 1000.0 / r.avg_time_ms;
            std::cout << "Estimated FPS (CPU only): " << std::fixed << std::setprecision(1) 
                      << fps << " fps (" << r.avg_time_ms << " ms/frame)\n";
        }
    }

    std::cout << "\nNote: These are CPU-only benchmarks. GPU acceleration (CUDA/Vulkan)\n";
    std::cout << "      is expected to provide 10-100x speedup for DCT and upscaling.\n";

    return 0;
}

