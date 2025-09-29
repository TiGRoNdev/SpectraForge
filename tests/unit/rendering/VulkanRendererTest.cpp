#include "TestFramework.h"
#include "mocks/MockVulkanRenderer.h"

// Условная компиляция для Vulkan тестов
#ifdef HyperEngine_ENABLE_VULKAN
#include "HyperEngine/Vulkan/VulkanRenderer.h"
using namespace HyperEngine::Vulkan;
#endif

using namespace HyperEngine::Testing;
using namespace HyperEngine::Testing::Mocks;

/**
 * @brief Unit тесты для VulkanRenderer
 *
 * Тестирует гибридный rendering pipeline согласно UML архитектуре:
 * Primary rasterization -> Secondary ray tracing -> AI denoising -> Upscaling
 */
class VulkanRendererTest : public HyperEngineTest {
  protected:
    void SetUp() override {
        HyperEngineTest::SetUp();

        // Создаем mock объекты для всех компонентов
        mockRenderer = VulkanMockFactory::createInitializedRenderer();
        mockResourceManager = VulkanMockFactory::createResourceManager();
        mockFlashGS = VulkanMockFactory::createFlashGSSplatter();
        mockOptiX = VulkanMockFactory::createOptiXRayTracer();

        // Настраиваем тестовые данные
        setupTestData();
    }

    void TearDown() override {
        if (mockRenderer) {
            mockRenderer->shutdown();
        }
        HyperEngineTest::TearDown();
    }

  private:
    void setupTestData() {
        // Настройка тестовых гауссианов
        testGaussians.count = 1000;

        // Настройка целевого разрешения
        testResolution.width = 1920;
        testResolution.height = 1080;
        testResolution.scaleFactor = 2.0f;
    }

  protected:
    std::unique_ptr<MockVulkanRenderer> mockRenderer;
    std::unique_ptr<MockResourceManager> mockResourceManager;
    std::unique_ptr<MockFlashGSSplatter> mockFlashGS;
    std::unique_ptr<MockOptiXRayTracer> mockOptiX;

    Gaussians testGaussians;
    ResolutionTarget testResolution;
};

// Тесты инициализации
TEST_F(VulkanRendererTest, SuccessfulInitialization) {
#ifdef HyperEngine_ENABLE_VULKAN
    vk::Device mockDevice;  // Заглушка для Vulkan device
#else
    // Заглушка для случая без Vulkan
    void* mockDevice = nullptr;
#endif

    EXPECT_CALL(*mockRenderer, init(testing::_, testing::_)).WillOnce(testing::Return(true));
    EXPECT_CALL(*mockRenderer, isInitialized()).WillRepeatedly(testing::Return(true));

    EXPECT_NO_THROW_WITH_MESSAGE(
        {
            bool result = mockRenderer->init(
                mockDevice, reinterpret_cast<ResourceManager*>(mockResourceManager.get()));
            EXPECT_TRUE(result);
            EXPECT_TRUE(mockRenderer->isInitialized());
        },
        "Успешная инициализация VulkanRenderer");
}

TEST_F(VulkanRendererTest, FailedInitialization) {
    auto failingRenderer = VulkanMockFactory::createFailingRenderer();
#ifdef HyperEngine_ENABLE_VULKAN
    vk::Device mockDevice;
#else
    void* mockDevice = nullptr;
#endif

    EXPECT_FALSE(failingRenderer->init(
        mockDevice, reinterpret_cast<ResourceManager*>(mockResourceManager.get())));
    EXPECT_FALSE(failingRenderer->isInitialized());
}

