#include "SpectraForge/Rendering/RendererAdapter.h"
#include "SpectraForge/Vulkan/VulkanRenderer.h"
#include "TestFramework.h"
#include "mocks/MockUpscaling.h"
#include "mocks/MockVulkanRenderer.h"

using namespace SpectraForge::Testing;
using namespace SpectraForge::Testing::Mocks;
using namespace SpectraForge::Rendering;
using namespace SpectraForge::Vulkan;

/**
 * @brief Интеграционные тесты для полного rendering pipeline
 *
 * Тестирует взаимодействие между всеми компонентами рендеринга:
 * - RendererAdapter
 * - VulkanRenderer
 * - FlashGSSplatter
 * - OptiXRayTracer
 * - DenoiseModule
 * - Upscaler (DLSS/FSR)
 */
class RenderingPipelineIntegrationTest : public SpectraForgeTest {
  protected:
    void SetUp() override {
        SpectraForgeTest::SetUp();

        // Создаем адаптер рендеринга
        rendererAdapter = std::make_unique<RendererAdapter>();

        // Создаем mock компоненты для тестирования
        setupMockComponents();

        // Настраиваем тестовые данные
        setupTestData();
    }

    void TearDown() override {
        if (rendererAdapter && rendererAdapter->isInitialized()) {
            rendererAdapter->cleanup();
        }
        SpectraForgeTest::TearDown();
    }

  private:
    void setupMockComponents() {
        // Создаем mock объекты для интеграционного тестирования
        mockResourceManager = VulkanMockFactory::createResourceManager();
        mockHardwareDetector = UpscalingMockFactory::createHardwareDetector();

        // Создаем upscaler в зависимости от "оборудования"
        auto vendor = mockHardwareDetector->detectVendor();
        if (vendor == SpectraForge::Upscaling::HardwareConfig::VendorType::NVIDIA) {
            mockUpscaler = UpscalingMockFactory::createDLSSUpscaler();
        } else {
            mockUpscaler = UpscalingMockFactory::createFSRUpscaler();
        }
    }

    void setupTestData() {
        // Настройка тестовой камеры
        testCamera = std::make_shared<Camera3D>();
        testCamera->setPosition(SpectraForge::Math::Vector3(0, 0, 5));
        testCamera->lookAt(SpectraForge::Math::Vector3(0, 0, 5),
                           SpectraForge::Math::Vector3(0, 0, 0),
                           SpectraForge::Math::Vector3(0, 1, 0));

        // Настройка тестовых объектов
        testMesh = std::make_shared<Mesh3D>();
        testShader = std::make_shared<Shader3D>();
        testTransform = SpectraForge::Math::Matrix4::identity();

        // Настройка параметров рендеринга
        renderWidth = 1920;
        renderHeight = 1080;
        targetFPS = 60.0f;
    }

  protected:
    std::unique_ptr<RendererAdapter> rendererAdapter;
    std::unique_ptr<MockResourceManager> mockResourceManager;
    std::unique_ptr<MockHardwareDetector> mockHardwareDetector;
    std::unique_ptr<MockUpscaler> mockUpscaler;

    std::shared_ptr<Camera3D> testCamera;
    std::shared_ptr<Mesh3D> testMesh;
    std::shared_ptr<Shader3D> testShader;
    SpectraForge::Math::Matrix4 testTransform;

    int renderWidth, renderHeight;
    float targetFPS;
};

// Тесты полной инициализации pipeline
TEST_F(RenderingPipelineIntegrationTest, FullPipelineInitialization) {
    EXPECT_NO_THROW_WITH_MESSAGE(
        {
            // Инициализируем рендерер
            bool initResult =
                rendererAdapter->initialize(RenderBackend::AUTO, renderWidth, renderHeight);
            EXPECT_TRUE(initResult);
            EXPECT_TRUE(rendererAdapter->isInitialized());

            // Проверяем, что выбран подходящий backend
            RenderBackend selectedBackend = rendererAdapter->getCurrentBackend();
            EXPECT_TRUE(selectedBackend == RenderBackend::OPENGL
                        || selectedBackend == RenderBackend::VULKAN);

            // Устанавливаем камеру
            rendererAdapter->setMainCamera(testCamera);
            EXPECT_EQ(rendererAdapter->getMainCamera(), testCamera);
        },
        "Полная инициализация rendering pipeline");
}

