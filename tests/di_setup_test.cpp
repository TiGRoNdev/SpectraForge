/**
 * @file di_setup_test.cpp
 * @brief Tests for DI Setup (P0.7)
 * 
 * Test Coverage:
 * - DI container registration
 * - Service resolution
 * - Lifetime management (Singleton, Scoped, Transient)
 * - Component integration
 */

#include <gtest/gtest.h>
#include "SpectraForge/App/DISetup.h"
#include "SpectraForge/Core/DependencyInjection/Container.h"
#include "SpectraForge/Core/EngineCore.h" // Contains ILogger interface
#include "SpectraForge/Rendering/Common/IRenderer.h"
#include "SpectraForge/Rendering/Camera3D.h"
#include "SpectraForge/App/Core/GameLoopManager.h"
#include "SpectraForge/App/Core/WindowManager.h"
#include "SpectraForge/App/Core/InputManager.h"
#include "SpectraForge/App/Core/SceneCoordinator.h"

using namespace SpectraForge;
using namespace SpectraForge::App;
using namespace SpectraForge::Core;

class DISetupTest : public ::testing::Test {
protected:
    void SetUp() override {
        container = std::make_unique<DI::Container>();
    }
    
    void TearDown() override {
        container.reset();
    }
    
    std::unique_ptr<DI::Container> container;
};

// ============================================================================
// Core Services Tests
// ============================================================================

TEST_F(DISetupTest, ConfigureRegistersAllDependencies) {
    // Arrange & Act
    DISetup::configure(*container);
    
    // Assert - Check all core registrations
    EXPECT_TRUE(container->isRegistered<SpectraForge::Core::ILogger>());
    EXPECT_TRUE(container->isRegistered<Rendering::IRenderer>());
    EXPECT_TRUE(container->isRegistered<Rendering::Camera3D>());
    EXPECT_TRUE(container->isRegistered<SpectraForge::App::Core::GameLoopManager>());
    EXPECT_TRUE(container->isRegistered<SpectraForge::App::Core::WindowManager>());
    EXPECT_TRUE(container->isRegistered<SpectraForge::App::Core::InputManager>());
    EXPECT_TRUE(container->isRegistered<SpectraForge::App::Core::SceneCoordinator>());
}

TEST_F(DISetupTest, LoggerIsSingleton) {
    // Arrange
    DISetup::configure(*container);
    
    // Act
    auto logger1 = container->resolve<SpectraForge::Core::ILogger>();
    auto logger2 = container->resolve<SpectraForge::Core::ILogger>();
    
    // Assert - Same instance
    EXPECT_EQ(logger1.get(), logger2.get());
}

TEST_F(DISetupTest, RendererIsSingleton) {
    // Arrange
    DISetup::configure(*container);
    
    // Act
    auto renderer1 = container->resolve<Rendering::IRenderer>();
    auto renderer2 = container->resolve<Rendering::IRenderer>();
    
    // Assert - Same instance
    EXPECT_EQ(renderer1.get(), renderer2.get());
}

TEST_F(DISetupTest, CameraIsTransient) {
    // Arrange
    DISetup::configure(*container);
    
    // Act
    auto camera1 = container->resolve<Rendering::Camera3D>();
    auto camera2 = container->resolve<Rendering::Camera3D>();
    
    // Assert - Different instances
    EXPECT_NE(camera1.get(), camera2.get());
}

// ============================================================================
// Application Components Tests (P0.3)
// ============================================================================

TEST_F(DISetupTest, GameLoopManagerIsScoped) {
    // Arrange
    DISetup::configure(*container);
    container->beginScope("engine1");
    
    // Act
    auto glm1 = container->resolve<SpectraForge::App::Core::GameLoopManager>();
    auto glm2 = container->resolve<SpectraForge::App::Core::GameLoopManager>();
    
    // Assert - Same instance within scope
    EXPECT_EQ(glm1.get(), glm2.get());
    
    // New scope -> different instance
    container->endScope();
    container->beginScope("engine2");
    auto glm3 = container->resolve<SpectraForge::App::Core::GameLoopManager>();
    EXPECT_NE(glm1.get(), glm3.get());
}

TEST_F(DISetupTest, WindowManagerIsScoped) {
    // Arrange
    DISetup::configure(*container);
    container->beginScope("engine1");
    
    // Act
    auto wm1 = container->resolve<SpectraForge::App::Core::WindowManager>();
    auto wm2 = container->resolve<SpectraForge::App::Core::WindowManager>();
    
    // Assert - Same instance within scope
    EXPECT_EQ(wm1.get(), wm2.get());
}

