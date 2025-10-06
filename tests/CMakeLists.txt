# Test configuration for SpectraForge

# Find Google Test
find_package(GTest REQUIRED)

# Triangle Splatting tests
add_executable(triangle_splatting_test
    triangle_splatting_test.cpp
)

target_link_libraries(triangle_splatting_test
    PRIVATE
    SpectraForge
    FrameOutput
    GTest::gtest_main
    Vulkan::Vulkan
)

# Set output directory
set_target_properties(triangle_splatting_test PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
)

# Add test target
add_custom_target(run_triangle_splatting_tests
    COMMAND triangle_splatting_test
    DEPENDS triangle_splatting_test
    WORKING_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
    COMMENT "Running Triangle Splatting tests"
)

# Enable testing
enable_testing()

# Add test to CTest
add_test(NAME triangle_splatting_test COMMAND triangle_splatting_test)
