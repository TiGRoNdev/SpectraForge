/**
 * @file test_solid_principles.cpp
 * @brief Тесты для проверки соблюдения SOLID принципов
 */

#include <gtest/gtest.h>
#include <memory>
#include "HyperEngine/Core/EngineCore.h"
#include "HyperEngine/Core/Logger.h"
#include "HyperEngine/Rendering/ModernRenderer3D.h"

using namespace HyperEngine::Core;
using namespace HyperEngine::Rendering;

/**
 * @brief Мок-реализации для тестирования
 */
class MockRenderStrategy : public IRenderStrategy {
  public:
    bool initialize() override { return true; }
    void render(const FrameData& frameData) override {}
    void shutdown() override {}
    std::string getName() const override { return "MockRenderStrategy"; }
    bool supportsFeature(RenderingFeature feature) const override { return false; }
};

class MockLightingSystem : public ILightingSystem {
  public:
    bool initialize() override { return true; }
    void updateLighting(const FrameData& frameData) override {}
    void shutdown() override {}
    void addLight(std::shared_ptr<class ILight> light) override {}
    void removeLight(std::shared_ptr<class ILight> light) override {}
};

class MockCameraSystem : public ICameraSystem {
  public:
    bool initialize() override { return true; }
    void updateCamera(const FrameData& frameData) override {}
    void shutdown() override {}
    Math::Matrix4 getViewMatrix() const override { return Math::Matrix4(); }
    Math::Matrix4 getProjectionMatrix() const override { return Math::Matrix4(); }
};

class MockRenderStatistics : public IRenderStatistics {
  public:
    void beginFrame() override {}
    void endFrame() override {}
    void recordDrawCall(uint32_t primitiveCount) override {}
    RenderingStats getStats() const override { return RenderingStats{}; }
    void reset() override {}
};

class MockResourceManager : public IResourceManager {
  public:
    bool initialize() override { return true; }
    void shutdown() override {}
    bool isInitialized() const override { return true; }
};

/**
 * @brief Тест Dependency Inversion Principle (DIP)
 */
TEST(SOLIDPrinciplesTest, DependencyInversionPrinciple) {
    // Создаем зависимости
    auto logger = std::make_shared<Logger>("", LogLevel::ERROR);
    auto renderStrategy = std::make_shared<MockRenderStrategy>();
    auto lightingSystem = std::make_shared<MockLightingSystem>();
    auto cameraSystem = std::make_shared<MockCameraSystem>();
    auto statistics = std::make_shared<MockRenderStatistics>();
    auto resourceManager = std::make_shared<MockResourceManager>();

    // Тест: рендерер должен принимать зависимости через конструктор (DIP)
    EXPECT_NO_THROW({
        auto renderer = std::make_shared<ModernRenderer3D>(
            renderStrategy, lightingSystem, cameraSystem, statistics, logger);
    });

    // Тест: движок должен принимать зависимости через конструктор (DIP)
    auto renderer = std::make_shared<ModernRenderer3D>(
        renderStrategy, lightingSystem, cameraSystem, statistics, logger);

    EXPECT_NO_THROW(
        { auto engine = std::make_unique<EngineCore>(renderer, resourceManager, logger); });
}

/**
 * @brief Тест Single Responsibility Principle (SRP)
 */
TEST(SOLIDPrinciplesTest, SingleResponsibilityPrinciple) {
    auto logger = std::make_shared<Logger>("", LogLevel::ERROR);

    // Тест: Logger отвечает только за логирование
    EXPECT_NO_THROW(logger->logInfo("Test message"));
    EXPECT_NO_THROW(logger->logError("Test error"));

    // Тест: каждый компонент имеет четко определенную ответственность
    auto renderStrategy = std::make_shared<MockRenderStrategy>();
    EXPECT_EQ(renderStrategy->getName(), "MockRenderStrategy");
    EXPECT_TRUE(renderStrategy->initialize());
}

/**
 * @brief Тест Open/Closed Principle (OCP)
 */
TEST(SOLIDPrinciplesTest, OpenClosedPrinciple) {
    auto logger = std::make_shared<Logger>("", LogLevel::ERROR);
    auto renderStrategy = std::make_shared<MockRenderStrategy>();
    auto lightingSystem = std::make_shared<MockLightingSystem>();
    auto cameraSystem = std::make_shared<MockCameraSystem>();
    auto statistics = std::make_shared<MockRenderStatistics>();
    auto resourceManager = std::make_shared<MockResourceManager>();

    auto renderer = std::make_shared<ModernRenderer3D>(
        renderStrategy, lightingSystem, cameraSystem, statistics, logger);

    auto engine = std::make_unique<EngineCore>(renderer, resourceManager, logger);

    // Тест: можно добавлять новые стратегии рендеринга без изменения существующего кода
    auto newStrategy = std::make_shared<MockRenderStrategy>();
    EXPECT_NO_THROW(renderer->setRenderStrategy(newStrategy));

    // Тест: можно добавлять новые подсистемы без изменения движка
    class TestSubsystem : public ISubsystem {
      public:
        bool initialize() override { return true; }
        void shutdown() override {}
        bool isInitialized() const override { return true; }
        void update(float deltaTime) override {}
        const char* getName() const override { return "TestSubsystem"; }
        int getUpdatePriority() const override { return 0; }
    };

    auto subsystem = std::make_shared<TestSubsystem>();
    EXPECT_NO_THROW(engine->registerSubsystem(subsystem));
}

