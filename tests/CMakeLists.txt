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

# Phase 1 connectivity tests
add_executable(connectivity_phase1_test
    connectivity_phase1_test.cpp
)

target_link_libraries(connectivity_phase1_test
    PRIVATE
    SpectraForge
    GTest::gtest_main
)

set_target_properties(connectivity_phase1_test PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
)

add_custom_target(run_connectivity_phase1_tests
    COMMAND connectivity_phase1_test
    DEPENDS connectivity_phase1_test
    WORKING_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
    COMMENT "Running Phase 1 connectivity tests"
)

add_test(NAME connectivity_phase1_test COMMAND connectivity_phase1_test)

# Interfaces (Week 1) tests
add_executable(render_strategy_interfaces_test
    render_strategy_interfaces_test.cpp
)

target_link_libraries(render_strategy_interfaces_test
    PRIVATE
    SpectraForge
    GTest::gtest_main
    Vulkan::Vulkan
)

set_target_properties(render_strategy_interfaces_test PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
)

add_custom_target(run_render_strategy_interfaces_tests
    COMMAND render_strategy_interfaces_test
    DEPENDS render_strategy_interfaces_test
    WORKING_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
    COMMENT "Running Render Strategy Interfaces tests"
)

add_test(NAME render_strategy_interfaces_test COMMAND render_strategy_interfaces_test)

# Week 2: Strategy factory tests
add_executable(render_strategy_factory_test
    render_strategy_factory_test.cpp
)

target_link_libraries(render_strategy_factory_test
    PRIVATE
    SpectraForge
    GTest::gtest_main
)

set_target_properties(render_strategy_factory_test PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
)

add_custom_target(run_render_strategy_factory_tests
    COMMAND render_strategy_factory_test
    DEPENDS render_strategy_factory_test
    WORKING_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
    COMMENT "Running Strategy Factory tests"
)

add_test(NAME render_strategy_factory_test COMMAND render_strategy_factory_test)

# Instanced Mesh tests (CPU-only)
add_executable(instanced_mesh_test
    instanced_mesh_test.cpp
)

target_link_libraries(instanced_mesh_test
    PRIVATE
    GTest::gtest_main
)

# Include GLM headers (header-only library)
target_include_directories(instanced_mesh_test PRIVATE ${CMAKE_SOURCE_DIR}/include)

set_target_properties(instanced_mesh_test PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
)

add_custom_target(run_instanced_mesh_tests
    COMMAND instanced_mesh_test
    DEPENDS instanced_mesh_test
    WORKING_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
    COMMENT "Running Instanced Mesh data packing tests"
)

add_test(NAME instanced_mesh_test COMMAND instanced_mesh_test)
