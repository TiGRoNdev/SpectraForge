# Физическая система SpectraForge

message(STATUS "Настройка SpectraForge_Physics")

# Create physics module as object library
add_library(SpectraForge_Physics OBJECT
    Physics3D.cpp
)

# Include directories
target_include_directories(SpectraForge_Physics PUBLIC
    ${CMAKE_SOURCE_DIR}/include
)

# Compile definitions
target_compile_definitions(SpectraForge_Physics PUBLIC
    SPECTRAFORGE_PHYSICS_BUILD
)

message(STATUS "SpectraForge_Physics module configured")
