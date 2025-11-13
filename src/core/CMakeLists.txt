# SpectraForge Core Module (Pure C++ with Vulkan)

# Create core module as object library
add_library(SpectraForge_Core OBJECT
    Component.cpp
    Console.cpp
    Container.cpp
    EngineCore.cpp
    GameObject3D.cpp
    Logger.cpp
    Transform3D.cpp
    VMAImplementation.cpp
    VMAMemoryManager.cpp
    VulkanHppDispatch.cpp
    Window.cpp
    WindowFactory.cpp
)

# Include directories
target_include_directories(SpectraForge_Core PUBLIC
    ${CMAKE_SOURCE_DIR}/include
    ${CMAKE_SOURCE_DIR}/external
)

# Link with Vulkan
target_link_libraries(SpectraForge_Core PUBLIC Vulkan::Vulkan)

# Compile definitions
target_compile_definitions(SpectraForge_Core PUBLIC
    ENGINE_3D_BUILD
    SpectraForge_ENABLE_VULKAN
)