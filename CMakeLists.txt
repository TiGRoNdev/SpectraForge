cmake_minimum_required(VERSION 3.16)
project(HyperEngine VERSION 1.0.0 LANGUAGES CXX)

# Установка стандарта C++
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Настройки компилятора
if(MSVC)
    add_compile_options(/W4)
    # Отключаем предупреждения о deprecated функциях
    add_compile_definitions(_CRT_SECURE_NO_WARNINGS)
else()
    add_compile_options(-Wall -Wextra -Wpedantic)
endif()

# Опции сборки
option(BUILD_ENGINE_3D "Build 3D Engine" ON)
option(BUILD_ENGINE_4D "Build 4D Engine" OFF)
option(BUILD_EXAMPLES "Build example applications" ON)
option(BUILD_TESTS "Build test suite" OFF)

# Включение директорий
include_directories(${CMAKE_SOURCE_DIR}/include)

# Поиск пакетов
if(BUILD_ENGINE_4D)
    find_package(Vulkan REQUIRED)
    find_package(unofficial-vulkan-memory-allocator CONFIG REQUIRED)
endif()

# Поиск GLFW3 через vcpkg
find_package(glfw3 CONFIG REQUIRED)

# Поиск GLM через vcpkg
find_package(glm CONFIG REQUIRED)

# ============================================================
# Сборка 3D движка
# ============================================================
if(BUILD_ENGINE_3D)
    message(STATUS "Building 3D Engine...")
    
    # Сбор файлов для 3D движка
    file(GLOB_RECURSE ENGINE3D_SOURCES 
        "src3D/*.cpp"
        "src3D/*.h"
    )
    
    # Создание библиотеки 3D движка
    add_library(Engine3D STATIC ${ENGINE3D_SOURCES})
    
    # Подключение заголовочных файлов
    target_include_directories(Engine3D PUBLIC 
        ${CMAKE_SOURCE_DIR}/include
        ${CMAKE_SOURCE_DIR}/include/Engine3D
    )
    
    # Подключение библиотек к 3D движку
    target_link_libraries(Engine3D PUBLIC 
        glfw
        glm::glm
    )
    
    # Определения компилятора для 3D
    target_compile_definitions(Engine3D PUBLIC ENGINE_3D_BUILD)
    
    # Установка 3D движка
    install(TARGETS Engine3D
        RUNTIME DESTINATION bin
        LIBRARY DESTINATION lib
        ARCHIVE DESTINATION lib
    )
    
    # Создание простого демо для 3D
    if(BUILD_EXAMPLES)
        add_executable(Engine3D_Demo 
            examples/3d_demo.cpp
        )
        target_link_libraries(Engine3D_Demo PRIVATE Engine3D)
    endif()
endif()

# ============================================================
# Сборка 4D движка (legacy Vulkan)
# ============================================================
if(BUILD_ENGINE_4D)
    message(STATUS "Building 4D Engine...")
    
    # Сбор файлов для 4D движка (старые файлы)
    file(GLOB_RECURSE ENGINE4D_SOURCES 
        "src/*.cpp"
        "src/*.h"
    )
    
    # Исключаем src3D файлы
    list(FILTER ENGINE4D_SOURCES EXCLUDE REGEX "src3D/.*")
    
    # Создание библиотеки 4D движка
    add_library(Engine4D STATIC ${ENGINE4D_SOURCES})
    
    # Подключение заголовочных файлов
    target_include_directories(Engine4D PUBLIC 
        ${CMAKE_SOURCE_DIR}/include
        ${CMAKE_SOURCE_DIR}/include/Engine4D
    )
    
    # Подключение библиотек к 4D движку
    target_link_libraries(Engine4D PUBLIC 
        Vulkan::Vulkan
        glfw
        glm::glm
        unofficial::vulkan-memory-allocator::vulkan-memory-allocator
    )
    
    # Определения компилятора для 4D
    target_compile_definitions(Engine4D PUBLIC ENGINE_4D_BUILD)
    
    # Установка 4D движка
    install(TARGETS Engine4D
        RUNTIME DESTINATION bin
        LIBRARY DESTINATION lib
        ARCHIVE DESTINATION lib
    )
    
    # Создание демо для 4D
    if(BUILD_EXAMPLES)
        add_executable(Engine4D_Demo 
            examples/4d_demo.cpp
        )
        target_link_libraries(Engine4D_Demo PRIVATE Engine4D)
    endif()
endif()

# ============================================================
# Тесты
# ============================================================
if(BUILD_TESTS)
    enable_testing()
    
    if(BUILD_ENGINE_3D)
        add_subdirectory(tests/3d)
    endif()
    
    if(BUILD_ENGINE_4D)
        add_subdirectory(tests/4d)
    endif()
endif()

# ============================================================
# Установка заголовочных файлов
# ============================================================
if(BUILD_ENGINE_3D)
    install(DIRECTORY include/Engine3D/ DESTINATION include/Engine3D)
endif()

if(BUILD_ENGINE_4D)
    install(DIRECTORY include/Engine4D/ DESTINATION include/Engine4D)
endif()

# ============================================================
# Информация о сборке
# ============================================================
message(STATUS "")
message(STATUS "=== HyperEngine Build Configuration ===")
message(STATUS "Build 3D Engine: ${BUILD_ENGINE_3D}")
message(STATUS "Build 4D Engine: ${BUILD_ENGINE_4D}")
message(STATUS "Build Examples: ${BUILD_EXAMPLES}")
message(STATUS "Build Tests: ${BUILD_TESTS}")
message(STATUS "C++ Standard: ${CMAKE_CXX_STANDARD}")
message(STATUS "Compiler: ${CMAKE_CXX_COMPILER_ID}")
message(STATUS "========================================")
message(STATUS "")
