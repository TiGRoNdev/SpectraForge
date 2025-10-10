# SpectraForge Rendering Module (Pure C++ with Vulkan)

# Create rendering module as object library
add_library(SpectraForge_Vulkan OBJECT
    # P0.2 REFACTORING: Core components (SOLID-compliant)
    core/RendererCore.cpp
    core/SwapchainManager.cpp
    core/FrameManager.cpp
    core/PipelineManager.cpp
    core/RendererDebugger.cpp
    core/RendererStatistics.cpp
    
    # HybridFreGSRenderer (Refactored orchestrator ~250 lines - P0.2 Complete)
    HybridFreGSRenderer.cpp
    
    # P0.5: Camera3D (recreated after OpenGL removal)
    Camera3D.cpp
    
    # Other rendering components
    FreGSRenderStrategy.cpp
    StrategyFactory.cpp
    VulkanValidation.cpp
    FreGSPass.cpp
    WaveletPass.cpp
    InstancedMeshPass.cpp
    RenderPass.cpp
    
    # Triangle Splatting Pass (Decomposed - P0.1 Refactoring)
    triangle_splatting/TriangleSplattingCore.cpp
    triangle_splatting/TriangleBufferManager.cpp
    triangle_splatting/FrustumCullingPass.cpp
    triangle_splatting/DepthSortingPass.cpp
    triangle_splatting/TriangleRasterizationPass.cpp
    triangle_splatting/TriangleSplattingDebugger.cpp
    triangle_splatting/TriangleSplattingStatistics.cpp
    triangle_splatting/TriangleSplattingPass.cpp
    
    # P0.5: Legacy OpenGL removed (4657 lines deleted)
    # Vulkan-only build (Pure C++ 3D Engine)
    
    # Vulkan subsystem files only
    vulkan/VulkanRenderer.cpp
    vulkan/VulkanEngine.cpp
    vulkan/ResourceManager.cpp
    vulkan/SceneManager.cpp
    vulkan/HardwareDetector.cpp
)

# PNG (for debug frame saving)
find_package(PNG REQUIRED)

# Include directories
target_include_directories(SpectraForge_Vulkan PUBLIC
    ${CMAKE_SOURCE_DIR}/include
)

# Link with Vulkan and core modules
target_link_libraries(SpectraForge_Vulkan PUBLIC
    Vulkan::Vulkan
    SpectraForge_Core
    SpectraForge_Math
    PNG::PNG
)

# Compile definitions
target_compile_definitions(SpectraForge_Vulkan PUBLIC
    ENGINE_3D_BUILD
    SpectraForge_ENABLE_VULKAN
)