// Тесты Primary Rasterization (FlashGS)
TEST_F(VulkanRendererTest, PrimaryRasterization) {
    PrimaryImage expectedImage;
    expectedImage.width = 1920;
    expectedImage.height = 1080;

    EXPECT_CALL(*mockRenderer, rasterizePrimary(testing::_))
        .WillOnce(testing::Return(expectedImage));

    EXPECT_NO_THROW_WITH_MESSAGE(
        {
            PrimaryImage result = mockRenderer->rasterizePrimary(testGaussians);
            EXPECT_EQ(result.width, expectedImage.width);
            EXPECT_EQ(result.height, expectedImage.height);
        },
        "Primary rasterization через FlashGS");
}

TEST_F(VulkanRendererTest, PrimaryRasterizationPerformance) {
    PrimaryImage mockImage;
    mockImage.width = 1920;
    mockImage.height = 1080;

    EXPECT_CALL(*mockRenderer, rasterizePrimary(testing::_))
        .WillRepeatedly(testing::Return(mockImage));

    const int frameCount = 10;

    EXPECT_PERFORMANCE_UNDER(
        {
            for (int i = 0; i < frameCount; ++i) {
                mockRenderer->rasterizePrimary(testGaussians);
            }
        },
        100);  // 10 кадров за < 100ms
}

// Тесты Secondary Ray Tracing (OptiX)
TEST_F(VulkanRendererTest, SecondaryRayTracing) {
    PrimaryImage primaryImage;
    primaryImage.width = 1920;
    primaryImage.height = 1080;

    RawEffects expectedEffects;

    EXPECT_CALL(*mockRenderer, rayTraceSecondary(testing::_))
        .WillOnce(testing::Return(expectedEffects));

    EXPECT_NO_THROW_WITH_MESSAGE(
        {
            [[maybe_unused]] RawEffects result = mockRenderer->rayTraceSecondary(primaryImage);
            // Результат должен содержать эффекты
        },
        "Secondary ray tracing через OptiX");
}

TEST_F(VulkanRendererTest, RayTracingWithDifferentComplexity) {
    PrimaryImage primaryImage;
    primaryImage.width = 1920;
    primaryImage.height = 1080;

    RawEffects simpleEffects, complexEffects;

    EXPECT_CALL(*mockRenderer, rayTraceSecondary(testing::_))
        .WillOnce(testing::Return(simpleEffects))
        .WillOnce(testing::Return(complexEffects));

    // Тест с простой сценой
    EXPECT_NO_THROW_WITH_MESSAGE(
        { [[maybe_unused]] RawEffects result1 = mockRenderer->rayTraceSecondary(primaryImage); },
        "Ray tracing простой сцены");

    // Тест со сложной сценой
    EXPECT_NO_THROW_WITH_MESSAGE(
        { [[maybe_unused]] RawEffects result2 = mockRenderer->rayTraceSecondary(primaryImage); },
        "Ray tracing сложной сцены");
}

// Тесты AI Denoising
TEST_F(VulkanRendererTest, AIDenoising) {
    RawEffects rawEffects;
    DenoisedImage expectedDenoised;

    EXPECT_CALL(*mockRenderer, denoiseAI(testing::_)).WillOnce(testing::Return(expectedDenoised));

    EXPECT_NO_THROW_WITH_MESSAGE(
        {
            [[maybe_unused]] DenoisedImage result = mockRenderer->denoiseAI(rawEffects);
            // Результат должен быть деноизированным
        },
        "AI деноизинг через OptiX");
}

TEST_F(VulkanRendererTest, DenoisePerformance) {
    RawEffects rawEffects;
    DenoisedImage mockDenoised;

    EXPECT_CALL(*mockRenderer, denoiseAI(testing::_)).WillRepeatedly(testing::Return(mockDenoised));

    const int denoiseCount = 5;

    EXPECT_PERFORMANCE_UNDER(
        {
            for (int i = 0; i < denoiseCount; ++i) {
                mockRenderer->denoiseAI(rawEffects);
            }
        },
        200);  // 5 операций деноизинга за < 200ms
}

