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

#include "SpectraForge/App/Core/GameLoopManager.h"
#include "SpectraForge/App/Core/InputManager.h"
#include "SpectraForge/App/Core/SceneCoordinator.h"
#include "SpectraForge/App/Core/WindowManager.h"
#include "SpectraForge/Core/DependencyInjection/Container.h"
#include "SpectraForge/Core/EngineCore.h" // Contains ILogger interface
#include "SpectraForge/Core/Interfaces/IEngineCore.h"
#include "SpectraForge/Core/Logger.h"
#include "SpectraForge/Rendering/Camera3D.h"
#include "SpectraForge/Rendering/Common/IRenderer.h"
#include "SpectraForge/Rendering/Common/IResourceManager.h"
#include "SpectraForge/Rendering/Common/IWindowBinder.h"
#include "SpectraForge/Rendering/Core/FrameManager.h"
#include "SpectraForge/Rendering/Core/PipelineManager.h"
#include "SpectraForge/Rendering/Core/RendererCore.h"
#include "SpectraForge/Rendering/Core/RendererDebugger.h"
#include "SpectraForge/Rendering/Core/RendererStatistics.h"
#include "SpectraForge/Rendering/Core/SwapchainManager.h"
#include "SpectraForge/Rendering/HybridFreGSRenderer.h"
#include "SpectraForge/Vulkan/SceneManager.h"
#include "SpectraForge/Vulkan/Interfaces/ISceneManager.h"

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
        container.registerSingleton<SpectraForge::Core::ILogger, SpectraForge::Core::Logger>();

        container.registerSingleton<Core::IGameLoopManager>(
            [](SpectraForge::Core::DI::Container&) { return std::make_shared<Core::GameLoopManager>(); });
        container.registerSingleton<Core::IWindowManager>(
            [](SpectraForge::Core::DI::Container&) { return std::make_shared<Core::WindowManager>(); });
        container.registerSingleton<Core::IInputManager>(
            [](SpectraForge::Core::DI::Container&) { return std::make_shared<Core::InputManager>(); });
        container.registerSingleton<Core::ISceneCoordinator>(
            [](SpectraForge::Core::DI::Container&) { return std::make_shared<Core::SceneCoordinator>(); });
        container.registerSingleton<Vulkan::ISceneManager>(
            [](SpectraForge::Core::DI::Container&) { return std::make_shared<Vulkan::SceneManager>(); });

        container.registerSingleton<SpectraForge::Core::IEngineCore>(
            [](SpectraForge::Core::DI::Container& c) {
                auto renderer = c.resolve<Rendering::IRenderer>();
                auto resources = c.resolve<Rendering::IResourceManager>();
                auto logger = c.resolve<SpectraForge::Core::ILogger>();
                return std::make_shared<SpectraForge::Core::EngineCore>(renderer, resources, logger);
            });
    }

    /**
     * @brief Register rendering services
     */
    static void registerRenderingServices(SpectraForge::Core::DI::Container& container) {
        container.registerSingleton<Rendering::Core::IRendererCore>(
            [](SpectraForge::Core::DI::Container&) { return std::make_shared<Rendering::Core::RendererCore>(); });
        container.registerSingleton<Rendering::Core::IRendererDebugger>(
            [](SpectraForge::Core::DI::Container&) { return std::make_shared<Rendering::Core::RendererDebugger>(); });
        container.registerSingleton<Rendering::Core::ISwapchainManagerFactory>(
            [](SpectraForge::Core::DI::Container&) { return std::make_shared<Rendering::Core::SwapchainManagerFactory>(); });
        container.registerSingleton<Rendering::Core::IPipelineManagerFactory>(
            [](SpectraForge::Core::DI::Container&) { return std::make_shared<Rendering::Core::PipelineManagerFactory>(); });
        container.registerSingleton<Rendering::Core::IFrameManagerFactory>(
            [](SpectraForge::Core::DI::Container&) { return std::make_shared<Rendering::Core::FrameManagerFactory>(); });
        container.registerSingleton<Rendering::Core::IRendererStatisticsFactory>([](SpectraForge::Core::DI::Container&) {
            return std::make_shared<Rendering::Core::RendererStatisticsFactory>();
        });

        container.registerSingleton<Rendering::IRenderer>([](SpectraForge::Core::DI::Container& c) {
            auto renderer = std::make_shared<Rendering::HybridFreGSRenderer>(
                c.resolve<Rendering::Core::IRendererCore>(),
                c.resolve<Rendering::Core::IRendererDebugger>(),
                c.resolve<Rendering::Core::ISwapchainManagerFactory>(),
                c.resolve<Rendering::Core::IPipelineManagerFactory>(),
                c.resolve<Rendering::Core::IFrameManagerFactory>(),
                c.resolve<Rendering::Core::IRendererStatisticsFactory>());
            c.registerInstance<Rendering::IWindowBinder>(renderer);
            return renderer;
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
        // Placeholder resource manager until full implementation
        class NullResourceManager final : public Rendering::IResourceManager {
          public:
            bool initialize() override { return true; }
            void shutdown() override {}
            Rendering::BufferHandle createBuffer(const Rendering::BufferDesc&) override {
                return Rendering::INVALID_HANDLE;
            }
            void updateBuffer(Rendering::BufferHandle, const void*, size_t, size_t) override {}
            void readBuffer(Rendering::BufferHandle, void*, size_t, size_t) override {}
            Rendering::TextureHandle createTexture(const Rendering::TextureDesc&) override {
                return Rendering::INVALID_HANDLE;
            }
            void updateTexture(Rendering::TextureHandle, const void*, uint32_t, uint32_t) override {}
            Rendering::ShaderHandle createShader(const std::string&, Rendering::ShaderType) override {
                return Rendering::INVALID_HANDLE;
            }
            Rendering::ShaderHandle createShaderFromFile(const std::string&, Rendering::ShaderType) override {
                return Rendering::INVALID_HANDLE;
            }
            void releaseResource(Rendering::ResourceHandle) override {}
            void releaseAllResources() override {}
            bool isValid(Rendering::ResourceHandle) const override { return false; }
            size_t getResourceSize(Rendering::ResourceHandle) const override { return 0; }
            Rendering::MemoryStats getMemoryStats() const override { return {}; }
            void waitForCompletion() override {}
            void flush() override {}
        };

        container.registerSingleton<Rendering::IResourceManager>(
            [](SpectraForge::Core::DI::Container&) { return std::make_shared<NullResourceManager>(); });
    }
};

} // namespace App
} // namespace SpectraForge
