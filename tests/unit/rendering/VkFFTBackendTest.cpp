/**
 * @file VkFFTBackendTest.cpp
 * @brief Unit тесты для VkFFT DCT-II backend
 * 
 * Проверяет корректность DCT-II трансформации согласно FreqVox Renderer Math.md
 */

#include <gtest/gtest.h>
#include "SpectraForge/Rendering/FreqVox/Backends/VkFFTBackend.h"
#include "SpectraForge/Rendering/FreqVox/FreqVoxTypes.h"
#include <vector>
#include <cmath>
#include <iostream>

using namespace SpectraForge::Rendering::FreqVox;
using namespace SpectraForge::Rendering::FreqVox::Backends;

/**
 * @brief Тест fixture для VkFFTBackend
 */
class VkFFTBackendTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Проверяем доступность VkFFT
        if (!VkFFTBackend::isAvailable()) {
            GTEST_SKIP() << "VkFFT недоступен (требуется HYPERENGINE_USE_VKFFT)";
        }
    }

    /**
     * @brief Вычисление CPU DCT-II для эталона
     * @param input Входные данные [blockSize x blockSize]
     * @param output Выходные данные [blockSize x blockSize]
     * @param blockSize Размер блока
     * 
     * Реализация согласно Math.md строки 30-32:
     * M̃[u,v] = Σ M[p,q] * cos(πu(2p+1)/2P) * cos(πv(2q+1)/2Q)
     */
    void compute_reference_dct2(const std::vector<float>& input,
                                std::vector<float>& output,
                                uint32_t blockSize) {
        const float pi = 3.14159265358979323846f;
        
        for (uint32_t u = 0; u < blockSize; ++u) {
            for (uint32_t v = 0; v < blockSize; ++v) {
                float sum = 0.0f;
                
                for (uint32_t p = 0; p < blockSize; ++p) {
                    for (uint32_t q = 0; q < blockSize; ++q) {
                        float val = input[p * blockSize + q];
                        float cos_u = std::cos(pi * u * (2.0f * p + 1.0f) / (2.0f * blockSize));
                        float cos_v = std::cos(pi * v * (2.0f * q + 1.0f) / (2.0f * blockSize));
                        sum += val * cos_u * cos_v;
                    }
                }
                
                output[u * blockSize + v] = sum;
            }
        }
    }

    /**
     * @brief Проверка близости двух массивов
     */
    bool arrays_close(const std::vector<float>& a, 
                      const std::vector<float>& b, 
                      float tolerance = 0.01f) {
        if (a.size() != b.size()) return false;
        
        for (size_t i = 0; i < a.size(); ++i) {
            float diff = std::abs(a[i] - b[i]);
            float rel_diff = diff / (std::abs(a[i]) + 1e-6f);
            
            if (rel_diff > tolerance) {
                std::cerr << "Различие в индексе " << i 
                          << ": a=" << a[i] 
                          << " b=" << b[i] 
                          << " rel_diff=" << rel_diff << std::endl;
                return false;
            }
        }
        
        return true;
    }
};

/**
 * @brief Базовый тест инициализации без Vulkan контекста
 * 
 * Проверяет, что backend корректно обрабатывает отсутствие Vulkan устройства
 */
TEST_F(VkFFTBackendTest, InitializationWithoutVulkanContext) {
    VkFFTBackend backend;
    
    DctBlockConfig config;
    config.blockSize = 8;
    config.batchCount = 1;
    
    // Без Vulkan контекста инициализация должна вернуть false
    bool result = backend.initialize(config);
    
    // В реальной среде с Vulkan это может быть true или false
    // В зависимости от доступности Vulkan runtime
    if (!result) {
        std::cout << "VkFFT: Инициализация без Vulkan контекста не удалась (ожидаемо)" << std::endl;
    }
}

/**
 * @brief Тест проверки доступности VkFFT
 */
TEST_F(VkFFTBackendTest, AvailabilityCheck) {
    bool available = VkFFTBackend::isAvailable();
    
#ifdef HYPERENGINE_USE_VKFFT
    EXPECT_TRUE(available) << "VkFFT должен быть доступен с HYPERENGINE_USE_VKFFT";
#else
    EXPECT_FALSE(available) << "VkFFT не должен быть доступен без HYPERENGINE_USE_VKFFT";
#endif
}

/**
 * @brief Тест конфигурации блока
 */
TEST_F(VkFFTBackendTest, BlockConfigValidation) {
    VkFFTBackend backend;
    
    // Некорректная конфигурация (blockSize=0)
    DctBlockConfig invalid_config;
    invalid_config.blockSize = 0;
    invalid_config.batchCount = 1;
    
    bool result = backend.initialize(invalid_config);
    EXPECT_FALSE(result) << "Инициализация с blockSize=0 должна провалиться";
}

/**
 * @brief Простой тест forward/inverse симметрии (без полной проверки DCT)
 * 
 * Проверяет, что forward -> inverse возвращает исходные данные
 * (с учетом нормализации)
 */
