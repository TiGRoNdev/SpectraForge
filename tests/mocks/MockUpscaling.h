#pragma once
#include <gmock/gmock.h>
#include "HyperEngine/Upscaling/Upscaler.h"

namespace HyperEngine::Testing::Mocks {

using namespace HyperEngine::Upscaling;

/**
 * @brief Mock объект для базового класса Upscaler
 *
 * Используется для тестирования различных стратегий upscaling
 * без необходимости инициализации реального DLSS/FSR
 */
class MockUpscaler : public Upscaler {
  public:
    // Основные методы жизненного цикла
    MOCK_METHOD(bool, init, (const HardwareConfig& config), (override));
    MOCK_METHOD(void, shutdown, (), (override));
    MOCK_METHOD(bool, isInitialized, (), (const override));

    // Upscaling методы
    MOCK_METHOD(FinalImage,
                upscaleImage,
                (const DenoisedImage& image, const ResolutionTarget& target),
                (override));

    // Информационные методы
    MOCK_METHOD(std::string, getUpscalerName, (), (const override));
    MOCK_METHOD(bool, isSupported, (), (const override));
    MOCK_METHOD(float, getPerformanceGain, (), (const override));

    // Настройки качества
    MOCK_METHOD(void, setQualityMode, (QualityMode mode), (override));
    MOCK_METHOD(QualityMode, getQualityMode, (), (const override));

    // Дополнительные features
    MOCK_METHOD(bool, supportsRayReconstruction, (), (const override));
    MOCK_METHOD(bool, supportsFrameGeneration, (), (const override));
    MOCK_METHOD(bool, supportsMotionVectors, (), (const override));
};

/**
 * @brief Mock объект для DLSS upscaler
 */
class MockDLSSUpscaler : public MockUpscaler {
  public:
    // DLSS специфичные методы
    MOCK_METHOD(bool, initStreamline, (), ());
    MOCK_METHOD(void, enableSuperResolution, (bool enable), ());
    MOCK_METHOD(void, enableRayReconstruction, (bool enable), ());
    MOCK_METHOD(void, enableFrameGeneration, (bool enable), ());

    // Получение информации о DLSS
    MOCK_METHOD(std::string, getDLSSVersion, (), (const));
    MOCK_METHOD(bool, supportsTensorCores, (), (const));

    // Настройки производительности
    MOCK_METHOD(void, setSharpening, (float value), ());
    MOCK_METHOD(void, setPreset, (const std::string& preset), ());

    std::string getUpscalerName() const override { return "DLSS"; }
};

/**
 * @brief Mock объект для FSR upscaler
 */
class MockFSRUpscaler : public MockUpscaler {
  public:
    // FSR специфичные методы
    MOCK_METHOD(bool, initFidelityFX, (), ());
    MOCK_METHOD(void, enableTemporalUpscaling, (bool enable), ());
    MOCK_METHOD(void, enableFrameInterpolation, (bool enable), ());
    MOCK_METHOD(void, enableNativeAA, (bool enable), ());

    // Получение информации о FSR
    MOCK_METHOD(std::string, getFSRVersion, (), (const));
    MOCK_METHOD(bool, supportsAsyncCompute, (), (const));

    // Настройки качества FSR
    MOCK_METHOD(void, setSharpness, (float value), ());
    MOCK_METHOD(void, setMipBias, (float value), ());

    std::string getUpscalerName() const override { return "FSR"; }
};

/**
 * @brief Mock объект для HardwareDetector
 */
class MockHardwareDetector {
  public:
    MOCK_METHOD(HardwareConfig::VendorType, detectVendor, (), ());
    MOCK_METHOD(bool, supportsRayTracing, (), ());
    MOCK_METHOD(bool, supportsTensorCores, (), ());
    MOCK_METHOD(bool, supportsAsyncCompute, (), ());
    MOCK_METHOD(size_t, getVRAMSize, (), ());
    MOCK_METHOD(std::string, getDeviceName, (), ());

    // Рекомендации по upscaler
    MOCK_METHOD(std::string, selectOptimalUpscaler, (), ());
    MOCK_METHOD(QualityMode,
                recommendQualityMode,
                (uint32_t targetWidth, uint32_t targetHeight),
                ());
};

/**
 * @brief Фабрика для создания mock upscaler объектов
 */
class UpscalingMockFactory {
  public:
    /**
     * @brief Создает mock DLSS upscaler с NVIDIA конфигурацией
     */
    static std::unique_ptr<MockDLSSUpscaler> createDLSSUpscaler() {
        auto mock = std::make_unique<MockDLSSUpscaler>();

        // Настройка DLSS конфигурации
        HardwareConfig nvidiaConfig;
        nvidiaConfig.vendor = HardwareConfig::VendorType::NVIDIA;
        nvidiaConfig.supportsRayTracing = true;
        nvidiaConfig.supportsTensorCores = true;
        nvidiaConfig.vramSize = 8 * 1024 * 1024 * 1024;  // 8GB
        nvidiaConfig.deviceName = "RTX 4080";

        EXPECT_CALL(*mock, init(testing::_)).WillRepeatedly(testing::Return(true));
        EXPECT_CALL(*mock, isInitialized()).WillRepeatedly(testing::Return(true));
        EXPECT_CALL(*mock, isSupported()).WillRepeatedly(testing::Return(true));
        EXPECT_CALL(*mock, getPerformanceGain())
            .WillRepeatedly(testing::Return(2.5f));  // 2.5x performance gain

        // DLSS специфичные ожидания
        EXPECT_CALL(*mock, supportsTensorCores()).WillRepeatedly(testing::Return(true));
        EXPECT_CALL(*mock, getDLSSVersion()).WillRepeatedly(testing::Return("3.1.1"));
        EXPECT_CALL(*mock, supportsRayReconstruction()).WillRepeatedly(testing::Return(true));
        EXPECT_CALL(*mock, supportsFrameGeneration()).WillRepeatedly(testing::Return(true));

        // Настройка upscaling результата
        setupUpscalingResult(*mock);

        return mock;
    }

