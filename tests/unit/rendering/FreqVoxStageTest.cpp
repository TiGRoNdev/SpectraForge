#include <gtest/gtest.h>
#include "SpectraForge/Rendering/FreqVox/FreqVoxRenderStage.h"

using namespace SpectraForge::Rendering;
using namespace SpectraForge::Rendering::FreqVox;

namespace {

class DummyResourceManager : public IResourceManager {
  public:
    ~DummyResourceManager() override = default;
    bool initialize() override { return true; }
    void shutdown() override {}
    
    BufferHandle createBuffer([[maybe_unused]] const BufferDesc& desc) override { 
        return BufferHandle{0}; 
    }
    
    void updateBuffer([[maybe_unused]] BufferHandle handle,
                     [[maybe_unused]] const void* data,
                     [[maybe_unused]] size_t size,
                     [[maybe_unused]] size_t offset = 0) override {}
    
    void readBuffer([[maybe_unused]] BufferHandle handle, 
                   [[maybe_unused]] void* data, 
                   [[maybe_unused]] size_t size, 
                   [[maybe_unused]] size_t offset = 0) override {}
    
    TextureHandle createTexture([[maybe_unused]] const TextureDesc& desc) override { 
        return TextureHandle{0}; 
    }
    
    void updateTexture([[maybe_unused]] TextureHandle handle,
                      [[maybe_unused]] const void* data,
                      [[maybe_unused]] uint32_t mipLevel = 0,
                      [[maybe_unused]] uint32_t arrayLayer = 0) override {}
    
    ShaderHandle createShader([[maybe_unused]] const std::string& source, 
                             [[maybe_unused]] ShaderType type) override { 
        return ShaderHandle{0}; 
    }
    
    ShaderHandle createShaderFromFile([[maybe_unused]] const std::string& filePath, 
                                     [[maybe_unused]] ShaderType type) override { 
        return ShaderHandle{0}; 
    }
    
    void releaseResource([[maybe_unused]] ResourceHandle handle) override {}
    void releaseAllResources() override {}
    
    bool isValid([[maybe_unused]] ResourceHandle handle) const override { 
        return true; 
    }
    
    size_t getResourceSize([[maybe_unused]] ResourceHandle handle) const override { 
        return 0; 
    }
    
    MemoryStats getMemoryStats() const override { 
        return MemoryStats{}; 
    }
    
    void waitForCompletion() override {}
    void flush() override {}
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


