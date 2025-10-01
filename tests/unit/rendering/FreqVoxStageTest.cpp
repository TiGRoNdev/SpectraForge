#include <gtest/gtest.h>
#include "HyperEngine/Rendering/FreqVox/FreqVoxRenderStage.h"

using namespace HyperEngine::Rendering;
using namespace HyperEngine::Rendering::FreqVox;

namespace {

class DummyResourceManager : public IResourceManager {
  public:
    ~DummyResourceManager() override = default;
    bool initialize() override { return true; }
    void shutdown() override {}
    BufferHandle createBuffer(size_t, BufferUsage, MemoryUsage) override { return INVALID_HANDLE; }
    void destroyBuffer(BufferHandle) override {}
    TextureHandle createTexture2D(int, int, TextureFormat, TextureUsage) override { return INVALID_HANDLE; }
    void destroyTexture(TextureHandle) override {}
};

TEST(FreqVoxStage, BasicInitExecuteShutdown) {
    auto rm = std::make_shared<DummyResourceManager>();
    FreqVoxRenderStage stage;
    EXPECT_TRUE(stage.initialize(rm));
    RenderContext ctx{};
    ctx.resourceManager = rm;
    stage.execute(ctx);
    EXPECT_TRUE(stage.isReady());
    EXPECT_EQ(stage.getName(), std::string("FreqVoxRenderStage"));
    stage.shutdown();
}

}  // namespace


