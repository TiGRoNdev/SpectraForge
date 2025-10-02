/**
 * @file FreqVoxBackendsTest.cpp
 * @brief Тесты для FFT/DCT бэкендов FreqVox
 */

#include "SpectraForge/Rendering/FreqVox/BackendFactory.h"
#include "SpectraForge/Rendering/FreqVox/Backends/SimpleDctBackend.h"
#include "gtest/gtest.h"
#include <vector>

using namespace SpectraForge::Rendering::FreqVox;

class FreqVoxBackendsTest : public ::testing::Test {
protected:
    DctBlockConfig config;

    void SetUp() override {
        config.blockSize = 8;
        config.batchCount = 2;
    }
};

// Тест фабрики бэкендов
TEST_F(FreqVoxBackendsTest, FactoryAutoCreate) {
    auto backend = BackendFactory::create(BackendFactory::BackendType::Auto);
    ASSERT_NE(backend, nullptr);
}

TEST_F(FreqVoxBackendsTest, FactorySimpleCreate) {
    auto backend = BackendFactory::create(BackendFactory::BackendType::Simple);
    ASSERT_NE(backend, nullptr);
}

TEST_F(FreqVoxBackendsTest, FactoryVkFFTCreate) {
    auto backend = BackendFactory::create(BackendFactory::BackendType::VkFFT);
    ASSERT_NE(backend, nullptr);
}

#ifdef HYPERENGINE_CUDA_AVAILABLE
TEST_F(FreqVoxBackendsTest, FactoryCuFFTCreate) {
    auto backend = BackendFactory::create(BackendFactory::BackendType::CuFFT);
    ASSERT_NE(backend, nullptr);
}
#endif

TEST_F(FreqVoxBackendsTest, FactoryAvailability) {
    EXPECT_TRUE(BackendFactory::isAvailable(BackendFactory::BackendType::Auto));
    EXPECT_TRUE(BackendFactory::isAvailable(BackendFactory::BackendType::Simple));
    EXPECT_TRUE(BackendFactory::isAvailable(BackendFactory::BackendType::VkFFT));

#ifdef HYPERENGINE_CUDA_AVAILABLE
    EXPECT_TRUE(BackendFactory::isAvailable(BackendFactory::BackendType::CuFFT));
#else
    EXPECT_FALSE(BackendFactory::isAvailable(BackendFactory::BackendType::CuFFT));
#endif
}

// Тест SimpleDctBackend
TEST_F(FreqVoxBackendsTest, SimpleDctBackendInitialize) {
    Backends::SimpleDctBackend backend;
    EXPECT_TRUE(backend.initialize(config));
}

TEST_F(FreqVoxBackendsTest, SimpleDctBackendTransformForward) {
    Backends::SimpleDctBackend backend;
    backend.initialize(config);

    size_t data_size = config.blockSize * config.blockSize * config.batchCount;
    std::vector<float> data(data_size, 1.0f);

    EXPECT_TRUE(backend.transform_forward(data));
    // SimpleDct - заглушка, данные не меняются
    EXPECT_FLOAT_EQ(data[0], 1.0f);
}

TEST_F(FreqVoxBackendsTest, SimpleDctBackendTransformInverse) {
    Backends::SimpleDctBackend backend;
    backend.initialize(config);

    size_t data_size = config.blockSize * config.blockSize * config.batchCount;
    std::vector<float> data(data_size, 2.0f);

    EXPECT_TRUE(backend.transform_inverse(data));
    EXPECT_FLOAT_EQ(data[0], 2.0f);
}

TEST_F(FreqVoxBackendsTest, SimpleDctBackendShutdown) {
    Backends::SimpleDctBackend backend;
    backend.initialize(config);
    backend.shutdown();
    // Не должно упасть
}

// Тест VkFFTBackend (заглушка)
TEST_F(FreqVoxBackendsTest, VkFFTBackendInitialize) {
    auto backend = BackendFactory::create(BackendFactory::BackendType::VkFFT);
    ASSERT_NE(backend, nullptr);
    EXPECT_TRUE(backend->initialize(config));
}

#ifdef HYPERENGINE_CUDA_AVAILABLE
// Тесты cuFFTBackend (если CUDA доступна)
TEST_F(FreqVoxBackendsTest, CuFFTBackendInitialize) {
    auto backend = BackendFactory::create(BackendFactory::BackendType::CuFFT);
    ASSERT_NE(backend, nullptr);
    EXPECT_TRUE(backend->initialize(config));
}

TEST_F(FreqVoxBackendsTest, CuFFTBackendTransform) {
    auto backend = BackendFactory::create(BackendFactory::BackendType::CuFFT);
    ASSERT_NE(backend, nullptr);
    ASSERT_TRUE(backend->initialize(config));

    size_t data_size = config.width * config.height * config.batch_size;
    std::vector<float> data(data_size);
    
    // Заполняем тестовыми данными
    for (size_t i = 0; i < data.size(); ++i) {
        data[i] = static_cast<float>(i % 10);
    }

    // Forward transform
    EXPECT_TRUE(backend->transform_forward(data));
    
    // Inverse transform
    EXPECT_TRUE(backend->transform_inverse(data));
    
    // После forward + inverse должны вернуться к исходным (с небольшой погрешностью)
    // Для реального теста нужно проверять с tolerance
}
#endif

// Тест жизненного цикла бэкенда
TEST_F(FreqVoxBackendsTest, BackendLifecycle) {
    auto backend = BackendFactory::create();
    ASSERT_NE(backend, nullptr);
    
    // Инициализация
    EXPECT_TRUE(backend->initialize(config));
    
    // Повторная инициализация должна работать
    EXPECT_TRUE(backend->initialize(config));
    
    // Shutdown
    backend->shutdown();
    
    // Повторный shutdown не должен крашиться
    backend->shutdown();
}

