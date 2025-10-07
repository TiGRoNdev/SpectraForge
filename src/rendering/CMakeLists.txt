# SpectraForge Rendering Module (Pure C++ with Vulkan)

# Create rendering module as object library
add_library(SpectraForge_Vulkan OBJECT
    HybridFreGSRenderer.cpp
    FreGSRenderStrategy.cpp
    StrategyFactory.cpp
    VulkanValidation.cpp
    TriangleSplattingPass.cpp
    FreGSPass.cpp
    WaveletPass.cpp
    RenderPass.cpp
    # Minimal OpenGL-compatible components used across the engine
    opengl/Camera3D.cpp
    opengl/Mesh3D.cpp
    # Vulkan subsystem files only (Vulkan-only build)
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