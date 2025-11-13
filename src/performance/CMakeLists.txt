# Performance benchmarking utilities for SpectraForge

# Triangle Splatting Benchmark
add_executable(TriangleSplattingBenchmark
    TriangleSplattingBenchmark.cpp
)

target_include_directories(TriangleSplattingBenchmark
    PRIVATE
    ${CMAKE_SOURCE_DIR}/include
    ${CMAKE_SOURCE_DIR}/external
)

target_link_libraries(TriangleSplattingBenchmark
    PRIVATE
    SpectraForge
    Vulkan::Vulkan
)

# Set output directory
set_target_properties(TriangleSplattingBenchmark PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
)

# Copy benchmark results to build directory
file(MAKE_DIRECTORY "${CMAKE_BINARY_DIR}/benchmark_results")

# Add benchmark target
add_custom_target(run_triangle_splatting_benchmark
    COMMAND TriangleSplattingBenchmark
    DEPENDS TriangleSplattingBenchmark
    WORKING_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
    COMMENT "Running Triangle Splatting performance benchmark"
)
