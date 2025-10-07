/**
 * @file modern_renderer3d_test.cpp
 * @brief Тесты для ModernRenderer3D (21 функция, ~35 тестов)
 * 
 * Покрытие:
 * - Lifecycle: конструктор, деструктор, initialize, shutdown
 * - IRenderer интерфейс: renderFrame, getType, supportsFeature
 * - Конфигурация: setConfigParameter, getConfigParameter, loadConfig, saveConfig
 * - Стратегии: setRenderStrategy
 * - Пост-эффекты: addPostProcessEffect, removePostProcessEffect
 */

#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include "SpectraForge/Rendering/RendererAdapter.h"
#include "SpectraForge/Rendering/Common/IRenderer.h"
#include "SpectraForge/Core/Logger.h"
#include <memory>
#include <stdexcept>

using namespace SpectraForge::Rendering;
using namespace SpectraForge::Core;
using ::testing::Return;
using ::testing::_;
using ::testing::NiceMock;

// ============================================================================
// Mock Classes
// ============================================================================

/**
 * @brief Mock IRenderStrategy
 */
class MockRenderStrategy : public IRenderStrategy {
public:
    MOCK_METHOD(bool, initialize, (), (override));
    MOCK_METHOD(void, shutdown, (), (override));
    MOCK_METHOD(void, render, (const FrameData&), (override));
    MOCK_METHOD(bool, supportsFeature, (RenderingFeature), (const, override));
    MOCK_METHOD(std::string, getName, (), (const, override));
};

/**
 * @brief Mock ILightingSystem
 */
class MockLightingSystem : public ILightingSystem {
public:
    MOCK_METHOD(bool, initialize, (), (override));
    MOCK_METHOD(void, shutdown, (), (override));
    MOCK_METHOD(void, updateLighting, (const FrameData&), (override));
};

/**
 * @brief Mock ICameraSystem
 */
class MockCameraSystem : public ICameraSystem {
public:
    MOCK_METHOD(bool, initialize, (), (override));
    MOCK_METHOD(void, shutdown, (), (override));
    MOCK_METHOD(void, updateCamera, (const FrameData&), (override));
};

/**
 * @brief Mock IRenderStatistics
 */
class MockRenderStatistics : public IRenderStatistics {
public:
    MOCK_METHOD(void, beginFrame, (), (override));
    MOCK_METHOD(void, endFrame, (), (override));
    MOCK_METHOD(RenderingStats, getStats, (), (const, override));
};

/**
 * @brief Mock IPostProcessEffect
 */
class MockPostProcessEffect : public IPostProcessEffect {
public:
    MOCK_METHOD(bool, initialize, (), (override));
    MOCK_METHOD(void, shutdown, (), (override));
    MOCK_METHOD(bool, isEnabled, (), (const, override));
    MOCK_METHOD(std::string, getName, (), (const, override));
};

// ============================================================================
// Test Fixture
// ============================================================================

class ModernRenderer3DTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Create mocks
        mockStrategy = std::make_shared<NiceMock<MockRenderStrategy>>();
        mockLighting = std::make_shared<NiceMock<MockLightingSystem>>();
        mockCamera = std::make_shared<NiceMock<MockCameraSystem>>();
        mockStats = std::make_shared<NiceMock<MockRenderStatistics>>();
        mockLogger = std::make_shared<Logger>();

        // Set default expectations
        ON_CALL(*mockStrategy, initialize()).WillByDefault(Return(true));
        ON_CALL(*mockStrategy, getName()).WillByDefault(Return("MockStrategy"));
        ON_CALL(*mockLighting, initialize()).WillByDefault(Return(true));
        ON_CALL(*mockCamera, initialize()).WillByDefault(Return(true));
        ON_CALL(*mockStats, getStats()).WillByDefault(Return(RenderingStats{}));
    }

    void TearDown() override {
        if (renderer) {
            renderer->shutdown();
        }
        renderer.reset();
    }

    std::unique_ptr<ModernRenderer3D> renderer;
    std::shared_ptr<MockRenderStrategy> mockStrategy;
    std::shared_ptr<MockLightingSystem> mockLighting;
    std::shared_ptr<MockCameraSystem> mockCamera;
    std::shared_ptr<MockRenderStatistics> mockStats;
    std::shared_ptr<Logger> mockLogger;
};

// ============================================================================
// 1. LIFECYCLE TESTS
// ============================================================================

