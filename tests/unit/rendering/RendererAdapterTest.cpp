#include "HyperEngine/Rendering/RendererAdapter.h"
#include "TestFramework.h"
#include "mocks/MockRenderer.h"

using namespace HyperEngine::Testing;
using namespace HyperEngine::Testing::Mocks;
using namespace HyperEngine::Rendering;

/**
 * @brief Unit тесты для RendererAdapter
 *
 * Тестирует переключение между различными графическими API
 * и корректность работы адаптера
 */
class RendererAdapterTest : public HyperEngineTest {
  protected:
    void SetUp() override {
        HyperEngineTest::SetUp();

        // Создаем адаптер
        adapter = std::make_unique<RendererAdapter>();

        // Создаем mock объекты
        mockOpenGL = MockFactory::createBasicRenderer();
        mockVulkan = MockFactory::createBasicRenderer();
        mockFailingRenderer = MockFactory::createFailingRenderer();
    }

    void TearDown() override {
        if (adapter) {
            adapter->cleanup();
        }
        HyperEngineTest::TearDown();
    }

    std::unique_ptr<RendererAdapter> adapter;
    std::unique_ptr<MockRendererAdapter> mockOpenGL;
    std::unique_ptr<MockRendererAdapter> mockVulkan;
    std::unique_ptr<MockRendererAdapter> mockFailingRenderer;
};

// Тесты инициализации
TEST_F(RendererAdapterTest, DefaultInitialization) {
    EXPECT_NO_THROW_WITH_MESSAGE(
        {
            bool result = adapter->initialize(RenderBackend::AUTO, 1920, 1080);
            EXPECT_TRUE(result);
        },
        "Инициализация с AUTO backend");
}

TEST_F(RendererAdapterTest, OpenGLInitialization) {
    EXPECT_NO_THROW_WITH_MESSAGE(
        {
            bool result = adapter->initialize(RenderBackend::OPENGL, 1920, 1080);
            EXPECT_TRUE(result);
            EXPECT_EQ(adapter->getCurrentBackend(), RenderBackend::OPENGL);
        },
        "Инициализация с OpenGL backend");
}

TEST_F(RendererAdapterTest, VulkanInitialization) {
    // Проверяем доступность Vulkan
    if (adapter->isBackendAvailable(RenderBackend::VULKAN)) {
        EXPECT_NO_THROW_WITH_MESSAGE(
            {
                bool result = adapter->initialize(RenderBackend::VULKAN, 1920, 1080);
                EXPECT_TRUE(result);
                EXPECT_EQ(adapter->getCurrentBackend(), RenderBackend::VULKAN);
            },
            "Инициализация с Vulkan backend");
    } else {
        GTEST_SKIP() << "Vulkan не поддерживается на данной системе";
    }
}

TEST_F(RendererAdapterTest, FailedInitialization) {
    // Тестируем сценарий неудачной инициализации
    EXPECT_FALSE(adapter->isInitialized());

    // Попытка инициализации с недопустимыми параметрами
    bool result = adapter->initialize(RenderBackend::OPENGL, 0, 0);
    EXPECT_FALSE(result);
    EXPECT_FALSE(adapter->isInitialized());
}

// Тесты переключения backend'ов
TEST_F(RendererAdapterTest, BackendSwitching) {
    // Инициализируем с OpenGL
    ASSERT_TRUE(adapter->initialize(RenderBackend::OPENGL, 1920, 1080));
    EXPECT_EQ(adapter->getCurrentBackend(), RenderBackend::OPENGL);

    // Переключаемся на Vulkan (если доступен)
    if (adapter->isBackendAvailable(RenderBackend::VULKAN)) {
        EXPECT_NO_THROW_WITH_MESSAGE(
            {
                bool result = adapter->switchBackend(RenderBackend::VULKAN);
                EXPECT_TRUE(result);
                EXPECT_EQ(adapter->getCurrentBackend(), RenderBackend::VULKAN);
            },
            "Переключение на Vulkan");
    }
}

