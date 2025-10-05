# SpectraForge Platform Module (оконная система)

# Create platform module as object library
add_library(SpectraForge_Platform OBJECT
    X11Window.cpp
)

# Include directories
target_include_directories(SpectraForge_Platform PUBLIC
    ${CMAKE_SOURCE_DIR}/include
)

# Compile definitions
target_compile_definitions(SpectraForge_Platform PUBLIC
    ENGINE_3D_BUILD
)

# Link with X11 on Linux
if(UNIX AND NOT APPLE)
    find_package(X11 REQUIRED)
    target_include_directories(SpectraForge_Platform PRIVATE ${X11_INCLUDE_DIR})
    target_link_libraries(SpectraForge_Platform PRIVATE ${X11_LIBRARIES})
endif()