TEST_F(VkFFTBackendTest, DISABLED_ForwardInverseSymmetry) {
    // DISABLED пока нет Vulkan контекста в тестах
    VkFFTBackend backend;
    
    DctBlockConfig config;
    config.blockSize = 8;
    config.batchCount = 1;
    
    ASSERT_TRUE(backend.initialize(config)) << "Инициализация не удалась";
    
    // Создаем тестовые данные
    std::vector<float> data(config.blockSize * config.blockSize);
    for (size_t i = 0; i < data.size(); ++i) {
        data[i] = static_cast<float>(i);
    }
    
    std::vector<float> original = data;
    
    // Forward DCT
    ASSERT_TRUE(backend.transform_forward(data)) << "Forward transform не удался";
    
    // Inverse DCT
    ASSERT_TRUE(backend.transform_inverse(data)) << "Inverse transform не удался";
    
    // Проверяем восстановление (с учетом возможной нормализации)
    // VkFFT может требовать ручной нормализации
    float norm_factor = 1.0f / (config.blockSize * config.blockSize);
    for (auto& val : data) {
        val *= norm_factor;
    }
    
    EXPECT_TRUE(arrays_close(data, original, 0.1f)) 
        << "Данные не восстановились после forward->inverse";
}

/**
 * @brief Тест DCT-II константного блока
 * 
 * DCT константного блока должна давать энергию только в DC компоненте [0,0]
 */
TEST_F(VkFFTBackendTest, DISABLED_ConstantBlockDCT) {
    // DISABLED пока нет Vulkan контекста
    VkFFTBackend backend;
    
    DctBlockConfig config;
    config.blockSize = 8;
    config.batchCount = 1;
    
    ASSERT_TRUE(backend.initialize(config)) << "Инициализация не удалась";
    
    // Константный блок
    std::vector<float> data(config.blockSize * config.blockSize, 1.0f);
    
    ASSERT_TRUE(backend.transform_forward(data)) << "Forward transform не удался";
    
    // DC компонента (0,0) должна быть значительно больше остальных
    float dc_component = std::abs(data[0]);
    float max_ac_component = 0.0f;
    
    for (size_t i = 1; i < data.size(); ++i) {
        max_ac_component = std::max(max_ac_component, std::abs(data[i]));
    }
    
    EXPECT_GT(dc_component, max_ac_component * 10.0f)
        << "DC компонента должна доминировать для константного блока";
}

/**
 * @brief Интеграционный тест сравнения с эталонной CPU DCT-II
 * 
 * Сравнивает результат VkFFT с reference реализацией согласно Math.md
 */
TEST_F(VkFFTBackendTest, DISABLED_CompareWithReferenceDCT) {
    // DISABLED - требует Vulkan контекст и долгая проверка
    VkFFTBackend backend;
    
    DctBlockConfig config;
    config.blockSize = 8;
    config.batchCount = 1;
    
    ASSERT_TRUE(backend.initialize(config)) << "Инициализация не удалась";
    
    // Тестовый блок с градиентом
    std::vector<float> input(config.blockSize * config.blockSize);
    for (uint32_t i = 0; i < config.blockSize; ++i) {
        for (uint32_t j = 0; j < config.blockSize; ++j) {
            input[i * config.blockSize + j] = static_cast<float>(i + j);
        }
    }
    
    // Вычисляем эталонную DCT-II на CPU
    std::vector<float> reference_output(config.blockSize * config.blockSize);
    compute_reference_dct2(input, reference_output, config.blockSize);
    
    // Вычисляем VkFFT DCT-II на GPU
    std::vector<float> vkfft_output = input;
    ASSERT_TRUE(backend.transform_forward(vkfft_output)) << "VkFFT forward не удался";
    
    // Сравниваем результаты
    EXPECT_TRUE(arrays_close(vkfft_output, reference_output, 0.05f))
        << "VkFFT DCT-II не совпадает с эталонной реализацией Math.md";
}

/**
 * @brief Тест производительности батчированной обработки
 */
TEST_F(VkFFTBackendTest, DISABLED_BatchProcessingPerformance) {
    // DISABLED - требует Vulkan и измерение времени
    VkFFTBackend backend;
    
    DctBlockConfig config;
    config.blockSize = 8;
    config.batchCount = 100;  // Батч из 100 блоков 8x8
    
    ASSERT_TRUE(backend.initialize(config)) << "Инициализация не удалась";
    
    size_t total_size = config.batchCount * config.blockSize * config.blockSize;
    std::vector<float> data(total_size, 1.0f);
    
    // Измеряем время forward transform
    auto start = std::chrono::high_resolution_clock::now();
    ASSERT_TRUE(backend.transform_forward(data));
    auto end = std::chrono::high_resolution_clock::now();
    
    auto duration_ms = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
    
    std::cout << "VkFFT batch DCT (" << config.batchCount << " блоков 8x8): " 
              << duration_ms << " ms" << std::endl;
    
    // Ожидаем, что батчированная обработка будет быстрой (< 10ms на GPU)
    EXPECT_LT(duration_ms, 100) << "Батчированная DCT слишком медленная";
}

/**
 * @brief Основная функция для запуска тестов
 */
int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    
    std::cout << "╔════════════════════════════════════════════╗" << std::endl;
    std::cout << "║   VkFFT DCT-II Backend Unit Tests         ║" << std::endl;
    std::cout << "║   FreqVox Renderer Math.md Compliance     ║" << std::endl;
    std::cout << "╚════════════════════════════════════════════╝" << std::endl;
    
#ifdef HYPERENGINE_USE_VKFFT
    std::cout << "✅ VkFFT поддержка включена" << std::endl;
#else
    std::cout << "⚠️  VkFFT поддержка НЕ включена (некоторые тесты будут пропущены)" << std::endl;
#endif
    
    return RUN_ALL_TESTS();
}