/**
 * @test Тест конструктора с dependency injection
 */
TEST_F(ModernRenderer3DTest, ConstructorDependencyInjectionTest) {
    // Act
    EXPECT_NO_THROW({
        renderer = std::make_unique<ModernRenderer3D>(
            mockStrategy, mockLighting, mockCamera, mockStats, mockLogger
        );
    });

    // Assert
    EXPECT_NE(renderer, nullptr);
    EXPECT_FALSE(renderer->isInitialized());
}

/**
 * @test Тест конструктора с nullptr strategy
 */
TEST_F(ModernRenderer3DTest, ConstructorNullStrategyTest) {
    // Act & Assert
    EXPECT_THROW({
        renderer = std::make_unique<ModernRenderer3D>(
            nullptr, mockLighting, mockCamera, mockStats, mockLogger
        );
    }, std::invalid_argument);
}

/**
 * @test Тест конструктора с nullptr lighting
 */
TEST_F(ModernRenderer3DTest, ConstructorNullLightingTest) {
    // Act & Assert
    EXPECT_THROW({
        renderer = std::make_unique<ModernRenderer3D>(
            mockStrategy, nullptr, mockCamera, mockStats, mockLogger
        );
    }, std::invalid_argument);
}

/**
 * @test Тест конструктора с nullptr camera
 */
TEST_F(ModernRenderer3DTest, ConstructorNullCameraTest) {
    // Act & Assert
    EXPECT_THROW({
        renderer = std::make_unique<ModernRenderer3D>(
            mockStrategy, mockLighting, nullptr, mockStats, mockLogger
        );
    }, std::invalid_argument);
}

/**
 * @test Тест конструктора с nullptr statistics
 */
TEST_F(ModernRenderer3DTest, ConstructorNullStatsTest) {
    // Act & Assert
    EXPECT_THROW({
        renderer = std::make_unique<ModernRenderer3D>(
            mockStrategy, mockLighting, mockCamera, nullptr, mockLogger
        );
    }, std::invalid_argument);
}

/**
 * @test Тест конструктора с nullptr logger
 */
TEST_F(ModernRenderer3DTest, ConstructorNullLoggerTest) {
    // Act & Assert
    EXPECT_THROW({
        renderer = std::make_unique<ModernRenderer3D>(
            mockStrategy, mockLighting, mockCamera, mockStats, nullptr
        );
    }, std::invalid_argument);
}

/**
 * @test Тест деструктора
 */
TEST_F(ModernRenderer3DTest, DestructorTest) {
    // Arrange
    auto tempRenderer = std::make_unique<ModernRenderer3D>(
        mockStrategy, mockLighting, mockCamera, mockStats, mockLogger
    );
    tempRenderer->initialize();

    // Act
    tempRenderer.reset();

    // Assert - should call shutdown automatically
    SUCCEED();
}

/**
 * @test Тест initialize() - успешная инициализация
 */
TEST_F(ModernRenderer3DTest, InitializeSuccessTest) {
    // Arrange
    renderer = std::make_unique<ModernRenderer3D>(
        mockStrategy, mockLighting, mockCamera, mockStats, mockLogger
    );

    // Act
    bool result = renderer->initialize();

    // Assert
    EXPECT_TRUE(result);
    EXPECT_TRUE(renderer->isInitialized());
    EXPECT_TRUE(renderer->isReady());
}

/**
 * @test Тест initialize() с ошибкой strategy
 */
TEST_F(ModernRenderer3DTest, InitializeStrategyFailureTest) {
    // Arrange
    ON_CALL(*mockStrategy, initialize()).WillByDefault(Return(false));
    renderer = std::make_unique<ModernRenderer3D>(
        mockStrategy, mockLighting, mockCamera, mockStats, mockLogger
    );

    // Act
    bool result = renderer->initialize();

    // Assert
    EXPECT_FALSE(result);
    EXPECT_FALSE(renderer->isInitialized());
}

/**
 * @test Тест повторной инициализации
 */
TEST_F(ModernRenderer3DTest, InitializeTwiceTest) {
    // Arrange
    renderer = std::make_unique<ModernRenderer3D>(
        mockStrategy, mockLighting, mockCamera, mockStats, mockLogger
    );
    renderer->initialize();

    // Act
    bool result = renderer->initialize();

    // Assert
    EXPECT_TRUE(result); // Should return true without re-initializing
}