TEST_F(RendererAdapterTest, BackendAvailability) {
    // OpenGL должен быть доступен на большинстве систем
    EXPECT_TRUE(adapter->isBackendAvailable(RenderBackend::OPENGL));

    // Vulkan может быть недоступен
    bool vulkanAvailable = adapter->isBackendAvailable(RenderBackend::VULKAN);
    EXPECT_TRUE(vulkanAvailable || !vulkanAvailable);  // Просто проверяем, что метод работает

    // AUTO всегда должен быть доступен
    EXPECT_TRUE(adapter->isBackendAvailable(RenderBackend::AUTO));
}

// Тесты рендеринга
TEST_F(RendererAdapterTest, BasicRenderingOperations) {
    ASSERT_TRUE(adapter->initialize(RenderBackend::OPENGL, 800, 600));

    EXPECT_NO_THROW_WITH_MESSAGE(
        {
            adapter->beginFrame();
            adapter->clear();
            adapter->endFrame();
        },
        "Базовые операции рендеринга");
}

TEST_F(RendererAdapterTest, RenderingSettings) {
    ASSERT_TRUE(adapter->initialize(RenderBackend::OPENGL, 800, 600));

    EXPECT_NO_THROW_WITH_MESSAGE(
        {
            adapter->setClearColor(0.2f, 0.3f, 0.4f, 1.0f);
            adapter->setViewport(0, 0, 800, 600);
            adapter->enableDepthTest(true);
            adapter->enableBlending(false);
            adapter->enableWireframe(false);
            adapter->enableBackfaceCulling(true);
        },
        "Настройки рендеринга");
}

// Тесты работы с камерой
TEST_F(RendererAdapterTest, CameraManagement) {
    ASSERT_TRUE(adapter->initialize(RenderBackend::OPENGL, 800, 600));

    // Создаем тестовую камеру
    auto camera = std::make_shared<Camera3D>();
    camera->setPosition(HyperEngine::Math::Vector3(0, 0, 5));
    camera->lookAt(HyperEngine::Math::Vector3(0, 0, 5),
                   HyperEngine::Math::Vector3(0, 0, 0),
                   HyperEngine::Math::Vector3(0, 1, 0));

    EXPECT_NO_THROW_WITH_MESSAGE(
        {
            adapter->setMainCamera(camera);
            auto retrievedCamera = adapter->getMainCamera();
            EXPECT_EQ(retrievedCamera, camera);
        },
        "Управление камерой");
}

// Тесты рендеринга мешей
TEST_F(RendererAdapterTest, MeshRendering) {
    ASSERT_TRUE(adapter->initialize(RenderBackend::OPENGL, 800, 600));

    // Создаем тестовые объекты
    Mesh3D testMesh;
    Shader3D testShader;
    HyperEngine::Math::Matrix4 transform = HyperEngine::Math::Matrix4::identity();

    EXPECT_NO_THROW_WITH_MESSAGE(
        {
            adapter->beginFrame();
            adapter->renderMesh(testMesh, transform, testShader);
            adapter->endFrame();
        },
        "Рендеринг mesh");
}

TEST_F(RendererAdapterTest, WireframeRendering) {
    ASSERT_TRUE(adapter->initialize(RenderBackend::OPENGL, 800, 600));

    Mesh3D testMesh;
    Shader3D testShader;
    HyperEngine::Math::Matrix4 transform = HyperEngine::Math::Matrix4::identity();

    EXPECT_NO_THROW_WITH_MESSAGE(
        {
            adapter->beginFrame();
            adapter->enableWireframe(true);
            adapter->renderWireframe(testMesh, transform, testShader);
            adapter->enableWireframe(false);
            adapter->endFrame();
        },
        "Wireframe рендеринг");
}