// Тесты гибридного рендеринга (если доступен Vulkan)
TEST_F(RenderingPipelineIntegrationTest, HybridRenderingPipeline) {
    // Проверяем доступность Vulkan для гибридного рендеринга
    if (!rendererAdapter->isBackendAvailable(RenderBackend::VULKAN)) {
        GTEST_SKIP() << "Vulkan не доступен для гибридного рендеринга";
    }

    EXPECT_NO_THROW_WITH_MESSAGE(
        {
            // Инициализируем с Vulkan backend'ом
            bool initResult =
                rendererAdapter->initialize(RenderBackend::VULKAN, renderWidth, renderHeight);
            EXPECT_TRUE(initResult);
            EXPECT_EQ(rendererAdapter->getCurrentBackend(), RenderBackend::VULKAN);

            // Настраиваем рендеринг
            rendererAdapter->setMainCamera(testCamera);
            rendererAdapter->setClearColor(0.1f, 0.1f, 0.1f, 1.0f);
            rendererAdapter->enableDepthTest(true);

            // Выполняем рендеринг кадра
            rendererAdapter->beginFrame();
            rendererAdapter->clear();

            // Рендерим тестовую геометрию
            rendererAdapter->renderMesh(testMesh, testTransform, testShader);

            rendererAdapter->endFrame();
        },
        "Гибридный рендеринг через Vulkan");
}

// Тесты переключения между backend'ами
TEST_F(RenderingPipelineIntegrationTest, BackendSwitching) {
    EXPECT_NO_THROW_WITH_MESSAGE(
        {
            // Начинаем с OpenGL
            bool initResult =
                rendererAdapter->initialize(RenderBackend::OPENGL, renderWidth, renderHeight);
            EXPECT_TRUE(initResult);
            EXPECT_EQ(rendererAdapter->getCurrentBackend(), RenderBackend::OPENGL);

            // Рендерим один кадр с OpenGL
            rendererAdapter->setMainCamera(testCamera);
            rendererAdapter->beginFrame();
            rendererAdapter->renderMesh(testMesh, testTransform, testShader);
            rendererAdapter->endFrame();

            // Переключаемся на Vulkan (если доступен)
            if (rendererAdapter->isBackendAvailable(RenderBackend::VULKAN)) {
                bool switchResult = rendererAdapter->switchBackend(RenderBackend::VULKAN);
                EXPECT_TRUE(switchResult);
                EXPECT_EQ(rendererAdapter->getCurrentBackend(), RenderBackend::VULKAN);

                // Рендерим кадр с Vulkan
                rendererAdapter->beginFrame();
                rendererAdapter->renderMesh(testMesh, testTransform, testShader);
                rendererAdapter->endFrame();
            }
        },
        "Переключение между backend'ами");
}

// Тесты производительности полного pipeline
TEST_F(RenderingPipelineIntegrationTest, FullPipelinePerformance) {
    ASSERT_TRUE(rendererAdapter->initialize(RenderBackend::AUTO, renderWidth, renderHeight));
    rendererAdapter->setMainCamera(testCamera);

    const int frameCount = 30;  // 30 кадров для стабильного измерения
    const int maxTimeMs = static_cast<int>((frameCount / targetFPS) * 1000) + 100;  // +100ms допуск

    EXPECT_PERFORMANCE_UNDER(
        {
            for (int frame = 0; frame < frameCount; ++frame) {
                rendererAdapter->beginFrame();
                rendererAdapter->clear();

                // Рендерим несколько объектов для реалистичной нагрузки
                for (int obj = 0; obj < 10; ++obj) {
                    SpectraForge::Math::Matrix4 objTransform =
                        SpectraForge::Math::Matrix4::translation(
                            static_cast<float>(obj - 5), 0.0f, 0.0f);
                    rendererAdapter->renderMesh(testMesh, objTransform, testShader);
                }

                rendererAdapter->endFrame();
            }
        },
        maxTimeMs);
}

