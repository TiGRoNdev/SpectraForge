#pragma once

#include <vulkan/vulkan.hpp>
#include <vk_mem_alloc.h>
#include <glm/glm.hpp>
#include <vector>
#include <memory>

namespace spectraforge {
namespace rendering {

/**
 * @brief Triangle Splatting Pass
 * 
 * Based on "Triangle Splatting for Real-Time Radiance Field Rendering" (2025)
 * https://trianglesplatting.github.io/
 * 
 * Renders triangles using signed distance fields (SDF) and smooth window functions
 * to achieve filled surfaces with sharp edges, preserving mesh connectivity.
 */
class TriangleSplattingPass final {
public:
    /**
     * @brief Triangle primitive with learnable parameters
     */
    struct Triangle {
        // Vertices (3D world space)
        glm::vec3 v0, v1, v2;

        // Texture coordinates for each vertex
        glm::vec2 texCoord0, texCoord1, texCoord2;

        // Appearance
        glm::vec3 color;       // RGB color (learnable)
        float opacity;         // Alpha (learnable)

        // Rendering parameters
        float sigma;           // Smoothness parameter for window function

        // Lighting
        glm::vec3 normal;      // Face normal for lighting

        // Material properties
        int materialId;        // Material index for texture lookup
        float padding[2];      // Align to 96 bytes

        Triangle()
            : v0(0.0f), v1(0.0f), v2(0.0f)
            , texCoord0(0.0f), texCoord1(0.0f), texCoord2(0.0f)
            , color(0.8f, 0.7f, 0.6f)
            , opacity(1.0f)
            , sigma(1.0f)
            , normal(0.0f, 1.0f, 0.0f)  // Default up normal
            , materialId(0)
            , padding{0.0f, 0.0f}
        {}
    };
    
    static_assert(sizeof(Triangle) == 104, "Triangle must be 104 bytes for alignment (3 vec3 vertices + 3 vec2 texCoords + vec3 color + float opacity + float sigma + vec3 normal + int materialId + padding)");

public:
    /**
     * @brief Configuration for Triangle Splatting Pass
     */
    struct Config {
        uint32_t outputWidth;
        uint32_t outputHeight;
        bool enableDepthSort;
        bool enableEarlyTermination;
        float alphaThreshold;
        bool enableTwoPassRendering;  // NEW: Two-Pass Rendering (O(N+M) instead of O(N×M))
        uint32_t maxTrianglesPerPixel; // NEW: Max triangles per pixel (for visibility buffer)
        
        Config() 
            : outputWidth(1920)
            , outputHeight(1080)
            , enableDepthSort(true)
            , enableEarlyTermination(true)
            , alphaThreshold(0.99f)
            , enableTwoPassRendering(false)  // 🐛 TEMPORARY DEBUG: Disable Two-Pass to test Single-Pass
            , maxTrianglesPerPixel(64)      // Typical coverage: 5-20 triangles/pixel
        {}
    };

    explicit TriangleSplattingPass(const Config& config = Config{});
    ~TriangleSplattingPass();

    // Disable copy/move
    TriangleSplattingPass(const TriangleSplattingPass&) = delete;
    TriangleSplattingPass& operator=(const TriangleSplattingPass&) = delete;

    /**
     * @brief Initialize the pass with Vulkan device and allocator
     * @param device Vulkan logical device
     * @param allocator VMA memory allocator
     * @param computeQueue Compute queue for command submission
     * @param commandPool Command pool for temporary command buffers
     */
        bool initialize(vk::Device device, 
                        VmaAllocator allocator,
                        vk::Queue computeQueue,
                        vk::Queue graphicsQueue,
                        vk::CommandPool commandPool);
    
    /**
     * @brief Execute the triangle splatting pass
     */
    void execute(vk::CommandBuffer cmd, uint32_t frameIndex);
    
    /**
     * @brief Cleanup resources
     */
    void cleanup();

    /**
     * @brief Upload triangles to GPU
     * @param triangles Vector of triangles to render
     */
    void uploadTriangles(const std::vector<Triangle>& triangles);
    
    /**
     * @brief Set view-projection matrix for 3D → 2D projection
     */
    void setViewProjection(const glm::mat4& viewProj);
    
    /**
     * @brief Set camera position for depth sorting
     */
    void setCameraPosition(const glm::vec3& cameraPos);
    
    /**
     * @brief Enable/disable frustum culling
     */
    void setFrustumCullingEnabled(bool enabled);
    
    /**
     * @brief Get visible triangle count after culling (read from GPU)
     */
    uint32_t getVisibleTriangleCount() const;
    
    /**
     * @brief Get total triangle count
     */
    uint32_t getTriangleCount() const { return triangleCount_; }
    