    /**
     * @brief Создает mock FSR upscaler с AMD конфигурацией
     */
    static std::unique_ptr<MockFSRUpscaler> createFSRUpscaler() {
        auto mock = std::make_unique<MockFSRUpscaler>();

        // Настройка FSR конфигурации
        HardwareConfig amdConfig;
        amdConfig.vendor = HardwareConfig::VendorType::AMD;
        amdConfig.supportsRayTracing = true;
        amdConfig.supportsTensorCores = false;
        amdConfig.supportsAsyncCompute = true;
        amdConfig.vramSize = 16 * 1024 * 1024 * 1024;  // 16GB
        amdConfig.deviceName = "RX 7800 XT";

        EXPECT_CALL(*mock, init(testing::_)).WillRepeatedly(testing::Return(true));
        EXPECT_CALL(*mock, isInitialized()).WillRepeatedly(testing::Return(true));
        EXPECT_CALL(*mock, isSupported()).WillRepeatedly(testing::Return(true));
        EXPECT_CALL(*mock, getPerformanceGain())
            .WillRepeatedly(testing::Return(1.8f));  // 1.8x performance gain

        // FSR специфичные ожидания
        EXPECT_CALL(*mock, supportsAsyncCompute()).WillRepeatedly(testing::Return(true));
        EXPECT_CALL(*mock, getFSRVersion()).WillRepeatedly(testing::Return("3.0"));
        EXPECT_CALL(*mock, supportsRayReconstruction()).WillRepeatedly(testing::Return(false));
        EXPECT_CALL(*mock, supportsFrameGeneration()).WillRepeatedly(testing::Return(true));

        // Настройка upscaling результата
        setupUpscalingResult(*mock);

        return mock;
    }

    /**
     * @brief Создает mock HardwareDetector
     */
    static std::unique_ptr<MockHardwareDetector> createHardwareDetector(
        HardwareConfig::VendorType vendor = HardwareConfig::VendorType::NVIDIA) {
        auto mock = std::make_unique<MockHardwareDetector>();

        EXPECT_CALL(*mock, detectVendor()).WillRepeatedly(testing::Return(vendor));

        if (vendor == HardwareConfig::VendorType::NVIDIA) {
            EXPECT_CALL(*mock, supportsTensorCores()).WillRepeatedly(testing::Return(true));
            EXPECT_CALL(*mock, selectOptimalUpscaler()).WillRepeatedly(testing::Return("DLSS"));
            EXPECT_CALL(*mock, getDeviceName()).WillRepeatedly(testing::Return("RTX 4080"));
        } else if (vendor == HardwareConfig::VendorType::AMD) {
            EXPECT_CALL(*mock, supportsTensorCores()).WillRepeatedly(testing::Return(false));
            EXPECT_CALL(*mock, supportsAsyncCompute()).WillRepeatedly(testing::Return(true));
            EXPECT_CALL(*mock, selectOptimalUpscaler()).WillRepeatedly(testing::Return("FSR"));
            EXPECT_CALL(*mock, getDeviceName()).WillRepeatedly(testing::Return("RX 7800 XT"));
        }

        EXPECT_CALL(*mock, supportsRayTracing()).WillRepeatedly(testing::Return(true));
        EXPECT_CALL(*mock, getVRAMSize())
            .WillRepeatedly(testing::Return(8 * 1024 * 1024 * 1024));  // 8GB

        return mock;
    }

    /**
     * @brief Создает неподдерживаемый upscaler (для тестирования fallback)
     */
    static std::unique_ptr<MockUpscaler> createUnsupportedUpscaler() {
        auto mock = std::make_unique<MockUpscaler>();

        EXPECT_CALL(*mock, init(testing::_)).WillRepeatedly(testing::Return(false));
        EXPECT_CALL(*mock, isInitialized()).WillRepeatedly(testing::Return(false));
        EXPECT_CALL(*mock, isSupported()).WillRepeatedly(testing::Return(false));
        EXPECT_CALL(*mock, getUpscalerName()).WillRepeatedly(testing::Return("Unsupported"));

        return mock;
    }

  private:
    /**
     * @brief Настройка результата upscaling для mock объекта
     */
    template <typename MockType>
    static void setupUpscalingResult(MockType& mock) {
        EXPECT_CALL(mock, upscaleImage(testing::_, testing::_))
            .WillRepeatedly(
                testing::Invoke([](const DenoisedImage& input, const ResolutionTarget& target) {
                    FinalImage result;
                    result.width = target.width;
                    result.height = target.height;
                    result.channels = input.channels;

                    // Симуляция upscaled буфера
                    size_t bufferSize = target.width * target.height * input.channels;
                    result.colorBuffer = new float[bufferSize];
                    std::fill_n(
                        result.colorBuffer, bufferSize, 0.5f);  // Заполняем серым цветом для тестов

                    return result;
                }));
    }
};

}  // namespace HyperEngine::Testing::Mocks