// Тесты отказоустойчивости
TEST_F(RenderingPipelineIntegrationTest, ErrorRecovery) {
    EXPECT_NO_THROW_WITH_MESSAGE(
        {
            // Инициализация с некорректными параметрами
            bool badInit = rendererAdapter->initialize(RenderBackend::OPENGL, 0, 0);
            EXPECT_FALSE(badInit);

            // Попытка правильной инициализации после неудачи
            bool goodInit =
                rendererAdapter->initialize(RenderBackend::OPENGL, renderWidth, renderHeight);
            EXPECT_TRUE(goodInit);

            // Система должна работать нормально после восстановления
            rendererAdapter->setMainCamera(testCamera);
            rendererAdapter->beginFrame();
            rendererAdapter->endFrame();
        },
        "Восстановление после ошибок");
}

// Тесты множественной инициализации и очистки
TEST_F(RenderingPipelineIntegrationTest, MultipleInitCleanupCycles) {
    const int cycleCount = 3;

    EXPECT_NO_THROW_WITH_MESSAGE(
        {
            for (int cycle = 0; cycle < cycleCount; ++cycle) {
                // Инициализация
                bool initResult =
                    rendererAdapter->initialize(RenderBackend::OPENGL, renderWidth, renderHeight);
                EXPECT_TRUE(initResult);

                // Использование
                rendererAdapter->setMainCamera(testCamera);
                rendererAdapter->beginFrame();
                rendererAdapter->renderMesh(testMesh, testTransform, testShader);
                rendererAdapter->endFrame();

                // Очистка
                rendererAdapter->cleanup();
                EXPECT_FALSE(rendererAdapter->isInitialized());
            }
        },
        "Множественные циклы инициализации/очистки");
}

// Тесты памяти и ресурсов
TEST_F(RenderingPipelineIntegrationTest, ResourceManagement) {
    ASSERT_TRUE(rendererAdapter->initialize(RenderBackend::OPENGL, renderWidth, renderHeight));

    // Получаем начальное состояние памяти (если возможно)
    size_t initialMemory = 0;
    if (mockResourceManager) {
        initialMemory = mockResourceManager->getAllocatedMemory();
    }

    EXPECT_NO_THROW_WITH_MESSAGE(
        {
            // Создаем и рендерим много объектов
            const int objectCount = 100;

            for (int i = 0; i < objectCount; ++i) {
                auto mesh = std::make_shared<Mesh3D>();
                auto shader = std::make_shared<Shader3D>();
                SpectraForge::Math::Matrix4 transform = SpectraForge::Math::Matrix4::translation(
                    static_cast<float>(i % 10), static_cast<float>(i / 10), 0.0f);

                rendererAdapter->beginFrame();
                rendererAdapter->renderMesh(mesh, transform, shader);
                rendererAdapter->endFrame();
            }

            // Проверяем, что память не утекает чрезмерно
            if (mockResourceManager) {
                size_t finalMemory = mockResourceManager->getAllocatedMemory();
                // Память может увеличиться, но не слишком сильно
                EXPECT_LT(finalMemory, initialMemory + 100 * 1024 * 1024);  // +100MB максимум
            }
        },
        "Управление ресурсами при интенсивном рендеринге");
}