    /**
     * @brief Get output image (result of triangle splatting)
     */
    vk::Image getOutputImage() const { return outputImage_; }
    vk::ImageView getOutputImageView() const { return outputImageView_; }

    /**
     * @brief Check if pass is initialized
     */
    bool isInitialized() const { return initialized_; }

private:
    // Configuration
    Config config_;
    bool initialized_ = false;
    
        // Vulkan resources
        vk::Device device_;
        VmaAllocator allocator_ = nullptr;
        vk::Queue computeQueue_;
        vk::Queue graphicsQueue_;  // For GPU uploads (staging buffer copies)
        vk::CommandPool commandPool_;
    
    // Shader and pipeline
    vk::ShaderModule computeShader_;
    vk::Pipeline pipeline_;
    vk::PipelineLayout pipelineLayout_;
    vk::DescriptorSetLayout descriptorSetLayout_;
    vk::DescriptorPool descriptorPool_;
    vk::DescriptorSet descriptorSet_;
    
    // Output image (RGBA16F)
    vk::Image outputImage_;
    VmaAllocation outputImageAllocation_;
    vk::ImageView outputImageView_;
    
    // Triangle storage (SSBO)
    vk::Buffer triangleBuffer_;
    VmaAllocation triangleBufferAllocation_;
    uint32_t triangleCount_ = 0;
    uint32_t maxTriangles_ = 100000;  // Max capacity
    
    // Depth-sorted indices (SSBO)
    vk::Buffer sortedIndicesBuffer_;
    VmaAllocation sortedIndicesAllocation_;
    
    // Depth keys buffer (for bitonic sort)
    vk::Buffer depthKeysBuffer_;
    VmaAllocation depthKeysAllocation_;
    
    // Bitonic sort resources
    vk::ShaderModule bitonicSortShader_;
    vk::Pipeline bitonicSortPipeline_;
    vk::PipelineLayout bitonicSortPipelineLayout_;
    vk::DescriptorSetLayout bitonicSortDescriptorSetLayout_;
    vk::DescriptorPool bitonicSortDescriptorPool_;
    vk::DescriptorSet bitonicSortDescriptorSet_;
    
    // Depth key computation resources
    vk::ShaderModule depthKeyComputeShader_;
    vk::Pipeline depthKeyComputePipeline_;
    vk::PipelineLayout depthKeyComputePipelineLayout_;
    vk::DescriptorSetLayout depthKeyComputeDescriptorSetLayout_;
    vk::DescriptorPool depthKeyComputeDescriptorPool_;
    vk::DescriptorSet depthKeyComputeDescriptorSet_;
    
    // Frustum culling resources
    vk::ShaderModule frustumCullingShader_;
    vk::Pipeline frustumCullingPipeline_;
    vk::PipelineLayout frustumCullingPipelineLayout_;
    vk::DescriptorSetLayout frustumCullingDescriptorSetLayout_;
    vk::DescriptorPool frustumCullingDescriptorPool_;
    vk::DescriptorSet frustumCullingDescriptorSet_;
    
    // Visible triangle indices buffer (compacted after culling)
    vk::Buffer visibleIndicesBuffer_;
    VmaAllocation visibleIndicesAllocation_;
    
    // Atomic counter buffer (stores visible triangle count)
    vk::Buffer atomicCounterBuffer_;
    VmaAllocation atomicCounterAllocation_;
    
    // Indirect dispatch buffer (for vkCmdDispatchIndirect)
    vk::Buffer indirectDispatchBuffer_;
    VmaAllocation indirectDispatchAllocation_;
    
    // Indirect args compute resources (fills indirectDispatchBuffer_)
    vk::ShaderModule indirectArgsShader_;
    vk::Pipeline indirectArgsPipeline_;
    vk::PipelineLayout indirectArgsPipelineLayout_;
    vk::DescriptorSetLayout indirectArgsDescriptorSetLayout_;
    vk::DescriptorPool indirectArgsDescriptorPool_;
    vk::DescriptorSet indirectArgsDescriptorSet_;
    
    // Push constants
    struct PushConstants {
        glm::mat4 viewProj;          // 64 bytes (offset 0)
        uint32_t outputWidth;        // offset 64
        uint32_t outputHeight;       // offset 68
        uint32_t triangleCount;      // offset 72
        uint32_t enableEarlyTerm;    // offset 76
        float alphaThreshold;        // offset 80
        uint32_t enableTileBinning;  // offset 84 (0 = off, 1 = on)
        uint32_t padding[2];         // offset 88-95
    } pushConstants_;
    
    struct BitonicSortPushConstants {
        uint32_t h;           // Height parameter
        uint32_t algorithm;   // Algorithm variant (0-3)
        uint32_t numElements; // Total elements
        uint32_t padding;     // Alignment
    };
    