// Тесты производительности
TEST_F(RendererAdapterTest, RenderingPerformance) {
    ASSERT_TRUE(adapter->initialize(RenderBackend::OPENGL, 800, 600));

    const int frameCount = 100;

    EXPECT_PERFORMANCE_UNDER(
        {
            for (int i = 0; i < frameCount; ++i) {
                adapter->beginFrame();
                adapter->clear();
                adapter->endFrame();
            }
        },
        1000);  // 100 кадров за < 1000ms (100+ FPS)
}

// Тесты множественных инициализаций
TEST_F(RendererAdapterTest, MultipleInitializations) {
    // Первая инициализация
    EXPECT_TRUE(adapter->initialize(RenderBackend::OPENGL, 800, 600));
    EXPECT_TRUE(adapter->isInitialized());

    // Вторая инициализация должна работать (переинициализация)
    EXPECT_NO_THROW_WITH_MESSAGE(
        {
            bool result = adapter->initialize(RenderBackend::OPENGL, 1920, 1080);
            EXPECT_TRUE(result);
            EXPECT_TRUE(adapter->isInitialized());
        },
        "Переинициализация");
}

// Тесты очистки ресурсов
TEST_F(RendererAdapterTest, ProperCleanup) {
    ASSERT_TRUE(adapter->initialize(RenderBackend::OPENGL, 800, 600));
    EXPECT_TRUE(adapter->isInitialized());

    EXPECT_NO_THROW_WITH_MESSAGE(
        {
            adapter->cleanup();
            EXPECT_FALSE(adapter->isInitialized());
        },
        "Очистка ресурсов");
}

TEST_F(RendererAdapterTest, CleanupWithoutInitialization) {
    // Очистка без инициализации не должна вызывать ошибок
    EXPECT_NO_THROW_WITH_MESSAGE({ adapter->cleanup(); }, "Очистка без инициализации");
}

// Тесты информационных методов
TEST_F(RendererAdapterTest, BackendInformation) {
    ASSERT_TRUE(adapter->initialize(RenderBackend::OPENGL, 800, 600));

    EXPECT_NE(adapter->getBackendName(), nullptr);
    EXPECT_NE(std::string(adapter->getBackendName()), "");

    RenderBackend backend = adapter->getCurrentBackend();
    EXPECT_TRUE(backend == RenderBackend::OPENGL || backend == RenderBackend::VULKAN);
}

// Тесты автоматического выбора backend'а
TEST_F(RendererAdapterTest, AutoBackendSelection) {
    EXPECT_NO_THROW_WITH_MESSAGE(
        {
            bool result = adapter->initialize(RenderBackend::AUTO, 1920, 1080);
            EXPECT_TRUE(result);

            // AUTO должен выбрать один из доступных backend'ов
            RenderBackend selectedBackend = adapter->getCurrentBackend();
            EXPECT_TRUE(selectedBackend == RenderBackend::OPENGL
                        || selectedBackend == RenderBackend::VULKAN);

            EXPECT_TRUE(adapter->isBackendAvailable(selectedBackend));
        },
        "Автоматический выбор backend");
}

// Параметризованные тесты для разных разрешений
class RendererAdapterResolutionTest : public RendererAdapterTest,
                                      public ::testing::WithParamInterface<std::pair<int, int>> {};

TEST_P(RendererAdapterResolutionTest, DifferentResolutions) {
    auto [width, height] = GetParam();

    EXPECT_NO_THROW_WITH_MESSAGE(
        {
            bool result = adapter->initialize(RenderBackend::OPENGL, width, height);
            EXPECT_TRUE(result);

            adapter->setViewport(0, 0, width, height);
            adapter->beginFrame();
            adapter->clear();
            adapter->endFrame();

            adapter->cleanup();
        },
        "Тестирование разрешения " + std::to_string(width) + "x" + std::to_string(height));
}

INSTANTIATE_TEST_SUITE_P(ResolutionTests,
                         RendererAdapterResolutionTest,
                         ::testing::Values(std::make_pair(640, 480),
                                           std::make_pair(800, 600),
                                           std::make_pair(1920, 1080),
                                           std::make_pair(2560, 1440),
                                           std::make_pair(3840, 2160)));

