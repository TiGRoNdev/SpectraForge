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
find_package(PkgConfig REQUIRED)
pkg_check_modules(VULKAN REQUIRED vulkan)

if(NOT VULKAN_FOUND)
    message(FATAL_ERROR "Vulkan SDK not found. Please install Vulkan SDK.")
endif()

message(STATUS "Found Vulkan SDK via pkg-config")
message(STATUS "Vulkan include: ${VULKAN_INCLUDE_DIRS}")
message(STATUS "Vulkan libraries: ${VULKAN_LIBRARIES}")

# Создаем цель для Vulkan
add_library(Vulkan::Vulkan INTERFACE IMPORTED)
set_target_properties(Vulkan::Vulkan PROPERTIES
    INTERFACE_INCLUDE_DIRECTORIES "${VULKAN_INCLUDE_DIRS}"
    INTERFACE_LINK_LIBRARIES "${VULKAN_LIBRARIES}"
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
    add_subdirectory(tests)
endif()

# Создаем основную библиотеку SpectraForge
add_library(SpectraForge STATIC
    $<TARGET_OBJECTS:SpectraForge_Core>
    $<TARGET_OBJECTS:SpectraForge_Math>
    $<TARGET_OBJECTS:SpectraForge_Platform>
    $<TARGET_OBJECTS:SpectraForge_Vulkan>
)

# Подключаем Vulkan
target_link_libraries(SpectraForge PUBLIC Vulkan::Vulkan)

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
        glfw
    )

    # Add X11 libraries for window support on Linux
    if(UNIX AND NOT APPLE)
        find_package(X11 REQUIRED)
        target_include_directories(SpectraForge_Example_Demo PRIVATE ${X11_INCLUDE_DIR})
        target_link_libraries(SpectraForge_Example_Demo PRIVATE ${X11_LIBRARIES})
    endif()

    message(STATUS "SpectraForge Example Demo created (Sponza scene with lighting)")
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
