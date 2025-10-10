/**
 * @file DISetup.h
 * @brief DI Container setup for SpectraForge Application (P0.7)
 * 
 * SOLID COMPLIANCE:
 * - DIP ✅: All dependencies registered through interfaces
 * - OCP ✅: Easily extensible registration
 * - SRP ✅: Single responsibility - configure DI
 */

#pragma once

#include "SpectraForge/Core/DependencyInjection/Container.h"
#include "SpectraForge/Core/EngineCore.h" // Contains ILogger interface
#include "SpectraForge/Core/Logger.h"
#include "SpectraForge/Rendering/Common/IRenderer.h"
#include "SpectraForge/Rendering/Common/IResourceManager.h"
#include "SpectraForge/Rendering/HybridFreGSRenderer.h"
#include "SpectraForge/Rendering/Camera3D.h"
#include "SpectraForge/App/Core/GameLoopManager.h"
#include "SpectraForge/App/Core/WindowManager.h"
#include "SpectraForge/App/Core/InputManager.h"
#include "SpectraForge/App/Core/SceneCoordinator.h"

namespace SpectraForge {
namespace App {

/**
 * @brief Setup Dependency Injection for SpectraForge
 * 
 * Registers all core services with appropriate lifetimes:
 * - Singleton: Logger, Renderer, ResourceManager
 * - Scoped: GameLoop, Window, Input, Scene components
 * - Transient: Temporary objects
 */
class DISetup {
public:
    /**
     * @brief Configure and register all dependencies
     * @param container DI container to configure
     */
    static void configure(SpectraForge::Core::DI::Container& container) {
        // P0.7: Core Services (Singleton)
        registerCoreServices(container);
        
        // P0.7: Rendering Services (Singleton)
        registerRenderingServices(container);
        
        // P0.7: Application Components (Scoped per Engine instance)
        registerApplicationComponents(container);
    }
    
    /**
     * @brief Configure with default implementations
     * Uses ServiceLocator global container
     */
    static void configureDefault() {
        auto& container = SpectraForge::Core::DI::ServiceLocator::getInstance();
        configure(container);
    }

private:
    /**
     * @brief Register core services (Logger, etc.)
     */
    static void registerCoreServices(SpectraForge::Core::DI::Container& container) {
        // Logger - Singleton (shared across entire application)
        container.registerSingleton<SpectraForge::Core::ILogger, SpectraForge::Core::Logger>();
    }
    
    /**
     * @brief Register rendering services
     */
    static void registerRenderingServices(SpectraForge::Core::DI::Container& container) {
        // Renderer - Singleton (one renderer instance)
        container.registerSingleton<Rendering::IRenderer>([](SpectraForge::Core::DI::Container&) {
            return std::make_shared<Rendering::HybridFreGSRenderer>();
        });
        
        // Camera3D - Transient (each Engine can have its own)
        container.registerTransient<Rendering::Camera3D>([](SpectraForge::Core::DI::Container&) {
            auto camera = std::make_shared<Rendering::Camera3D>();
            camera->setPerspective(60.0f, 16.0f / 9.0f, 0.1f, 1000.0f);
            return camera;
        });
    }
    
    /**
     * @brief Register application components (P0.3 components)
     */
    static void registerApplicationComponents(SpectraForge::Core::DI::Container& container) {
        // GameLoopManager - Scoped (one per Engine instance)
        container.registerScoped<Core::GameLoopManager>([](SpectraForge::Core::DI::Container&) {
            return std::make_shared<Core::GameLoopManager>();
        });
        
        // WindowManager - Scoped (one per Engine instance)
        container.registerScoped<Core::WindowManager>([](SpectraForge::Core::DI::Container&) {
            return std::make_shared<Core::WindowManager>();
        });
        
        // InputManager - Scoped (one per Engine instance)
        container.registerScoped<Core::InputManager>([](SpectraForge::Core::DI::Container&) {
            return std::make_shared<Core::InputManager>();
        });
        
        // SceneCoordinator - Scoped (one per Engine instance)
        container.registerScoped<Core::SceneCoordinator>([](SpectraForge::Core::DI::Container&) {
            return std::make_shared<Core::SceneCoordinator>();
        });
    }
};

} // namespace App
} // namespace SpectraForge