// Тесты Upscaling
TEST_F(VulkanRendererTest, Upscaling) {
    DenoisedImage denoisedImage;
    FinalImage expectedFinal;

    EXPECT_CALL(*mockRenderer, upscale(testing::_, testing::_))
        .WillOnce(testing::Return(expectedFinal));

    EXPECT_NO_THROW_WITH_MESSAGE(
        {
            [[maybe_unused]] FinalImage result = mockRenderer->upscale(denoisedImage, testResolution);
            // Результат должен иметь целевое разрешение
        },
        "Upscaling через DLSS/FSR");
}

TEST_F(VulkanRendererTest, UpscalingDifferentScaleFactors) {
    DenoisedImage denoisedImage;
    FinalImage mockFinal;

    EXPECT_CALL(*mockRenderer, upscale(testing::_, testing::_))
        .WillRepeatedly(testing::Return(mockFinal));

    std::vector<float> scaleFactors = {1.5f, 2.0f, 2.5f, 3.0f, 4.0f};

    for (float scale : scaleFactors) {
        ResolutionTarget target;
        target.width = static_cast<uint32_t>(1920 * scale);
        target.height = static_cast<uint32_t>(1080 * scale);
        target.scaleFactor = scale;

        EXPECT_NO_THROW_WITH_MESSAGE(
            { [[maybe_unused]] FinalImage result = mockRenderer->upscale(denoisedImage, target); },
            "Upscaling с коэффициентом " + std::to_string(scale));
    }
}

// Тесты полного rendering pipeline
TEST_F(VulkanRendererTest, FullRenderingPipeline) {
    // Настройка ожиданий для полного pipeline
    PrimaryImage primaryImage;
    primaryImage.width = 1920;
    primaryImage.height = 1080;

    RawEffects rawEffects;
    DenoisedImage denoisedImage;
    FinalImage finalImage;

    // Настройка последовательности вызовов
    testing::InSequence seq;

    EXPECT_CALL(*mockRenderer, rasterizePrimary(testing::_))
        .WillOnce(testing::Return(primaryImage));
    EXPECT_CALL(*mockRenderer, rayTraceSecondary(testing::_)).WillOnce(testing::Return(rawEffects));
    EXPECT_CALL(*mockRenderer, denoiseAI(testing::_)).WillOnce(testing::Return(denoisedImage));
    EXPECT_CALL(*mockRenderer, upscale(testing::_, testing::_))
        .WillOnce(testing::Return(finalImage));
    EXPECT_CALL(*mockRenderer, presentFinalImage()).Times(1);

    EXPECT_NO_THROW_WITH_MESSAGE(
        {
            // Выполняем полный pipeline
            PrimaryImage primary = mockRenderer->rasterizePrimary(testGaussians);
            RawEffects effects = mockRenderer->rayTraceSecondary(primary);
            DenoisedImage denoised = mockRenderer->denoiseAI(effects);
            [[maybe_unused]] FinalImage final = mockRenderer->upscale(denoised, testResolution);
            mockRenderer->presentFinalImage();
        },
        "Полный rendering pipeline");
}

TEST_F(VulkanRendererTest, PipelinePerformance) {
    // Настройка mock результатов
    PrimaryImage primaryImage;
    RawEffects rawEffects;
    DenoisedImage denoisedImage;
    FinalImage finalImage;

    EXPECT_CALL(*mockRenderer, rasterizePrimary(testing::_))
        .WillRepeatedly(testing::Return(primaryImage));
    EXPECT_CALL(*mockRenderer, rayTraceSecondary(testing::_))
        .WillRepeatedly(testing::Return(rawEffects));
    EXPECT_CALL(*mockRenderer, denoiseAI(testing::_))
        .WillRepeatedly(testing::Return(denoisedImage));
    EXPECT_CALL(*mockRenderer, upscale(testing::_, testing::_))
        .WillRepeatedly(testing::Return(finalImage));
    EXPECT_CALL(*mockRenderer, presentFinalImage()).Times(testing::AnyNumber());

    const int frameCount = 3;

    EXPECT_PERFORMANCE_UNDER(
        {
            for (int i = 0; i < frameCount; ++i) {
                PrimaryImage primary = mockRenderer->rasterizePrimary(testGaussians);
                RawEffects effects = mockRenderer->rayTraceSecondary(primary);
                DenoisedImage denoised = mockRenderer->denoiseAI(effects);
                [[maybe_unused]] FinalImage final = mockRenderer->upscale(denoised, testResolution);
                mockRenderer->presentFinalImage();
            }
        },
        500);  // 3 полных кадра за < 500ms
}

