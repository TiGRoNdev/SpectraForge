# App module - High-level Engine facade

add_library(SpectraForge_App STATIC
    Engine.cpp
)

target_include_directories(SpectraForge_App PUBLIC
    ${CMAKE_SOURCE_DIR}/include
)

target_link_libraries(SpectraForge_App PUBLIC
    SpectraForge_Math
    SpectraForge_Core
    SpectraForge_Vulkan
)

