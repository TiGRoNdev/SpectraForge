#pragma once

#include <vulkan/vulkan.hpp>
#include <vk_mem_alloc.h>
#include <glm/glm.hpp>
#include <vector>
#include <cstdint>
#include <memory>
#include <SpectraForge/Rendering/Mesh3D.h>

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
     * @brief Convert mesh data to triangles
     * @param mesh Input mesh data
     * @param sigma Smoothness parameter for window function
     * @return Vector of triangles ready for GPU upload
     */
    static std::vector<Triangle> convertMeshToTriangles(const SpectraForge::Rendering::Mesh3D& mesh, float sigma = 1.0f);
    
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
     * @brief Set debug visualization mode
     * @param mode 0 = normal rendering, 1 = SDF visualization, 2 = barycentric visualization
     */
    void setDebugMode(uint32_t mode);
    
    /**
     * @brief Set lighting parameters
     * @param lightDirection Directional light direction (normalized)
     * @param lightIntensity Light intensity (0.0 - 2.0)
     * @param ambientColor Ambient light color
     * @param ambientIntensity Ambient light intensity
     */
    void setLighting(const glm::vec3& lightDirection, float lightIntensity,
                    const glm::vec3& ambientColor, float ambientIntensity);
    
    /**
     * @brief Enable/disable lighting
     * @param enabled true to enable lighting, false to disable
     */
    void setLightingEnabled(bool enabled);
    
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
    
    /**
     * @brief Save current frame to PPM file (DEBUG)
     */
    void saveFrameToPPM(const std::string& filename);

    // =========================================================================
    // Совместимые синонимы API для интеграции с HybridFreGSRenderer
    // =========================================================================
public:
    inline void setBackfaceCullingEnabled(bool enable) { setFrustumCullingEnabled(enable); }
    inline void setBackgroundColor(const glm::vec4& /*color*/) { /* no-op, управляется в renderer */ }
    inline void setAlphaBlendingEnabled(bool /*enable*/) { /* no-op until implemented */ }
    inline void setTriangleBudget(uint32_t /*maxTriangles*/) { /* no-op until implemented */ }
    inline void setEarlyTerminationEnabled(bool /*enable*/) { /* no-op until implemented */ }

    inline uint32_t getCulledTriangleCount() const {
        const uint32_t visible = getVisibleTriangleCount();
        return (visible > 0u) ? (visible / 3u) : 0u; // эвристика до появления реального подсчёта
    }

    inline void flushUniforms() { /* no-op */ }

    // Обёртка под bool не добавляется: базовая сигнатура уже void(std::string const&),
    // возврат значения обеспечивается на уровне HybridFreGSRenderer через try/catch

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
    struct UnifiedPushConstants {
        glm::mat4 viewProj;                // 64 bytes
        std::uint32_t outputWidth;         // 4 bytes
        std::uint32_t outputHeight;        // 4 bytes
        std::uint32_t triangleCount;       // 4 bytes
        std::uint32_t enableEarlyTermination; // 4 bytes
        float alphaThreshold;              // 4 bytes
        std::uint32_t enableTileBinning;   // 4 bytes
        std::uint32_t debugMode;           // 4 bytes
        std::uint32_t padding;             // 4 bytes
        // Total: 96 bytes
    };
    UnifiedPushConstants pushConstants_{};
    // Использовать эту структуру во ВСЕХ compute shaders
    // И установить size = 96 во всех pipeline layouts
    
    // Camera position for depth sorting
    glm::vec3 cameraPosition_ = glm::vec3(0.0f, 0.0f, 0.0f);
    
    // Frustum culling enabled flag
    bool enableFrustumCulling_ = true;
    
    // Lighting parameters
    glm::vec3 lightDirection_ = glm::vec3(0.0f, 0.0f, -1.0f); // Default downward light
    float lightIntensity_ = 1.0f;
    glm::vec3 ambientColor_ = glm::vec3(0.1f, 0.1f, 0.1f); // Dark gray ambient
    float ambientIntensity_ = 0.3f;
    bool enableLighting_ = true;

    // Tile culling resources (для оптимизации производительности)
    vk::Buffer tileCullingBuffer_;
    VmaAllocation tileCullingAllocation_;
    uint32_t tileCount_ = 0;  // Количество тайлов (ширина/16 * высота/16)

    // Material and texture resources
    vk::Buffer materialTexturesBuffer_;
    VmaAllocation materialTexturesAllocation_;
    vk::Buffer textureDataBuffer_;
    VmaAllocation textureDataAllocation_;
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
        glm::vec3 lightDirection;        // Directional light direction (normalized)
        float lightIntensity;            // Light intensity (0.0 - 2.0)
        glm::vec3 ambientColor;          // Ambient light color
        float ambientIntensity;          // Ambient light intensity
        uint32_t enableLighting;         // Enable/disable lighting (0=off, 1=on)
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

