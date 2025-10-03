#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include "SpectraForge/App/Engine.h"
#include "SpectraForge/App/Config.h"
#include "SpectraForge/Core/Logger.h"
#include "SpectraForge/Rendering/Common/IRenderer.h"
#include "SpectraForge/Rendering/Common/IResourceManager.h"

using namespace SpectraForge;
using ::testing::Return;

namespace {

class MockRenderer : public Rendering::IRenderer {
  public:
    MOCK_METHOD(bool, initialize, (), (override));
    MOCK_METHOD(void, renderFrame, (const Rendering::FrameData&), (override));
    MOCK_METHOD(void, shutdown, (), (override));
    MOCK_METHOD(Rendering::RendererType, getType, (), (const, override));
    MOCK_METHOD(bool, supportsFeature, (Rendering::RenderingFeature), (const, override));
    MOCK_METHOD(std::string, getName, (), (const, override));
    MOCK_METHOD(std::string, getApiVersion, (), (const, override));
    MOCK_METHOD(bool, isReady, (), (const, override));
    MOCK_METHOD(bool, isInitialized, (), (const, override));
    MOCK_METHOD(void, beginFrame, (), (override));
    MOCK_METHOD(void, endFrame, (), (override));
    MOCK_METHOD(Rendering::RenderingStats, getStats, (), (const, override));
};

class MockResourceManager : public Rendering::IResourceManager {
  public:
    MOCK_METHOD(bool, initialize, (), (override));
    MOCK_METHOD(void, shutdown, (), (override));
    MOCK_METHOD(Rendering::BufferHandle, createBuffer, (const Rendering::BufferDesc&), (override));
    MOCK_METHOD(void, updateBuffer,
                (Rendering::BufferHandle, const void*, size_t, size_t), (override));
    MOCK_METHOD(void, readBuffer, (Rendering::BufferHandle, void*, size_t, size_t), (override));
    MOCK_METHOD(Rendering::TextureHandle, createTexture, (const Rendering::TextureDesc&), (override));
    MOCK_METHOD(void, updateTexture, (Rendering::TextureHandle, const void*, uint32_t, uint32_t), (override));
    MOCK_METHOD(Rendering::ShaderHandle, createShader, (const std::string&, Rendering::ShaderType), (override));
    MOCK_METHOD(Rendering::ShaderHandle, createShaderFromFile,
                (const std::string&, Rendering::ShaderType), (override));
    MOCK_METHOD(void, releaseResource, (Rendering::ResourceHandle), (override));
    MOCK_METHOD(void, releaseAllResources, (), (override));
    MOCK_METHOD(bool, isValid, (Rendering::ResourceHandle), (const, override));
    MOCK_METHOD(Rendering::MemoryStats, getMemoryStats, (), (const, override));
    MOCK_METHOD(size_t, getResourceSize, (Rendering::ResourceHandle), (const, override));
    MOCK_METHOD(void, waitForCompletion, (), (override));
    MOCK_METHOD(void, flush, (), (override));
};

TEST(AppEngine, InitRenderShutdown) {
    App::AppConfig cfg;
    cfg.window_title = "Test";

    auto logger = std::make_shared<Core::Logger>("", Core::LogLevel::ERROR_LEVEL);
    auto renderer = std::make_shared<MockRenderer>();
    auto resources = std::make_shared<MockResourceManager>();

    EXPECT_CALL(*renderer, initialize()).WillOnce(Return(true));
    EXPECT_CALL(*resources, initialize()).WillOnce(Return(true));
    EXPECT_CALL(*renderer, isInitialized()).WillRepeatedly(Return(true));
    EXPECT_CALL(*renderer, beginFrame()).Times(1);
    EXPECT_CALL(*renderer, endFrame()).Times(1);

    App::Engine app(cfg, logger, renderer, resources);
    ASSERT_TRUE(app.init());

    Vulkan::SceneData scene{};
    scene.scenePath = "dummy";
    EXPECT_TRUE(app.load_scene(scene));

    app.update(0.016f);
    app.render();
    app.shutdown();
}

}  // namespace