/**
 * @test Тест shutdown()
 */
TEST_F(ModernRenderer3DTest, ShutdownTest) {
    // Arrange
    renderer = std::make_unique<ModernRenderer3D>(
        mockStrategy, mockLighting, mockCamera, mockStats, mockLogger
    );
    renderer->initialize();

    // Act
    renderer->shutdown();

    // Assert
    EXPECT_FALSE(renderer->isInitialized());
    EXPECT_FALSE(renderer->isReady());
}

/**
 * @test Тест shutdown() без инициализации
 */
TEST_F(ModernRenderer3DTest, ShutdownWithoutInitTest) {
    // Arrange
    renderer = std::make_unique<ModernRenderer3D>(
        mockStrategy, mockLighting, mockCamera, mockStats, mockLogger
    );

    // Act & Assert
    EXPECT_NO_THROW(renderer->shutdown());
}

// ============================================================================
// 2. RENDERING TESTS
// ============================================================================

/**
 * @test Тест renderFrame()
 */
TEST_F(ModernRenderer3DTest, RenderFrameTest) {
    // Arrange
    renderer = std::make_unique<ModernRenderer3D>(
        mockStrategy, mockLighting, mockCamera, mockStats, mockLogger
    );
    renderer->initialize();
    FrameData frameData;
    frameData.deltaTime = 0.016f;

    // Expect
    EXPECT_CALL(*mockStats, beginFrame()).Times(1);
    EXPECT_CALL(*mockCamera, updateCamera(_)).Times(1);
    EXPECT_CALL(*mockLighting, updateLighting(_)).Times(1);
    EXPECT_CALL(*mockStrategy, render(_)).Times(1);
    EXPECT_CALL(*mockStats, endFrame()).Times(1);

    // Act
    renderer->renderFrame(frameData);

    // Assert - verified by EXPECT_CALL
}

/**
 * @test Тест renderFrame() без инициализации
 */
TEST_F(ModernRenderer3DTest, RenderFrameWithoutInitTest) {
    // Arrange
    renderer = std::make_unique<ModernRenderer3D>(
        mockStrategy, mockLighting, mockCamera, mockStats, mockLogger
    );
    FrameData frameData;

    // Act & Assert
    EXPECT_NO_THROW(renderer->renderFrame(frameData));
}

// ============================================================================
// 3. QUERY TESTS
// ============================================================================

/**
 * @test Тест getType()
 */
TEST_F(ModernRenderer3DTest, GetTypeTest) {
    // Arrange
    renderer = std::make_unique<ModernRenderer3D>(
        mockStrategy, mockLighting, mockCamera, mockStats, mockLogger
    );

    // Act
    RendererType type = renderer->getType();

    // Assert
    EXPECT_EQ(type, RendererType::OpenGL); // По умолчанию
}

/**
 * @test Тест supportsFeature()
 */
TEST_F(ModernRenderer3DTest, SupportsFeatureTest) {
    // Arrange
    ON_CALL(*mockStrategy, supportsFeature(_)).WillByDefault(Return(true));
    renderer = std::make_unique<ModernRenderer3D>(
        mockStrategy, mockLighting, mockCamera, mockStats, mockLogger
    );

    // Act & Assert
    EXPECT_TRUE(renderer->supportsFeature(RenderingFeature::OpenGLCore));
}

/**
 * @test Тест getName()
 */
TEST_F(ModernRenderer3DTest, GetNameTest) {
    // Arrange
    renderer = std::make_unique<ModernRenderer3D>(
        mockStrategy, mockLighting, mockCamera, mockStats, mockLogger
    );

    // Act
    std::string name = renderer->getName();

    // Assert
    EXPECT_EQ(name, "ModernRenderer3D (MockStrategy)");
}

/**
 * @test Тест getApiVersion()
 */
TEST_F(ModernRenderer3DTest, GetApiVersionTest) {
    // Arrange
    renderer = std::make_unique<ModernRenderer3D>(
        mockStrategy, mockLighting, mockCamera, mockStats, mockLogger
    );

    // Act
    std::string version = renderer->getApiVersion();

    // Assert
    EXPECT_EQ(version, "OpenGL 4.6");
}

/**
 * @test Тест isReady()
 */