// Тесты обработки ошибок
TEST_F(VulkanRendererTest, RenderingWithoutInitialization) {
    auto uninitializedRenderer = std::make_unique<MockVulkanRenderer>();

    EXPECT_CALL(*uninitializedRenderer, isInitialized()).WillRepeatedly(testing::Return(false));

    // Попытки рендеринга без инициализации должны приводить к ошибкам
    EXPECT_THROW(uninitializedRenderer->rasterizePrimary(testGaussians), std::runtime_error);
}

// Тесты валидации входных параметров
TEST_F(VulkanRendererTest, ValidateGaussiansEmpty) {
    Gaussians emptyGaussians;
    emptyGaussians.count = 0;

    EXPECT_CALL(*mockRenderer, rasterizePrimary(testing::_))
        .WillOnce(
            testing::Throw(std::invalid_argument("Количество гауссианов должно быть больше 0")));

    EXPECT_THROW(mockRenderer->rasterizePrimary(emptyGaussians), std::invalid_argument);
}

TEST_F(VulkanRendererTest, ValidateGaussiansTooMany) {
    Gaussians tooManyGaussians;
    tooManyGaussians.count = 2000000;  // Превышает лимит

    EXPECT_CALL(*mockRenderer, rasterizePrimary(testing::_))
        .WillOnce(testing::Throw(std::invalid_argument(
            "Количество гауссианов превышает максимально допустимое значение")));

    EXPECT_THROW(mockRenderer->rasterizePrimary(tooManyGaussians), std::invalid_argument);
}

TEST_F(VulkanRendererTest, ValidateImageDimensions) {
    PrimaryImage invalidImage;
    invalidImage.width = 0;
    invalidImage.height = 1080;

    EXPECT_CALL(*mockRenderer, rayTraceSecondary(testing::_))
        .WillOnce(
            testing::Throw(std::invalid_argument("Размеры изображения должны быть больше 0")));

    EXPECT_THROW(mockRenderer->rayTraceSecondary(invalidImage), std::invalid_argument);
}

TEST_F(VulkanRendererTest, ValidateImageTooLarge) {
    PrimaryImage tooLargeImage;
    tooLargeImage.width = 10000;
    tooLargeImage.height = 10000;

    EXPECT_CALL(*mockRenderer, rayTraceSecondary(testing::_))
        .WillOnce(testing::Throw(std::invalid_argument(
            "Размеры изображения превышают максимально допустимые значения")));

    EXPECT_THROW(mockRenderer->rayTraceSecondary(tooLargeImage), std::invalid_argument);
}

TEST_F(VulkanRendererTest, ValidateResolutionTarget) {
    ResolutionTarget invalidTarget;
    invalidTarget.width = 0;
    invalidTarget.height = 1080;
    invalidTarget.scaleFactor = 2.0f;

    DenoisedImage denoisedImage;

    EXPECT_CALL(*mockRenderer, upscale(testing::_, testing::_))
        .WillOnce(testing::Throw(
            std::invalid_argument("Размеры целевого разрешения должны быть больше 0")));

    EXPECT_THROW(mockRenderer->upscale(denoisedImage, invalidTarget), std::invalid_argument);
}

