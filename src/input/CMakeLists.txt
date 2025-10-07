# Система ввода SpectraForge

message(STATUS "Настройка SpectraForge_Input")

# Create input module as object library
add_library(SpectraForge_Input OBJECT
    Input3D.cpp
)

# Include directories
target_include_directories(SpectraForge_Input PUBLIC
    ${CMAKE_SOURCE_DIR}/include
)

# Link GLFW
find_package(glfw3 REQUIRED)
target_link_libraries(SpectraForge_Input PUBLIC glfw)

# Compile definitions
target_compile_definitions(SpectraForge_Input PUBLIC
    SPECTRAFORGE_INPUT_BUILD
)

message(STATUS "SpectraForge_Input module configured")
