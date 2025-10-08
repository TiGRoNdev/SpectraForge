#include <SpectraForge/Rendering/RenderPass/TriangleSplattingPass.h>
#include <SpectraForge/Rendering/RenderPass/TriangleSplatting/TriangleSplattingCore.h>
#include <SpectraForge/Rendering/RenderPass/TriangleSplatting/TriangleBufferManager.h>
#include <SpectraForge/Rendering/RenderPass/TriangleSplatting/FrustumCullingPass.h>
#include <SpectraForge/Rendering/RenderPass/TriangleSplatting/DepthSortingPass.h>
#include <SpectraForge/Rendering/RenderPass/TriangleSplatting/TriangleRasterizationPass.h>
#include <SpectraForge/Rendering/RenderPass/TriangleSplatting/TriangleSplattingDebugger.h>
#include <SpectraForge/Rendering/RenderPass/TriangleSplatting/TriangleSplattingStatistics.h>
#include <SpectraForge/Core/Console.h>
#include <stdexcept>

namespace spectraforge {
namespace rendering {

TriangleSplattingPass::TriangleSplattingPass(const Config& config)
    : config_(config) {
    // Constructor only stores config, actual initialization happens in initialize()
    Console::info("TriangleSplattingPass created with config");
}

TriangleSplattingPass::~TriangleSplattingPass() {
    cleanup();
}

bool TriangleSplattingPass::initialize(vk::Device device,
                                       VmaAllocator allocator,
                                       vk::Queue computeQueue,
                                       vk::Queue graphicsQueue,
                                       vk::CommandPool commandPool) {
    if (initialized_) {
        Console::warn("TriangleSplattingPass already initialized");
        return true;
    }
    
    if (!device || !allocator) {
        Console::error("Invalid Vulkan device or allocator");
        return false;
    }
    
    device_ = device;
    allocator_ = allocator;
    computeQueue_ = computeQueue;
    graphicsQueue_ = graphicsQueue;
    commandPool_ = commandPool;
    
    // Initialize all subsystems
    if (!initializeSubsystems()) {
        Console::error("Failed to initialize subsystems");
        cleanup();
        return false;
    }
    
    initialized_ = true;
    Console::info("TriangleSplattingPass initialized successfully");
    
    return true;
}

bool TriangleSplattingPass::initializeSubsystems() {
    // 1. Create all subsystems
    core_ = std::make_unique<TriangleSplattingCore>();
    bufferManager_ = std::make_unique<TriangleBufferManager>();
    cullingPass_ = std::make_unique<FrustumCullingPass>();
    sortingPass_ = std::make_unique<DepthSortingPass>();
    rasterPass_ = std::make_unique<TriangleRasterizationPass>();
    debugger_ = std::make_unique<TriangleSplattingDebugger>();
    statistics_ = std::make_unique<TriangleSplattingStatistics>();
    
    // 2. Initialize Core (must be first)
    TriangleSplattingCore::Config coreConfig;
    coreConfig.outputWidth = config_.outputWidth;
    coreConfig.outputHeight = config_.outputHeight;
    
    if (!core_->initialize(device_, allocator_, coreConfig)) {
        Console::error("Failed to initialize TriangleSplattingCore");
        return false;
    }
    
    // 3. Initialize BufferManager
    if (!bufferManager_->initialize(device_, allocator_, config_.maxTriangles)) {
        Console::error("Failed to initialize TriangleBufferManager");
        return false;
    }
    
    // 4. Initialize FrustumCullingPass (зависит от BufferManager)
    if (config_.enableFrustumCulling) {
        if (!cullingPass_->initialize(device_, allocator_, *bufferManager_, commandPool_, computeQueue_)) {
            Console::error("Failed to initialize FrustumCullingPass");
            return false;
        }
    }
    
    // 5. Initialize DepthSortingPass (зависит от BufferManager)
    if (config_.enableDepthSorting) {
        if (!sortingPass_->initialize(device_, allocator_, *bufferManager_, commandPool_, computeQueue_)) {
            Console::error("Failed to initialize DepthSortingPass");
            return false;
        }
        
        sortingPass_->setSortMode(config_.sortMode);
    }
    
    // 6. Initialize TriangleRasterizationPass (зависит от Core и BufferManager)
    TriangleRasterizationPass::Config rasterConfig;
    rasterConfig.enableEarlyTermination = config_.enableEarlyTermination;
    rasterConfig.alphaThreshold = config_.alphaThreshold;
    rasterConfig.enableTwoPassRendering = config_.enableTwoPassRendering;
    
    if (!rasterPass_->initialize(device_, allocator_, *core_, *bufferManager_, rasterConfig)) {
        Console::error("Failed to initialize TriangleRasterizationPass");
        return false;
    }
    
    // 7. Initialize Statistics
    statistics_->reset();
    
    Console::info("All subsystems initialized successfully");
    
    return true;
}

void TriangleSplattingPass::cleanup() {
    if (!initialized_) {
        return;
    }
    
    // Cleanup in reverse order
    if (rasterPass_) rasterPass_->cleanup();
    if (sortingPass_) sortingPass_->cleanup();
    if (cullingPass_) cullingPass_->cleanup();
    if (bufferManager_) bufferManager_->cleanup();
    if (core_) core_->cleanup();
    
    // Reset subsystems
    statistics_.reset();
    debugger_.reset();
    rasterPass_.reset();
    sortingPass_.reset();
    cullingPass_.reset();
    bufferManager_.reset();
    core_.reset();
    
    initialized_ = false;
    
    Console::info("TriangleSplattingPass cleaned up");
}

void TriangleSplattingPass::execute(vk::CommandBuffer cmd, uint32_t frameIndex) {
    if (!initialized_) {
        throw std::runtime_error("TriangleSplattingPass not initialized");
    }
    
    const uint32_t triangleCount = bufferManager_->getTriangleCount();
    
    if (triangleCount == 0) {
        Console::warn("TriangleSplattingPass: No triangles to render");
        return;
    }
    
    // Step 1: Frustum Culling (optional)
    visibleTriangleCount_ = triangleCount;
    
    if (config_.enableFrustumCulling && frustumCullingEnabled_ && cullingPass_) {
        cullingPass_->execute(cmd, viewProj_, triangleCount);
        visibleTriangleCount_ = cullingPass_->getVisibleCount();
    }
    
    // Step 2: Depth Sorting (optional)
    if (config_.enableDepthSorting && sortingPass_ && visibleTriangleCount_ > 0) {
        sortingPass_->execute(cmd, cameraPos_, visibleTriangleCount_);
    }
    
    // Step 3: Rasterization
    if (rasterPass_ && visibleTriangleCount_ > 0) {
        rasterPass_->execute(cmd, viewProj_, visibleTriangleCount_);
    }
    
    // Step 4: Update statistics
    if (statistics_) {
        statistics_->update(triangleCount, visibleTriangleCount_);
    }
}

void TriangleSplattingPass::uploadTriangles(const std::vector<Triangle>& triangles) {
    if (!initialized_ || !bufferManager_) {
        throw std::runtime_error("TriangleSplattingPass not initialized");
    }
    
    // Allocate temporary command buffer для transfer
    vk::CommandBufferAllocateInfo allocInfo;
    allocInfo.level = vk::CommandBufferLevel::ePrimary;
    allocInfo.commandPool = commandPool_;
    allocInfo.commandBufferCount = 1;
    
    vk::CommandBuffer cmd = device_.allocateCommandBuffers(allocInfo)[0];
    
    vk::CommandBufferBeginInfo beginInfo;
    beginInfo.flags = vk::CommandBufferUsageFlagBits::eOneTimeSubmit;
    
    cmd.begin(beginInfo);
    
    bufferManager_->uploadTriangles(triangles, cmd, graphicsQueue_);
    
    cmd.end();
    
    // Submit
    vk::SubmitInfo submitInfo;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &cmd;
    
    graphicsQueue_.submit(1, &submitInfo, vk::Fence());
    graphicsQueue_.waitIdle();
    
    device_.freeCommandBuffers(commandPool_, 1, &cmd);
    
    Console::info("Uploaded " + std::to_string(triangles.size()) + " triangles");
}

void TriangleSplattingPass::setViewProjection(const glm::mat4& viewProj) {
    viewProj_ = viewProj;
}

void TriangleSplattingPass::setCameraPosition(const glm::vec3& cameraPos) {
    cameraPos_ = cameraPos;
}

void TriangleSplattingPass::setFrustumCullingEnabled(bool enabled) {
    frustumCullingEnabled_ = enabled;
    Console::info(std::string("Frustum culling ") + (enabled ? "enabled" : "disabled"));
}

void TriangleSplattingPass::setDebugMode(uint32_t mode) {
    if (debugger_) {
        debugger_->setDebugMode(mode);
    }
    
    if (rasterPass_) {
        rasterPass_->setDebugMode(mode);
    }
}

uint32_t TriangleSplattingPass::getTriangleCount() const {
    if (!bufferManager_) {
        return 0;
    }
    
    return bufferManager_->getTriangleCount();
}

uint32_t TriangleSplattingPass::getVisibleTriangleCount() const {
    return visibleTriangleCount_;
}

vk::Image TriangleSplattingPass::getOutputImage() const {
    if (!core_) {
        return vk::Image();
    }
    
    return core_->getOutputImage();
}

vk::ImageView TriangleSplattingPass::getOutputImageView() const {
    if (!core_) {
        return vk::ImageView();
    }
    
    return core_->getOutputImageView();
}

} // namespace rendering
} // namespace spectraforge