    struct DepthKeyComputePushConstants {
        glm::vec3 cameraPos;     // Camera position (12 bytes)
        uint32_t triangleCount;  // Number of triangles to process (visible count if culling enabled)
        uint32_t useCulling;     // 1 = use visibleIndices, 0 = process all triangles
        uint32_t padding;        // Alignment
    };
    
    struct FrustumCullingPushConstants {
        glm::mat4 viewProj;      // View-Projection matrix (64 bytes)
        uint32_t triangleCount;  // Total triangles (4 bytes)
        uint32_t padding[15];    // Align to 128 bytes
    };
    
    struct IndirectArgsPushConstants {
        uint32_t threadsPerGroup; // Threads per workgroup (usually 256)
        uint32_t padding[3];      // Alignment
    };
    
    // Camera position for depth sorting
    glm::vec3 cameraPosition_ = glm::vec3(0.0f, 0.0f, 0.0f);
    
    // Frustum culling enabled flag
    bool enableFrustumCulling_ = true;

    // Tile culling resources (для оптимизации производительности)
    vk::Buffer tileCullingBuffer_;
    VmaAllocation tileCullingAllocation_;
    uint32_t tileCount_ = 0;  // Количество тайлов (ширина/16 * высота/16)
    // Tile culling pipeline resources
    vk::ShaderModule tileCullingShader_;
    vk::Pipeline tileCullingPipeline_;
    vk::PipelineLayout tileCullingPipelineLayout_;
    vk::DescriptorSetLayout tileCullingDescriptorSetLayout_;
    vk::DescriptorPool tileCullingDescriptorPool_;
    vk::DescriptorSet tileCullingDescriptorSet_;

    // ========================================================================
    // Two-Pass Rendering resources (Priority #1 Optimization)
    // ========================================================================
    // Visibility Pass (Pass 1): Each triangle writes its ID to pixels it covers
    vk::ShaderModule visibilityPassShader_;
    vk::Pipeline visibilityPassPipeline_;
    vk::PipelineLayout visibilityPassPipelineLayout_;
    vk::DescriptorSetLayout visibilityPassDescriptorSetLayout_;
    vk::DescriptorPool visibilityPassDescriptorPool_;
    vk::DescriptorSet visibilityPassDescriptorSet_;
    
    // Shading Pass (Pass 2): Each pixel processes only its visible triangles
    vk::ShaderModule shadingPassShader_;
    vk::Pipeline shadingPassPipeline_;
    vk::PipelineLayout shadingPassPipelineLayout_;
    vk::DescriptorSetLayout shadingPassDescriptorSetLayout_;
    vk::DescriptorPool shadingPassDescriptorPool_;
    vk::DescriptorSet shadingPassDescriptorSet_;
    
    // Visibility buffer: stores list of triangle IDs per pixel
    // Layout: for each pixel: [count, tri0, tri1, ..., triN]
    vk::Buffer visibilityBuffer_;
    VmaAllocation visibilityBufferAllocation_;
    
    // Pixel counters: atomic counters for each pixel (used during visibility pass)
    vk::Buffer pixelCountersBuffer_;
    VmaAllocation pixelCountersAllocation_;
    
    struct TwoPassPushConstants {
        glm::mat4 viewProj;
        uint32_t outputWidth;
        uint32_t outputHeight;
        uint32_t triangleCount;
        uint32_t maxTrianglesPerPixel;
        uint32_t enableEarlyTermination;
        float alphaThreshold;
        uint32_t padding;
    };

    // Helper methods
    bool createShaderModule();
    bool createPipeline();
    bool createDescriptorSets();
    bool createOutputImage();
    bool createBuffers();
    bool createBitonicSortResources();
    bool createDepthKeyComputeResources();
    bool createFrustumCullingResources();
    bool createIndirectArgsResources();
    bool createTileCullingResources();
    bool createTileCullingPipelineResources();
    void sortTrianglesByDepth(vk::CommandBuffer cmd);
    void computeDepthKeys(vk::CommandBuffer cmd, const glm::vec3& cameraPos);
    void performFrustumCulling(vk::CommandBuffer cmd);
    void computeIndirectArgs(vk::CommandBuffer cmd);
    uint32_t nextPowerOfTwo(uint32_t n);
    
    // Two-Pass Rendering methods
    bool createTwoPassResources();
    void executeTwoPassRendering(vk::CommandBuffer cmd);
    void executeVisibilityPass(vk::CommandBuffer cmd);
    void executeShadingPass(vk::CommandBuffer cmd);
};

} // namespace rendering
} // namespace spectraforge

