# SpectraForge Upscaling Module

# Create upscaling module as object library
add_library(SpectraForge_Upscaling OBJECT
    Upscaler.cpp
    UpscalerFactory.cpp
    NativeUpscaler.cpp
    FSR2Upscaler.cpp
    DLSSUpscaler.cpp
)

# Include directories
target_include_directories(SpectraForge_Upscaling PUBLIC
    ${CMAKE_SOURCE_DIR}/include
)

# Link Vulkan
if(Vulkan_FOUND)
    target_link_libraries(SpectraForge_Upscaling PUBLIC
        ${Vulkan_LIBRARIES}
    )
    target_include_directories(SpectraForge_Upscaling PUBLIC
        ${Vulkan_INCLUDE_DIRS}
    )
endif()

# Compile definitions
target_compile_definitions(SpectraForge_Upscaling PUBLIC
    SPECTRAFORGE_UPSCALING_BUILD
)

message(STATUS "SpectraForge_Upscaling module configured")