TEST_F(ModernRenderer3DTest, IsReadyTest) {
    // Arrange
    renderer = std::make_unique<ModernRenderer3D>(
        mockStrategy, mockLighting, mockCamera, mockStats, mockLogger
    );

    // Act & Assert - before init
    EXPECT_FALSE(renderer->isReady());

    renderer->initialize();

    // Act & Assert - after init
    EXPECT_TRUE(renderer->isReady());
}

/**
 * @test Тест getStats()
 */
TEST_F(ModernRenderer3DTest, GetStatsTest) {
    // Arrange
    RenderingStats expectedStats;
    expectedStats.fps = 60.0f;
    expectedStats.frameTime = 16.67f;
    ON_CALL(*mockStats, getStats()).WillByDefault(Return(expectedStats));
    
    renderer = std::make_unique<ModernRenderer3D>(
        mockStrategy, mockLighting, mockCamera, mockStats, mockLogger
    );

    // Act
    RenderingStats stats = renderer->getStats();

    // Assert
    EXPECT_FLOAT_EQ(stats.fps, 60.0f);
    EXPECT_FLOAT_EQ(stats.frameTime, 16.67f);
}

// ============================================================================
// 4. CONFIGURATION TESTS
// ============================================================================

/**
 * @test Тест setConfigParameter() и getConfigParameter()
 */
TEST_F(ModernRenderer3DTest, SetGetConfigParameterTest) {
    // Arrange
    renderer = std::make_unique<ModernRenderer3D>(
        mockStrategy, mockLighting, mockCamera, mockStats, mockLogger
    );

    // Act
    renderer->setConfigParameter("width", 1920);
    renderer->setConfigParameter("height", 1080);
    renderer->setConfigParameter("vsync", true);

    // Assert
    EXPECT_EQ(std::any_cast<int>(renderer->getConfigParameter("width")), 1920);
    EXPECT_EQ(std::any_cast<int>(renderer->getConfigParameter("height")), 1080);
    EXPECT_EQ(std::any_cast<bool>(renderer->getConfigParameter("vsync")), true);
}

/**
 * @test Тест getConfigParameter() для несуществующего параметра
 */
TEST_F(ModernRenderer3DTest, GetConfigParameterNotFoundTest) {
    // Arrange
    renderer = std::make_unique<ModernRenderer3D>(
        mockStrategy, mockLighting, mockCamera, mockStats, mockLogger
    );

    // Act & Assert
    EXPECT_THROW({
        renderer->getConfigParameter("nonexistent");
    }, std::runtime_error);
}

/**
 * @test Тест hasConfigParameter()
 */
TEST_F(ModernRenderer3DTest, HasConfigParameterTest) {
    // Arrange
    renderer = std::make_unique<ModernRenderer3D>(
        mockStrategy, mockLighting, mockCamera, mockStats, mockLogger
    );
    renderer->setConfigParameter("test", 42);

    // Act & Assert
    EXPECT_TRUE(renderer->hasConfigParameter("test"));
    EXPECT_FALSE(renderer->hasConfigParameter("nonexistent"));
}

/**
 * @test Тест loadConfig()
 */
TEST_F(ModernRenderer3DTest, LoadConfigTest) {
    // Arrange
    renderer = std::make_unique<ModernRenderer3D>(
        mockStrategy, mockLighting, mockCamera, mockStats, mockLogger
    );

    // Act
    bool result = renderer->loadConfig("config.json");

    // Assert
    EXPECT_TRUE(result);
}

/**
 * @test Тест saveConfig()
 */
TEST_F(ModernRenderer3DTest, SaveConfigTest) {
    // Arrange
    renderer = std::make_unique<ModernRenderer3D>(
        mockStrategy, mockLighting, mockCamera, mockStats, mockLogger
    );

    // Act
    bool result = renderer->saveConfig("config.json");

    // Assert
    EXPECT_TRUE(result);
}

// ============================================================================
// 5. STRATEGY TESTS
// ============================================================================

/**
 * @test Тест setRenderStrategy()
 */
TEST_F(ModernRenderer3DTest, SetRenderStrategyTest) {
    // Arrange
    renderer = std::make_unique<ModernRenderer3D>(
        mockStrategy, mockLighting, mockCamera, mockStats, mockLogger
    );
    renderer->initialize();

    auto newStrategy = std::make_shared<NiceMock<MockRenderStrategy>>();
    ON_CALL(*newStrategy, initialize()).WillByDefault(Return(true));
    ON_CALL(*newStrategy, getName()).WillByDefault(Return("NewStrategy"));

    // Expect old strategy shutdown
    EXPECT_CALL(*mockStrategy, shutdown()).Times(1);

    // Act
    renderer->setRenderStrategy(newStrategy);

    // Assert
    EXPECT_EQ(renderer->getName(), "ModernRenderer3D (NewStrategy)");
}

