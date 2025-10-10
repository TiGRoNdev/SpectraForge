cmake_minimum_required(VERSION 3.16)

# SpectraForge - Pure C++ 3D Engine with Vulkan and Hybrid TriangleSplatting + FreGS rendering
project(SpectraForge VERSION 1.0.0 LANGUAGES CXX)

# Устанавливаем стандарт C++
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Опции сборки
option(BUILD_EXAMPLES "Build example applications" ON)
option(BUILD_TESTS "Build test suite" OFF)

message(STATUS "=== SpectraForge Configuration ===")
message(STATUS "Pure C++ with Vulkan only")
message(STATUS "Hybrid TriangleSplatting + FreGS renderer")
message(STATUS "==================================")

# Настройки компилятора
if(MSVC)
    add_compile_options(/W4)
    add_compile_definitions(_CRT_SECURE_NO_WARNINGS)
    set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>DLL")
else()
    add_compile_options(-Wall -Wextra -Wpedantic)
endif()

# Включение директорий
include_directories(${CMAKE_SOURCE_DIR}/include)

# ============================================================================
# Vulkan SDK Setup (Pure C++ with Vulkan only)
# ============================================================================

# Используем системный Vulkan SDK
find_package(Vulkan QUIET)

if(NOT Vulkan_FOUND)
    find_package(PkgConfig QUIET)
    if(PkgConfig_FOUND)
        pkg_check_modules(VULKAN QUIET vulkan)
    endif()

    if(VULKAN_FOUND)
        message(STATUS "Found Vulkan SDK via pkg-config")
        message(STATUS "Vulkan include: ${VULKAN_INCLUDE_DIRS}")
        message(STATUS "Vulkan libraries: ${VULKAN_LIBRARIES}")

        add_library(Vulkan::Vulkan INTERFACE IMPORTED)
        set_target_properties(Vulkan::Vulkan PROPERTIES
            INTERFACE_INCLUDE_DIRECTORIES "${VULKAN_INCLUDE_DIRS}"
            INTERFACE_LINK_LIBRARIES "${VULKAN_LIBRARIES}"
        )
    endif()
else()
    message(STATUS "Found Vulkan SDK via find_package")
endif()

if(NOT TARGET Vulkan::Vulkan)
    message(FATAL_ERROR "Vulkan SDK not found. Please install Vulkan SDK or provide pkg-config file.")
endif()

# Vulkan Memory Allocator (header-only dependency)
include(FetchContent)
FetchContent_Declare(
    vma
    URL https://github.com/GPUOpen-LibrariesAndSDKs/VulkanMemoryAllocator/archive/refs/tags/v3.1.0.zip
    DOWNLOAD_EXTRACT_TIMESTAMP TRUE
)
FetchContent_GetProperties(vma)
if(NOT vma_POPULATED)
    FetchContent_Populate(vma)
endif()

set(VMA_INCLUDE_DIR ${vma_SOURCE_DIR}/include)
message(STATUS "VulkanMemoryAllocator include dir: ${VMA_INCLUDE_DIR}")

add_library(VulkanMemoryAllocator INTERFACE)
target_include_directories(VulkanMemoryAllocator INTERFACE
    ${VMA_INCLUDE_DIR}
)

# Добавляем дополнительные пути поиска для GLSLC (shader compiler)
find_program(GLSLC_EXECUTABLE glslc)

if(GLSLC_EXECUTABLE)
    message(STATUS "Found glslc: ${GLSLC_EXECUTABLE}")
else()
    message(WARNING "glslc not found - shader compilation may not work")
endif()

# ============================================================
# Сборка SpectraForge Core Library
# ============================================================

# Подключаем модульную структуру
add_subdirectory(src)