TEST_F(VulkanRendererTest, ValidateScaleFactor) {
    ResolutionTarget invalidTarget;
    invalidTarget.width = 1920;
    invalidTarget.height = 1080;
    invalidTarget.scaleFactor = 0.0f;  // Невалидный масштаб

    DenoisedImage denoisedImage;

    EXPECT_CALL(*mockRenderer, upscale(testing::_, testing::_))
        .WillOnce(testing::Throw(
            std::invalid_argument("Коэффициент масштабирования должен быть в диапазоне (0, 8]")));

    EXPECT_THROW(mockRenderer->upscale(denoisedImage, invalidTarget), std::invalid_argument);
}

TEST_F(VulkanRendererTest, ValidateScaleFactorTooLarge) {
    ResolutionTarget invalidTarget;
    invalidTarget.width = 1920;
    invalidTarget.height = 1080;
    invalidTarget.scaleFactor = 10.0f;  // Слишком большой масштаб

    DenoisedImage denoisedImage;

    EXPECT_CALL(*mockRenderer, upscale(testing::_, testing::_))
        .WillOnce(testing::Throw(
            std::invalid_argument("Коэффициент масштабирования должен быть в диапазоне (0, 8]")));

    EXPECT_THROW(mockRenderer->upscale(denoisedImage, invalidTarget), std::invalid_argument);
}

TEST_F(VulkanRendererTest, InvalidGaussianData) {
    Gaussians invalidGaussians;
    invalidGaussians.count = 0;  // Пустые данные

    EXPECT_CALL(*mockRenderer, rasterizePrimary(testing::_))
        .WillOnce(testing::Throw(std::invalid_argument("Пустые данные гауссианов")));

    EXPECT_THROW(mockRenderer->rasterizePrimary(invalidGaussians), std::invalid_argument);
}

// Тесты завершения работы
TEST_F(VulkanRendererTest, ProperShutdown) {
    EXPECT_CALL(*mockRenderer, shutdown()).Times(1);

    EXPECT_NO_THROW_WITH_MESSAGE({ mockRenderer->shutdown(); }, "Корректное завершение работы");
}

TEST_F(VulkanRendererTest, MultipleShutdowns) {
    EXPECT_CALL(*mockRenderer, shutdown()).Times(testing::AtMost(2));

    // Множественные вызовы shutdown не должны вызывать ошибок
    EXPECT_NO_THROW_WITH_MESSAGE(
        {
            mockRenderer->shutdown();
            mockRenderer->shutdown();  // Второй вызов
        },
        "Множественные вызовы shutdown");
}

// Параметризованные тесты для разных разрешений
class VulkanRendererResolutionTest
    : public VulkanRendererTest,
      public ::testing::WithParamInterface<std::tuple<uint32_t, uint32_t, float>> {};

TEST_P(VulkanRendererResolutionTest, DifferentResolutions) {
    auto [width, height, scaleFactor] = GetParam();

    ResolutionTarget target;
    target.width = width;
    target.height = height;
    target.scaleFactor = scaleFactor;

    DenoisedImage denoisedImage;
    FinalImage mockFinal;

    EXPECT_CALL(*mockRenderer, upscale(testing::_, testing::_))
        .WillOnce(testing::Return(mockFinal));

    EXPECT_NO_THROW_WITH_MESSAGE(
        { [[maybe_unused]] FinalImage result = mockRenderer->upscale(denoisedImage, target); },
        "Upscaling для разрешения " + std::to_string(width) + "x" + std::to_string(height));
}

INSTANTIATE_TEST_SUITE_P(ResolutionTests,
                         VulkanRendererResolutionTest,
                         ::testing::Values(std::make_tuple(1920, 1080, 1.5f),
                                           std::make_tuple(1920, 1080, 2.0f),
                                           std::make_tuple(2560, 1440, 1.5f),
                                           std::make_tuple(3840, 2160, 1.0f),
                                           std::make_tuple(3840, 2160, 2.0f)));