/**
 * @test Тест setRenderStrategy() с nullptr
 */
TEST_F(ModernRenderer3DTest, SetRenderStrategyNullTest) {
    // Arrange
    renderer = std::make_unique<ModernRenderer3D>(
        mockStrategy, mockLighting, mockCamera, mockStats, mockLogger
    );

    // Act & Assert
    EXPECT_NO_THROW(renderer->setRenderStrategy(nullptr));
    EXPECT_EQ(renderer->getName(), "ModernRenderer3D (MockStrategy)");
}

// ============================================================================
// 6. POST-PROCESS EFFECTS TESTS
// ============================================================================

/**
 * @test Тест addPostProcessEffect()
 */
TEST_F(ModernRenderer3DTest, AddPostProcessEffectTest) {
    // Arrange
    renderer = std::make_unique<ModernRenderer3D>(
        mockStrategy, mockLighting, mockCamera, mockStats, mockLogger
    );
    renderer->initialize();

    auto effect = std::make_shared<NiceMock<MockPostProcessEffect>>();
    ON_CALL(*effect, initialize()).WillByDefault(Return(true));
    ON_CALL(*effect, getName()).WillByDefault(Return("TestEffect"));

    // Expect initialization
    EXPECT_CALL(*effect, initialize()).Times(1);

    // Act
    renderer->addPostProcessEffect(effect);

    // Assert - verified by EXPECT_CALL
}

/**
 * @test Тест addPostProcessEffect() с nullptr
 */
TEST_F(ModernRenderer3DTest, AddPostProcessEffectNullTest) {
    // Arrange
    renderer = std::make_unique<ModernRenderer3D>(
        mockStrategy, mockLighting, mockCamera, mockStats, mockLogger
    );

    // Act & Assert
    EXPECT_NO_THROW(renderer->addPostProcessEffect(nullptr));
}

/**
 * @test Тест добавления дубликата эффекта
 */
TEST_F(ModernRenderer3DTest, AddPostProcessEffectDuplicateTest) {
    // Arrange
    renderer = std::make_unique<ModernRenderer3D>(
        mockStrategy, mockLighting, mockCamera, mockStats, mockLogger
    );
    renderer->initialize();

    auto effect = std::make_shared<NiceMock<MockPostProcessEffect>>();
    ON_CALL(*effect, initialize()).WillByDefault(Return(true));
    ON_CALL(*effect, getName()).WillByDefault(Return("TestEffect"));

    // Expect only one initialization
    EXPECT_CALL(*effect, initialize()).Times(1);

    // Act
    renderer->addPostProcessEffect(effect);
    renderer->addPostProcessEffect(effect); // Duplicate

    // Assert - verified by EXPECT_CALL
}

/**
 * @test Тест removePostProcessEffect()
 */
TEST_F(ModernRenderer3DTest, RemovePostProcessEffectTest) {
    // Arrange
    renderer = std::make_unique<ModernRenderer3D>(
        mockStrategy, mockLighting, mockCamera, mockStats, mockLogger
    );
    renderer->initialize();

    auto effect = std::make_shared<NiceMock<MockPostProcessEffect>>();
    ON_CALL(*effect, initialize()).WillByDefault(Return(true));
    ON_CALL(*effect, getName()).WillByDefault(Return("TestEffect"));
    
    renderer->addPostProcessEffect(effect);

    // Expect shutdown when removed
    EXPECT_CALL(*effect, shutdown()).Times(1);

    // Act
    renderer->removePostProcessEffect(effect);

    // Assert - verified by EXPECT_CALL
}

/**
 * @test Тест removePostProcessEffect() с nullptr
 */
TEST_F(ModernRenderer3DTest, RemovePostProcessEffectNullTest) {
    // Arrange
    renderer = std::make_unique<ModernRenderer3D>(
        mockStrategy, mockLighting, mockCamera, mockStats, mockLogger
    );

    // Act & Assert
    EXPECT_NO_THROW(renderer->removePostProcessEffect(nullptr));
}

// ============================================================================
// MAIN
// ============================================================================

int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