# Подключаем тесты (если включены)
option(ENABLE_TESTS "Enable tests" ON)
if(ENABLE_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()

# Создаем основную библиотеку SpectraForge
add_library(SpectraForge STATIC
    $<TARGET_OBJECTS:SpectraForge_Core>
    $<TARGET_OBJECTS:SpectraForge_Math>
    $<TARGET_OBJECTS:SpectraForge_Platform>
    $<TARGET_OBJECTS:SpectraForge_Vulkan>
    $<TARGET_OBJECTS:SpectraForge_Upscaling>
    $<TARGET_OBJECTS:SpectraForge_Input>
    $<TARGET_OBJECTS:SpectraForge_Physics>
    $<TARGET_OBJECTS:SpectraForge_App>
)

# Подключаем только Vulkan. PNG удалён из общей библиотеки и перенесён в FrameOutput.
target_link_libraries(SpectraForge PUBLIC Vulkan::Vulkan glfw)

# Определения компилятора
target_compile_definitions(SpectraForge PUBLIC
    ENGINE_3D_BUILD
    SpectraForge_ENABLE_VULKAN
)

# Включаем заголовочные файлы
target_include_directories(SpectraForge PUBLIC
        ${CMAKE_SOURCE_DIR}/include
)

# Установка библиотеки
install(TARGETS SpectraForge
        RUNTIME DESTINATION bin
        LIBRARY DESTINATION lib
        ARCHIVE DESTINATION lib
    )

# Создание демо приложения
if(BUILD_EXAMPLES)
    add_executable(DebugSponza_Demo
        examples/DebugSponza_Demo.cpp
    )
    target_link_libraries(DebugSponza_Demo PRIVATE 
        SpectraForge_App
        SpectraForge_Core
        SpectraForge_Math
        SpectraForge_Platform
        SpectraForge_Vulkan
        SpectraForge
        FrameOutput
        glfw
    )

    # Add X11 libraries for window support on Linux
    if(UNIX AND NOT APPLE)
        find_package(X11 REQUIRED)
        target_include_directories(DebugSponza_Demo PRIVATE ${X11_INCLUDE_DIR})
        target_link_libraries(DebugSponza_Demo PRIVATE ${X11_LIBRARIES})
    endif()

    message(STATUS "DebugSponza_Demo created (Debug sponza scene with lighting)")

    # SpectraForge Example Demo (Sponza scene with Triangle Splatting)
    add_executable(SpectraForge_Example_Demo
        examples/SpectraForge_Example_Demo.cpp
    )
    target_link_libraries(SpectraForge_Example_Demo PRIVATE 
        SpectraForge_App
        SpectraForge_Core
        SpectraForge_Math
        SpectraForge_Platform
        SpectraForge_Vulkan
        SpectraForge
        FrameOutput
        glfw
    )

    # Add X11 libraries for window support on Linux
    if(UNIX AND NOT APPLE)
        find_package(X11 REQUIRED)
        target_include_directories(SpectraForge_Example_Demo PRIVATE ${X11_INCLUDE_DIR})
        target_link_libraries(SpectraForge_Example_Demo PRIVATE ${X11_LIBRARIES})
    endif()

    message(STATUS "SpectraForge Example Demo created (Sponza scene with lighting)")

    # Blue Cube minimal demo
    add_executable(BlueCube_Demo
        examples/BlueCube_Demo.cpp
    )
    target_link_libraries(BlueCube_Demo PRIVATE 
        SpectraForge_App
        SpectraForge_Core
        SpectraForge_Math
        SpectraForge_Platform
        SpectraForge_Vulkan
        SpectraForge
        FrameOutput
        glfw
    )

    if(UNIX AND NOT APPLE)
        find_package(X11 REQUIRED)
        target_include_directories(BlueCube_Demo PRIVATE ${X11_INCLUDE_DIR})
        target_link_libraries(BlueCube_Demo PRIVATE ${X11_LIBRARIES})
    endif()

    message(STATUS "BlueCube_Demo created (rotating blue cube)")
    
    # Optimized demo with 60+ FPS on mobile GPUs
    add_executable(SpectraForge_Optimized_Demo
        examples/SpectraForge_Optimized_Demo.cpp
    )
    target_link_libraries(SpectraForge_Optimized_Demo PRIVATE 
        SpectraForge_App
        SpectraForge_Core
        SpectraForge_Math
        SpectraForge_Platform
        SpectraForge_Vulkan
        SpectraForge
        FrameOutput
        glfw
    )
    
    if(UNIX AND NOT APPLE)
        find_package(X11 REQUIRED)
        target_include_directories(SpectraForge_Optimized_Demo PRIVATE ${X11_INCLUDE_DIR})
        target_link_libraries(SpectraForge_Optimized_Demo PRIVATE ${X11_LIBRARIES})
    endif()
    
    message(STATUS "SpectraForge_Optimized_Demo created (60+ FPS, 4K HDR support)")
    
    # Instanced Mesh Demo
    add_executable(InstancedMeshDemo
        examples/InstancedMeshDemo.cpp
    )
    target_link_libraries(InstancedMeshDemo PRIVATE 
        SpectraForge_App
        SpectraForge_Core
        SpectraForge_Math
        SpectraForge_Platform
        SpectraForge_Vulkan
        SpectraForge
        FrameOutput
        glfw
    )
    
    if(UNIX AND NOT APPLE)
        find_package(X11 REQUIRED)
        target_include_directories(InstancedMeshDemo PRIVATE ${X11_INCLUDE_DIR})
        target_link_libraries(InstancedMeshDemo PRIVATE ${X11_LIBRARIES})
    endif()
    
    message(STATUS "InstancedMeshDemo created (instanced rendering showcase)")
    
    # Simple Triangle Test
    add_executable(SimpleTriangleTest
        examples/SimpleTriangleTest.cpp
    )
    target_link_libraries(SimpleTriangleTest PRIVATE 
        SpectraForge_App
        SpectraForge_Core
        SpectraForge_Math
        SpectraForge
        FrameOutput
        glfw
        ${CMAKE_DL_LIBS}
        ${CMAKE_THREAD_LIBS_INIT}
        ${PNG_LIBRARIES}
    )
    set_target_properties(SimpleTriangleTest PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}"
    )
    
    if(UNIX AND NOT APPLE)
        find_package(X11 REQUIRED)
        target_include_directories(SimpleTriangleTest PRIVATE ${X11_INCLUDE_DIR})
        target_link_libraries(SimpleTriangleTest PRIVATE ${X11_LIBRARIES})
    endif()
    
    message(STATUS "SimpleTriangleTest created (minimal triangle debug test)")
endif()

# ============================================================
# Финальная информация о сборке
# ============================================================
message(STATUS "")
message(STATUS "=== SpectraForge Build Summary ===")
message(STATUS "Pure C++ 3D Engine with Vulkan")
message(STATUS "Hybrid TriangleSplatting + FreGS renderer")
message(STATUS "Vulkan SDK: ${VULKAN_SDK_PATH}")
message(STATUS "Build Examples: ${BUILD_EXAMPLES}")
message(STATUS "==================================")
message(STATUS "")
