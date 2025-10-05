# SpectraForge Math Module (Pure C++)

# Create math module as object library
add_library(SpectraForge_Math OBJECT
    Vec4.cpp
    vector/Vector3.cpp
    vector/Vector4.cpp
    matrix/Matrix4.cpp
    matrix/Quaternion.cpp
)

# Include directories
target_include_directories(SpectraForge_Math PUBLIC
    ${CMAKE_SOURCE_DIR}/include
)

# Compile definitions
target_compile_definitions(SpectraForge_Math PUBLIC
    ENGINE_3D_BUILD
)