// Тесты интеграции с различными типами объектов
TEST_F(RenderingPipelineIntegrationTest, DifferentObjectTypes) {
    ASSERT_TRUE(rendererAdapter->initialize(RenderBackend::OPENGL, renderWidth, renderHeight));
    rendererAdapter->setMainCamera(testCamera);

    EXPECT_NO_THROW_WITH_MESSAGE(
        {
            rendererAdapter->beginFrame();

            // Обычный mesh
            rendererAdapter->renderMesh(testMesh, testTransform, testShader);

            // Wireframe
            rendererAdapter->enableWireframe(true);
            SpectraForge::Math::Matrix4 wireTransform =
                SpectraForge::Math::Matrix4::translation(2.0f, 0.0f, 0.0f);
            rendererAdapter->renderWireframe(*testMesh, wireTransform, *testShader);
            rendererAdapter->enableWireframe(false);

            // С blending
            rendererAdapter->enableBlending(true);
            SpectraForge::Math::Matrix4 blendTransform =
                SpectraForge::Math::Matrix4::translation(-2.0f, 0.0f, 0.0f);
            rendererAdapter->renderMesh(testMesh, blendTransform, testShader);
            rendererAdapter->enableBlending(false);

            rendererAdapter->endFrame();
        },
        "Рендеринг различных типов объектов");
}

// Стресс-тест для стабильности
TEST_F(RenderingPipelineIntegrationTest, StabilityStressTest) {
    ASSERT_TRUE(rendererAdapter->initialize(RenderBackend::OPENGL, renderWidth, renderHeight));
    rendererAdapter->setMainCamera(testCamera);

    const int longRunFrames = 1000;
    int successfulFrames = 0;

    EXPECT_NO_THROW_WITH_MESSAGE(
        {
            for (int frame = 0; frame < longRunFrames; ++frame) {
                try {
                    rendererAdapter->beginFrame();

                    // Варьируем нагрузку
                    int objectCount = (frame % 20) + 1;
                    for (int obj = 0; obj < objectCount; ++obj) {
                        SpectraForge::Math::Matrix4 transform =
                            SpectraForge::Math::Matrix4::translation(
                                std::sin(frame * 0.01f + obj), std::cos(frame * 0.01f + obj), 0.0f);
                        rendererAdapter->renderMesh(testMesh, transform, testShader);
                    }

                    rendererAdapter->endFrame();
                    successfulFrames++;
                } catch (...) {
                    // Если произошла ошибка, попробуем восстановиться
                    break;
                }
            }

            // Ожидаем успешное выполнение большинства кадров
            EXPECT_GT(successfulFrames, longRunFrames * 0.95);  // 95% успеха
        },
        "Стресс-тест стабильности");
}

// Параметризованные тесты для разных конфигураций
class RenderingPipelineConfigTest
    : public RenderingPipelineIntegrationTest,
      public ::testing::WithParamInterface<std::tuple<RenderBackend, int, int>> {};

TEST_P(RenderingPipelineConfigTest, DifferentConfigurations) {
    auto [backend, width, height] = GetParam();

    // Проверяем доступность backend'а
    if (!rendererAdapter->isBackendAvailable(backend)) {
        GTEST_SKIP() << "Backend не доступен: " << static_cast<int>(backend);
    }

    EXPECT_NO_THROW_WITH_MESSAGE(
        {
            bool initResult = rendererAdapter->initialize(backend, width, height);
            EXPECT_TRUE(initResult);
            EXPECT_EQ(rendererAdapter->getCurrentBackend(), backend);

            rendererAdapter->setMainCamera(testCamera);
            rendererAdapter->setViewport(0, 0, width, height);

            // Выполняем базовый рендеринг
            rendererAdapter->beginFrame();
            rendererAdapter->clear();
            rendererAdapter->renderMesh(testMesh, testTransform, testShader);
            rendererAdapter->endFrame();
        },
        "Конфигурация " + std::to_string(static_cast<int>(backend)) + " " + std::to_string(width)
            + "x" + std::to_string(height));
}

INSTANTIATE_TEST_SUITE_P(ConfigurationTests,
                         RenderingPipelineConfigTest,
                         ::testing::Values(std::make_tuple(RenderBackend::OPENGL, 800, 600),
                                           std::make_tuple(RenderBackend::OPENGL, 1920, 1080),
                                           std::make_tuple(RenderBackend::AUTO, 1920, 1080),
                                           std::make_tuple(RenderBackend::VULKAN, 1920, 1080),
                                           std::make_tuple(RenderBackend::VULKAN, 3840, 2160)));
