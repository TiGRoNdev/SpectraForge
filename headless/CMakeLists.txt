add_library(SpectraForgeHeadless STATIC
    src/Engine.cpp
    src/Logger.cpp
)

target_include_directories(SpectraForgeHeadless
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)

add_executable(BlueCube_Demo_FIXED
    examples/BlueCube_Demo_FIXED.cpp
)
target_link_libraries(BlueCube_Demo_FIXED PRIVATE SpectraForgeHeadless)
set_target_properties(BlueCube_Demo_FIXED PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/examples"
)

add_executable(SpectraForge_Example_Demo_FIXED
    examples/SpectraForge_Example_Demo_FIXED.cpp
)
target_link_libraries(SpectraForge_Example_Demo_FIXED PRIVATE SpectraForgeHeadless)
set_target_properties(SpectraForge_Example_Demo_FIXED PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/examples"
)

add_subdirectory(tests)
