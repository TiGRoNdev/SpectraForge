add_library(FrameOutput SHARED
    FrameOutput.cpp
)

find_package(PNG REQUIRED)

target_include_directories(FrameOutput PUBLIC
    ${CMAKE_SOURCE_DIR}/include
)

target_link_libraries(FrameOutput PRIVATE
    PNG::PNG
)