TEST_F(DISetupTest, InputManagerIsScoped) {
    // Arrange
    DISetup::configure(*container);
    container->beginScope("engine1");
    
    // Act
    auto im1 = container->resolve<SpectraForge::App::Core::InputManager>();
    auto im2 = container->resolve<SpectraForge::App::Core::InputManager>();
    
    // Assert - Same instance within scope
    EXPECT_EQ(im1.get(), im2.get());
}

TEST_F(DISetupTest, SceneCoordinatorIsScoped) {
    // Arrange
    DISetup::configure(*container);
    container->beginScope("engine1");
    
    // Act
    auto sc1 = container->resolve<SpectraForge::App::Core::SceneCoordinator>();
    auto sc2 = container->resolve<SpectraForge::App::Core::SceneCoordinator>();
    
    // Assert - Same instance within scope
    EXPECT_EQ(sc1.get(), sc2.get());
}

// ============================================================================
// Integration Tests
// ============================================================================

TEST_F(DISetupTest, CanResolveAllComponentsTogether) {
    // Arrange
    DISetup::configure(*container);
    container->beginScope("test");
    
    // Act - Resolve all components
    auto logger = container->resolve<SpectraForge::Core::ILogger>();
    auto renderer = container->resolve<Rendering::IRenderer>();
    auto camera = container->resolve<Rendering::Camera3D>();
    auto gameLoop = container->resolve<SpectraForge::App::Core::GameLoopManager>();
    auto window = container->resolve<SpectraForge::App::Core::WindowManager>();
    auto input = container->resolve<SpectraForge::App::Core::InputManager>();
    auto scene = container->resolve<SpectraForge::App::Core::SceneCoordinator>();
    
    // Assert - All non-null
    EXPECT_NE(logger, nullptr);
    EXPECT_NE(renderer, nullptr);
    EXPECT_NE(camera, nullptr);
    EXPECT_NE(gameLoop, nullptr);
    EXPECT_NE(window, nullptr);
    EXPECT_NE(input, nullptr);
    EXPECT_NE(scene, nullptr);
}

TEST_F(DISetupTest, ConfigureDefaultUsesServiceLocator) {
    // Arrange
    DI::ServiceLocator::reset();
    
    // Act
    DISetup::configureDefault();
    
    // Assert
    auto& globalContainer = DI::ServiceLocator::getInstance();
    EXPECT_TRUE(globalContainer.isRegistered<SpectraForge::Core::ILogger>());
    EXPECT_TRUE(globalContainer.isRegistered<Rendering::IRenderer>());
    
    // Cleanup
    DI::ServiceLocator::reset();
}

TEST_F(DISetupTest, ServiceLocatorGetReturnsValidLogger) {
    // Arrange
    DISetup::configureDefault();
    
    // Act
    auto logger = DI::ServiceLocator::get<SpectraForge::Core::ILogger>();
    
    // Assert
    EXPECT_NE(logger, nullptr);
    
    // Cleanup
    DI::ServiceLocator::reset();
}

TEST_F(DISetupTest, MultipleEngineInstancesGetSeparateComponents) {
    // Arrange
    DISetup::configure(*container);
    
    // Act - Engine 1
    container->beginScope("engine1");
    auto glm1 = container->resolve<SpectraForge::App::Core::GameLoopManager>();
    container->endScope();
    
    // Act - Engine 2
    container->beginScope("engine2");
    auto glm2 = container->resolve<SpectraForge::App::Core::GameLoopManager>();
    container->endScope();
    
    // Assert - Different instances for different scopes
    EXPECT_NE(glm1.get(), glm2.get());
}

TEST_F(DISetupTest, CameraHasCorrectDefaultSettings) {
    // Arrange
    DISetup::configure(*container);
    
    // Act
    auto camera = container->resolve<Rendering::Camera3D>();
    
    // Assert - Camera configured with defaults
    EXPECT_FLOAT_EQ(camera->getFieldOfView(), 60.0f);
    EXPECT_FLOAT_EQ(camera->getAspectRatio(), 16.0f / 9.0f);
    EXPECT_FLOAT_EQ(camera->getNearPlane(), 0.1f);
    EXPECT_FLOAT_EQ(camera->getFarPlane(), 1000.0f);
}

// ============================================================================
// Error Handling Tests
// ============================================================================

TEST_F(DISetupTest, ResolvingUnregisteredTypeThrows) {
    // Arrange - Empty container
    
    // Act & Assert
    EXPECT_THROW(container->resolve<SpectraForge::Core::ILogger>(), std::runtime_error);
}

TEST_F(DISetupTest, CanReregisterServices) {
    // Arrange
    DISetup::configure(*container);
    auto logger1 = container->resolve<SpectraForge::Core::ILogger>();
    
    // Act - Clear and reconfigure
    container->clear();
    DISetup::configure(*container);
    auto logger2 = container->resolve<SpectraForge::Core::ILogger>();
    
    // Assert - New instance after clear
    EXPECT_NE(logger1.get(), logger2.get());
}