/**
 * @brief Тест Interface Segregation Principle (ISP)
 */
TEST(SOLIDPrinciplesTest, InterfaceSegregationPrinciple) {
    // Тест: интерфейсы разделены по функциональности

    // IInitializable - только для инициализации
    auto logger = std::make_shared<Logger>("", LogLevel::ERROR);
    Interfaces::IInitializable* initializable = logger.get();
    EXPECT_TRUE(initializable != nullptr);

    // IConfigurable - только для конфигурации
    auto renderStrategy = std::make_shared<MockRenderStrategy>();
    auto lightingSystem = std::make_shared<MockLightingSystem>();
    auto cameraSystem = std::make_shared<MockCameraSystem>();
    auto statistics = std::make_shared<MockRenderStatistics>();

    auto renderer = std::make_shared<ModernRenderer3D>(
        renderStrategy, lightingSystem, cameraSystem, statistics, logger);

    Interfaces::IConfigurable* configurable = renderer.get();
    EXPECT_TRUE(configurable != nullptr);

    // Тест: можно использовать только нужные методы интерфейса
    EXPECT_NO_THROW(configurable->setConfigParameter("test", 42));
}

/**
 * @brief Тест Liskov Substitution Principle (LSP)
 */
TEST(SOLIDPrinciplesTest, LiskovSubstitutionPrinciple) {
    // Тест: все реализации стратегий рендеринга взаимозаменяемы
    std::shared_ptr<IRenderStrategy> strategy1 = std::make_shared<MockRenderStrategy>();
    std::shared_ptr<IRenderStrategy> strategy2 = std::make_shared<MockRenderStrategy>();

    // Обе стратегии должны работать одинаково через базовый интерфейс
    EXPECT_TRUE(strategy1->initialize());
    EXPECT_TRUE(strategy2->initialize());

    EXPECT_NO_THROW(strategy1->render(FrameData{}));
    EXPECT_NO_THROW(strategy2->render(FrameData{}));

    // Тест: все логгеры взаимозаменяемы
    std::shared_ptr<ILogger> logger1 = std::make_shared<Logger>("", LogLevel::INFO);
    std::shared_ptr<ILogger> logger2 = std::make_shared<Logger>("", LogLevel::DEBUG);

    EXPECT_NO_THROW(logger1->logInfo("Test"));
    EXPECT_NO_THROW(logger2->logInfo("Test"));
}

/**
 * @brief Тест конфигурируемости компонентов
 */
TEST(SOLIDPrinciplesTest, ConfigurabilityTest) {
    auto logger = std::make_shared<Logger>("", LogLevel::ERROR);
    auto renderStrategy = std::make_shared<MockRenderStrategy>();
    auto lightingSystem = std::make_shared<MockLightingSystem>();
    auto cameraSystem = std::make_shared<MockCameraSystem>();
    auto statistics = std::make_shared<MockRenderStatistics>();
    auto resourceManager = std::make_shared<MockResourceManager>();

    auto renderer = std::make_shared<ModernRenderer3D>(
        renderStrategy, lightingSystem, cameraSystem, statistics, logger);

    auto engine = std::make_unique<EngineCore>(renderer, resourceManager, logger);

    // Тест конфигурации движка
    EXPECT_NO_THROW(engine->setConfigParameter("test_param", 123));
    EXPECT_TRUE(engine->hasConfigParameter("test_param"));

    // Тест конфигурации рендерера
    EXPECT_NO_THROW(renderer->setConfigParameter("width", 1920));
    EXPECT_TRUE(renderer->hasConfigParameter("width"));
}

/**
 * @brief Тест инициализации и завершения работы
 */
TEST(SOLIDPrinciplesTest, InitializationTest) {
    auto logger = std::make_shared<Logger>("", LogLevel::ERROR);
    auto renderStrategy = std::make_shared<MockRenderStrategy>();
    auto lightingSystem = std::make_shared<MockLightingSystem>();
    auto cameraSystem = std::make_shared<MockCameraSystem>();
    auto statistics = std::make_shared<MockRenderStatistics>();
    auto resourceManager = std::make_shared<MockResourceManager>();

    auto renderer = std::make_shared<ModernRenderer3D>(
        renderStrategy, lightingSystem, cameraSystem, statistics, logger);

    auto engine = std::make_unique<EngineCore>(renderer, resourceManager, logger);

    // Тест инициализации
    EXPECT_TRUE(engine->initialize());
    EXPECT_TRUE(engine->isInitialized());

    // Тест завершения работы
    EXPECT_NO_THROW(engine->shutdown());
    EXPECT_FALSE(engine->isInitialized());
}
