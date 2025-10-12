add_executable(headless_engine_test headless_engine_test.cpp)

target_link_libraries(headless_engine_test PRIVATE SpectraForgeHeadless)

target_include_directories(headless_engine_test PRIVATE ${CMAKE_SOURCE_DIR}/headless/include)

set_target_properties(headless_engine_test PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
)

add_test(NAME headless_engine_test COMMAND headless_engine_test)
