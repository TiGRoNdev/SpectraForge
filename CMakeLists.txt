cmake_minimum_required(VERSION 3.16)

# Определяем языки в зависимости от опций
set(PROJECT_LANGUAGES CXX)

# Опции сборки (определяем рано, чтобы использовать в project())
option(BUILD_ENGINE_3D "Build 3D Engine" ON)
option(BUILD_ENGINE_4D "Build 4D Engine" OFF)
option(BUILD_VULKAN_RENDERER "Build Vulkan-based Hybrid Renderer" ON)
option(BUILD_WITH_CUDA "Build with CUDA support" OFF)
option(BUILD_WITH_OPTIX "Build with OptiX support" ON)
option(BUILD_WITH_DLSS "Build with DLSS support" ON)
option(BUILD_WITH_FSR "Build with FSR support" ON)
option(BUILD_EXAMPLES "Build example applications" ON)
option(BUILD_TESTS "Build test suite" ON)
option(ENABLE_CODE_COVERAGE "Enable code coverage for tests" ON)

# Ранняя настройка CUDA для поддержки
if(BUILD_WITH_CUDA)
    # Пытаемся найти CUDA compiler
    find_program(CUDA_NVCC_EXECUTABLE nvcc)
    if(CUDA_NVCC_EXECUTABLE)
        set(CMAKE_CUDA_COMPILER ${CUDA_NVCC_EXECUTABLE})
        # Только добавляем CUDA если компилятор найден
        # Но не включаем его в PROJECT_LANGUAGES пока не убедимся что все работает
        message(STATUS "CUDA compiler found: ${CUDA_NVCC_EXECUTABLE}")
        set(CUDA_COMPILER_AVAILABLE TRUE)
    else()
        message(WARNING "CUDA compiler not found. CUDA support will be disabled.")
        set(BUILD_WITH_CUDA OFF)
        set(CUDA_COMPILER_AVAILABLE FALSE)
    endif()
else()
    message(STATUS "CUDA support disabled by user")
    set(CUDA_COMPILER_AVAILABLE FALSE)
endif()

# Опции для адаптеров рендеринга
option(ENABLE_OPENGL_BACKEND "Enable OpenGL rendering backend" ON)
option(ENABLE_VULKAN_BACKEND "Enable Vulkan rendering backend" ON)
option(DEFAULT_BACKEND_VULKAN "Use Vulkan as default backend (otherwise OpenGL)" ON)

# Создаем проект с поддерживаемыми языками
project(HyperEngine VERSION 1.0.0 LANGUAGES ${PROJECT_LANGUAGES})

# Включаем CUDA если нужно
if(BUILD_WITH_CUDA AND CUDA_COMPILER_AVAILABLE)
    enable_language(CUDA)
    set(CMAKE_CUDA_ARCHITECTURES "50;60;70;75;80;86")
    message(STATUS "CUDA language enabled")
endif()

# Устанавливаем стандарт C++ для всего проекта
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Обеспечиваем наличие Vulkan Memory Allocator (только если BUILD_VULKAN_RENDERER включен)
if(BUILD_VULKAN_RENDERER)
    find_path(VMA_INCLUDE_DIR
        NAMES vk_mem_alloc.h
        PATHS ${VULKAN_MEMORY_ALLOCATOR_INCLUDE_DIRS}
              ${VulkanMemoryAllocator_INCLUDE_DIRS}
              ${VulkanMemoryAllocator_INCLUDE_DIR}
              /usr/local/include/vma
              /usr/local/include
              /usr/include
        PATH_SUFFIXES include include/vma vma
    )

    if(NOT VMA_INCLUDE_DIR)
        message(FATAL_ERROR "Unable to locate vk_mem_alloc.h. Please ensure Vulkan Memory Allocator is installed and available.")
    endif()

    set(VMA_INCLUDE_DIR "${VMA_INCLUDE_DIR}" CACHE PATH "Vulkan Memory Allocator include directory")
endif()

# Проверяем CUDA без включения языка (обходим проблему с toolset)
if(BUILD_WITH_CUDA AND CUDA_COMPILER_AVAILABLE)
    # Сначала найдем CUDAToolkit для проверки совместимости
    find_package(CUDAToolkit QUIET)

    if(CUDAToolkit_FOUND)
        message(STATUS "CUDAToolkit found: ${CUDAToolkit_VERSION}")

        # Используем только runtime CUDA без компиляции (обходим проблему toolset)
        message(STATUS "Using CUDA runtime libraries only (no kernel compilation)")
        set(CUDA_AVAILABLE FALSE)
        set(CUDA_RUNTIME_ONLY TRUE)
    else()
        message(WARNING "CUDAToolkit not found. CUDA support will be disabled.")
        set(BUILD_WITH_CUDA OFF)
        set(CUDA_AVAILABLE FALSE)
        set(CUDA_RUNTIME_ONLY FALSE)
    endif()
else()
    set(CUDA_AVAILABLE FALSE)
    set(CUDA_RUNTIME_ONLY FALSE)
endif()

# Настройки компилятора
if(MSVC)
    add_compile_options(/W4)
    # Отключаем предупреждения о deprecated функциях
    add_compile_definitions(_CRT_SECURE_NO_WARNINGS)

    # Исправление конфликтов LIBCMT - используем динамическую CRT
    set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>DLL")

    # Дополнительные флаги линкера для исправления конфликтов
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /NODEFAULTLIB:LIBCMT")
    set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} /NODEFAULTLIB:LIBCMT")
else()
    add_compile_options(-Wall -Wextra -Wpedantic)
endif()

# Определение макросов для адаптеров рендеринга
if(ENABLE_OPENGL_BACKEND)
    add_compile_definitions(HyperEngine_ENABLE_OPENGL)
    message(STATUS "OpenGL backend enabled")
endif()

if(DEFAULT_BACKEND_VULKAN)
    add_compile_definitions(HyperEngine_DEFAULT_BACKEND_VULKAN)
    message(STATUS "Vulkan set as default backend")
else()
    add_compile_definitions(HyperEngine_DEFAULT_BACKEND_OPENGL)
    message(STATUS "OpenGL set as default backend")
endif()

# Опции сборки уже определены выше

# Включение директорий
include_directories(${CMAKE_SOURCE_DIR}/include)

# Поиск пакетов
if(BUILD_ENGINE_4D OR BUILD_VULKAN_RENDERER)
    # Используем Vulkan из системы
    find_package(Vulkan REQUIRED)

    # Опциональные зависимости Vulkan
    find_package(VulkanMemoryAllocator CONFIG QUIET)
    
    # Fallback: создаем интерфейсный таргет для VMA если не найден через find_package
    if(NOT TARGET VulkanMemoryAllocator::VulkanMemoryAllocator AND NOT TARGET GPUOpen::VulkanMemoryAllocator)
        # Ищем заголовочный файл VMA
        find_path(VMA_INCLUDE_DIR
            NAMES vk_mem_alloc.h
            PATHS
                /usr/local/include/vma
                /usr/include/vma
                ${CMAKE_SOURCE_DIR}/external/vma/include
                ${CMAKE_SOURCE_DIR}/external/VulkanMemoryAllocator/include
        )
        
        if(VMA_INCLUDE_DIR)
            message(STATUS "VulkanMemoryAllocator found (header-only): ${VMA_INCLUDE_DIR}")
            add_library(VulkanMemoryAllocator INTERFACE)
            add_library(VulkanMemoryAllocator::VulkanMemoryAllocator ALIAS VulkanMemoryAllocator)
            target_include_directories(VulkanMemoryAllocator INTERFACE ${VMA_INCLUDE_DIR})
            target_link_libraries(VulkanMemoryAllocator INTERFACE Vulkan::Vulkan)
        else()
            message(STATUS "VulkanMemoryAllocator not found - VMA features will be disabled")
        endif()
    endif()
    
    find_package(spirv_cross_core CONFIG QUIET)
    find_package(unofficial-shaderc CONFIG QUIET)
    find_package(imgui CONFIG QUIET)
    find_package(assimp CONFIG QUIET)
    find_package(Stb QUIET)
endif()

# Find GLFW3 (window and input library)
find_package(glfw3 CONFIG REQUIRED)

# Find GLEW (OpenGL extension loader)
find_package(GLEW REQUIRED)

# Find OpenGL
find_package(OpenGL REQUIRED)

# Find GLM (mathematics library) - CONFIG mode or header-only fallback
find_package(glm CONFIG QUIET)

if(NOT glm_FOUND)
    # Fallback: поиск GLM как header-only библиотеки (для системной установки)
    find_path(GLM_INCLUDE_DIR 
        NAMES glm/glm.hpp
        PATHS 
            /usr/include
            /usr/local/include
            ${CMAKE_SOURCE_DIR}/external/glm
    )
    
    if(GLM_INCLUDE_DIR)
        message(STATUS "GLM found (header-only): ${GLM_INCLUDE_DIR}")
        # Создаем интерфейс библиотеку для GLM
        add_library(glm INTERFACE)
        add_library(glm::glm ALIAS glm)
        target_include_directories(glm INTERFACE ${GLM_INCLUDE_DIR})
    else()
        message(FATAL_ERROR "GLM not found. Install libglm-dev package.")
    endif()
else()
    message(STATUS "GLM found via CMake config")
endif()

# Добавление пути к CMake модулям
list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")

# Поиск CUDA Toolkit (опциональный)
if(BUILD_WITH_CUDA)
    # Проверяем наличие nvcc
    find_program(CUDA_NVCC_EXECUTABLE nvcc)

    if(CUDA_NVCC_EXECUTABLE)
        message(STATUS "CUDA compiler found: ${CUDA_NVCC_EXECUTABLE}")

        # Пытаемся найти CUDAToolkit для линковки библиотек
        find_package(CUDAToolkit QUIET)

        if(CUDAToolkit_FOUND)
            message(STATUS "CUDA Toolkit found: ${CUDAToolkit_VERSION}")

            # Устанавливаем переменную что CUDA доступен для определений препроцессора
            set(CUDA_AVAILABLE TRUE)

            # Ищем расширенную CUDA конфигурацию
            find_package(CUDAAdvanced QUIET)

        else()
            message(STATUS "CUDA compiler found but CUDAToolkit package not available")
            set(CUDA_AVAILABLE FALSE)
        endif()

    else()
        message(WARNING "CUDA compiler (nvcc) not found. Disabling CUDA support.")
        set(BUILD_WITH_CUDA OFF)
        set(CUDA_AVAILABLE FALSE)
    endif()
endif()

# Поиск OptiX SDK
if(BUILD_WITH_OPTIX)
    find_package(OptiX)
    if(OptiX_FOUND)
        message(STATUS "OptiX SDK found: ${OptiX_VERSION}")
    else()
        message(WARNING "OptiX SDK not found. Ray tracing features will be disabled.")
        set(BUILD_WITH_OPTIX OFF)
    endif()
endif()

# Поиск DLSS SDK
if(BUILD_WITH_DLSS)
    # ВАЖНО: DLSS (Streamline) официально поддерживает только Windows
    if(UNIX AND NOT APPLE)
        message(WARNING "DLSS (Streamline SDK) officially supports only Windows.")
        message(WARNING "For Linux, consider using FSR (FidelityFX Super Resolution) instead.")
        message(WARNING "DLSS support will be disabled. Use -DBUILD_WITH_DLSS=OFF to suppress this warning.")
        set(BUILD_WITH_DLSS OFF)
    else()
        find_package(DLSS)
        if(DLSS_FOUND)
            message(STATUS "DLSS SDK found: ${DLSS_ROOT_DIR} (version ${DLSS_VERSION})")
        else()
            message(WARNING "DLSS SDK not found. DLSS upscaling will be disabled.")
            set(BUILD_WITH_DLSS OFF)
        endif()
    endif()
endif()

# Поиск FSR SDK
if(BUILD_WITH_FSR)
    find_package(FSR)
    if(FSR_FOUND)
        message(STATUS "FSR SDK found")
    else()
        message(WARNING "FSR SDK not found. FSR upscaling will be disabled.")
        set(BUILD_WITH_FSR OFF)
    endif()
endif()

# ============================================================
# Сборка 3D движка
# ============================================================
if(BUILD_ENGINE_3D)
    if(ENABLE_VULKAN_BACKEND AND BUILD_VULKAN_RENDERER)
        add_compile_definitions(HyperEngine_ENABLE_VULKAN)
    endif()

    message(STATUS "Building HyperEngine with modular architecture...")

    # Подключаем новую модульную структуру
    add_subdirectory(src)

    # Создаем основную библиотеку HyperEngine, объединяющую все модули
    add_library(HyperEngine INTERFACE)

    # Подключаем все модули к основной библиотеке
    target_link_libraries(HyperEngine INTERFACE
        HyperEngine_Core
        HyperEngine_Math
        HyperEngine_OpenGL
        HyperEngine_Physics
        HyperEngine_Input
    )

    # Подключаем Vulkan модуль если включен
    if(ENABLE_VULKAN_BACKEND AND BUILD_VULKAN_RENDERER)
        target_link_libraries(HyperEngine INTERFACE HyperEngine_Vulkan)
        target_include_directories(HyperEngine INTERFACE ${VMA_INCLUDE_DIR})
    endif()

    # Подключаем опциональные модули
    if(BUILD_WITH_CUDA)
        target_link_libraries(HyperEngine INTERFACE HyperEngine_CUDA)
    endif()

    if(BUILD_WITH_OPTIX)
        target_link_libraries(HyperEngine INTERFACE HyperEngine_OptiX)
    endif()

    # Подключение внешних библиотек
    target_link_libraries(HyperEngine INTERFACE
        glfw
        GLEW::GLEW
        glm::glm
    )

    # Определения компилятора для 3D
    target_compile_definitions(HyperEngine INTERFACE ENGINE_3D_BUILD)

    # Установка 3D движка
    install(TARGETS HyperEngine
        RUNTIME DESTINATION bin
        LIBRARY DESTINATION lib
        ARCHIVE DESTINATION lib
    )

    # Создание демо для 3D
    if(BUILD_EXAMPLES)
        # Оригинальное 3D демо
        add_executable(HyperEngine_Demo
            examples/3d_demo.cpp
        )
        target_link_libraries(HyperEngine_Demo PRIVATE HyperEngine)

        # Новое демо оптимального рендерера
        add_executable(OptimalRenderer_Demo
            examples/optimal_renderer_demo.cpp
        )
        target_link_libraries(OptimalRenderer_Demo PRIVATE HyperEngine)

        # Демо UTF-8 консоли
        add_executable(UTF8Console_Demo
            examples/utf8_console_demo.cpp
        )
        target_link_libraries(UTF8Console_Demo PRIVATE HyperEngine)

        # Демо адаптеров рендеринга
        add_executable(RendererAdapter_Demo
            examples/renderer_adapter_demo.cpp
        )
        target_link_libraries(RendererAdapter_Demo PRIVATE HyperEngine)

        # Демо безопасной консоли
        add_executable(SafeConsole_Demo
            examples/safe_console_demo.cpp
        )
        target_link_libraries(SafeConsole_Demo PRIVATE HyperEngine)

        # SOLID Principles Demo
        add_executable(SOLIDPrinciples_Demo
            examples/solid_principles_demo.cpp
            src/core/EngineCore.cpp
            src/core/Logger.cpp
            src/rendering/ModernRenderer3D.cpp
        )
        target_link_libraries(SOLIDPrinciples_Demo PRIVATE HyperEngine)
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

    # Исключаем src файлы
    list(FILTER ENGINE4D_SOURCES EXCLUDE REGEX "src/.*")

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
    )

    if(TARGET VulkanMemoryAllocator::VulkanMemoryAllocator)
        target_link_libraries(Engine4D PUBLIC VulkanMemoryAllocator::VulkanMemoryAllocator)
    endif()

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
# Сборка Vulkan Hybrid Renderer
# ============================================================
if(BUILD_VULKAN_RENDERER)
    message(STATUS "Building Vulkan Hybrid Renderer...")

    # Сбор файлов для Vulkan рендерера
    # ТОЛЬКО rendering директория, остальное уже собрано в отдельные модули в src/CMakeLists.txt
    file(GLOB_RECURSE VULKAN_CPP_SOURCES 
        "src/rendering/*.cpp"
    )
    file(GLOB_RECURSE VULKAN_HEADERS 
        "src/rendering/*.h"
    )

    # Сбор CUDA файлов отдельно (если они нужны)
    file(GLOB_RECURSE VULKAN_CUDA_CPP_SOURCES "src/cuda/*.cpp")
    file(GLOB_RECURSE VULKAN_CUDA_CU_SOURCES "src/cuda/*.cu")
    
    # Исключаем CUDA файлы если CUDA выключена
    if(NOT BUILD_WITH_CUDA)
        # Не добавляем CUDA файлы к VULKAN_CPP_SOURCES
        message(STATUS "Excluding CUDA files from build (BUILD_WITH_CUDA=OFF)")
    else()
        # Добавляем CUDA cpp файлы если CUDA включена
        list(APPEND VULKAN_CPP_SOURCES ${VULKAN_CUDA_CPP_SOURCES})
        message(STATUS "Including CUDA cpp files in build")
    endif()
    
    # Сбор OptiX файлов отдельно (если они нужны)
    file(GLOB_RECURSE VULKAN_OPTIX_SOURCES "src/optix/*.cpp")
    
    # Исключаем OptiX файлы если OptiX выключен
    if(NOT BUILD_WITH_OPTIX)
        message(STATUS "Excluding OptiX files from build (BUILD_WITH_OPTIX=OFF)")
    else()
        # Добавляем OptiX файлы если OptiX включен
        list(APPEND VULKAN_CPP_SOURCES ${VULKAN_OPTIX_SOURCES})
        message(STATUS "Including OptiX files in build")
    endif()

    # Объединяем исходники
    set(VULKAN_SOURCES ${VULKAN_CPP_SOURCES} ${VULKAN_HEADERS})

    # Приоритет библиотекам над объектными файлами для лучшей линковки
    if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/cuda_build/cuda_kernels_partial.lib")
        # Если есть библиотека но нет отдельных объектных файлов
        set(CUDA_LIBRARY_FILE "${CMAKE_CURRENT_SOURCE_DIR}/cuda_build/cuda_kernels_partial.lib")
        message(STATUS "Using compiled CUDA library: ${CUDA_LIBRARY_FILE}")
        set(USING_COMPILED_CUDA_LIBRARY TRUE)
    elseif(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/cuda_build/gaussian_optimization.obj")
        # Fallback: используем объектные файлы если нет библиотеки
        set(CUDA_OBJECT_FILES)
        list(APPEND CUDA_OBJECT_FILES
            "${CMAKE_CURRENT_SOURCE_DIR}/cuda_build/gaussian_optimization.obj"
            "${CMAKE_CURRENT_SOURCE_DIR}/cuda_build/tile_rasterization.obj"
            "${CMAKE_CURRENT_SOURCE_DIR}/cuda_build/depth_sorting.obj"
        )
        message(STATUS "Using compiled CUDA object files: ${CUDA_OBJECT_FILES}")
        set(USING_COMPILED_CUDA_OBJECTS TRUE)
    endif()

    # Создание библиотеки Vulkan рендерера
    add_library(VulkanRenderer STATIC ${VULKAN_SOURCES})

    if(WIN32)
        target_compile_definitions(VulkanRenderer PUBLIC VK_USE_PLATFORM_WIN32_KHR)
    endif()

    target_compile_definitions(VulkanRenderer PUBLIC HyperEngine_ENABLE_VULKAN)

    # Устанавливаем CUDA файлы как CUDA язык если доступен
    if(CUDA_AVAILABLE AND VULKAN_CUDA_CU_SOURCES)
        set_source_files_properties(${VULKAN_CUDA_CU_SOURCES} PROPERTIES LANGUAGE CUDA)
        message(STATUS "Set CUDA language for: ${VULKAN_CUDA_CU_SOURCES}")
    endif()

    # Настройка CUDA для VulkanRenderer если доступен
    if(CUDA_AVAILABLE)
        # Базовые CUDA настройки для полной компиляции
        set_property(TARGET VulkanRenderer PROPERTY CUDA_STANDARD 17)
        set_property(TARGET VulkanRenderer PROPERTY CUDA_STANDARD_REQUIRED ON)
        set_property(TARGET VulkanRenderer PROPERTY CUDA_SEPARABLE_COMPILATION ON)
        set_property(TARGET VulkanRenderer PROPERTY CUDA_RESOLVE_DEVICE_SYMBOLS ON)
        set_property(TARGET VulkanRenderer PROPERTY CUDA_ARCHITECTURES "${CMAKE_CUDA_ARCHITECTURES}")

        # CUDA compile definitions
        target_compile_definitions(VulkanRenderer PRIVATE
            CUDA_VULKAN_INTEROP_SUPPORTED=1
            VULKAN_RENDERER_CUDA_SUPPORT=1
        )

        # Добавляем CUDA include директории
        find_package(CUDAToolkit REQUIRED)
        target_include_directories(VulkanRenderer PRIVATE
            ${CUDAToolkit_INCLUDE_DIRS}
        )
    elseif(USING_COMPILED_CUDA_OBJECTS OR USING_COMPILED_CUDA_LIBRARY)
        # CUDA поддержка через скомпилированные объектные файлы/библиотеки
        target_compile_definitions(VulkanRenderer PRIVATE
            CUDA_VULKAN_INTEROP_SUPPORTED=1
            VULKAN_RENDERER_CUDA_SUPPORT=1
            USING_PRECOMPILED_CUDA=1
        )

        # Добавляем CUDA include пути для скомпилированных файлов
        # Всегда ищем CUDA toolkit для линковки runtime библиотек
        find_package(CUDAToolkit REQUIRED)
        target_include_directories(VulkanRenderer PRIVATE
            ${CUDAToolkit_INCLUDE_DIRS}
        )
        target_link_libraries(VulkanRenderer PUBLIC CUDA::cudart CUDA::cuda_driver)

        message(STATUS "VulkanRenderer: CUDA поддержка через скомпилированные файлы включена")
    elseif(CUDA_RUNTIME_ONLY)
        # Runtime CUDA поддержка без компиляции kernels
        target_compile_definitions(VulkanRenderer PRIVATE
            CUDA_VULKAN_INTEROP_SUPPORTED=1
            VULKAN_RENDERER_CUDA_SUPPORT=1
        )

        # Добавляем CUDA include пути для runtime
        find_package(CUDAToolkit REQUIRED)
        target_include_directories(VulkanRenderer PRIVATE
            ${CUDAToolkit_INCLUDE_DIRS}
        )
        
        # Линковка только runtime CUDA библиотек
        target_link_libraries(VulkanRenderer PUBLIC CUDA::cudart CUDA::cuda_driver)

        message(STATUS "VulkanRenderer: Runtime CUDA поддержка включена (stubs + runtime)")
    else()
        message(STATUS "VulkanRenderer: CUDA поддержка отключена - используются stubs")
    endif()

    # Подключение заголовочных файлов
    target_include_directories(VulkanRenderer PUBLIC
        ${CMAKE_SOURCE_DIR}/include
        ${CMAKE_SOURCE_DIR}/include/HyperEngine
        ${CMAKE_SOURCE_DIR}/include/HyperEngine/Vulkan
        ${CMAKE_SOURCE_DIR}/include/HyperEngine/CUDA
        ${CMAKE_SOURCE_DIR}/include/HyperEngine/OptiX
        ${CMAKE_SOURCE_DIR}/include/HyperEngine/Upscaling
        ${VMA_INCLUDE_DIR}
    )

    # Линковка скомпилированных CUDA файлов
    if(USING_COMPILED_CUDA_OBJECTS)
        # Добавляем объектные файлы к target
        target_sources(VulkanRenderer PRIVATE ${CUDA_OBJECT_FILES})
        
        # Добавляем CUDA include пути для использования с объектными файлами
        find_package(CUDAToolkit REQUIRED)
        target_include_directories(VulkanRenderer PRIVATE ${CUDAToolkit_INCLUDE_DIRS})
        target_link_libraries(VulkanRenderer PUBLIC CUDA::cudart CUDA::cuda_driver)
        
        message(STATUS "Linked CUDA object files to VulkanRenderer")
    elseif(USING_COMPILED_CUDA_LIBRARY)
        # Линкуем библиотеку
        target_link_libraries(VulkanRenderer PUBLIC ${CUDA_LIBRARY_FILE})
        
        # Добавляем CUDA include пути для использования с библиотекой
        find_package(CUDAToolkit REQUIRED)
        target_include_directories(VulkanRenderer PRIVATE ${CUDAToolkit_INCLUDE_DIRS})
        target_link_libraries(VulkanRenderer PUBLIC CUDA::cudart CUDA::cuda_driver)
        
        message(STATUS "Linked CUDA library to VulkanRenderer")
    endif()

    # Базовые библиотеки для Vulkan рендерера
    target_link_libraries(VulkanRenderer PUBLIC
        HyperEngine_Core
        HyperEngine_Math
        HyperEngine_Physics
        HyperEngine_Input
        HyperEngine_Upscaling
        Vulkan::Vulkan
        glfw
        glm::glm
    )
    
    # Опционально линкуем OpenGL если он доступен
    if(TARGET HyperEngine_OpenGL)
        target_link_libraries(VulkanRenderer PUBLIC HyperEngine_OpenGL)
    endif()

    # Опциональные зависимости Vulkan
    if(TARGET VulkanMemoryAllocator::VulkanMemoryAllocator)
        target_link_libraries(VulkanRenderer PUBLIC VulkanMemoryAllocator::VulkanMemoryAllocator)
    endif()
    if(TARGET spirv_cross_core::spirv_cross_core)
        target_link_libraries(VulkanRenderer PUBLIC spirv_cross_core::spirv_cross_core)
    endif()
    if(TARGET unofficial::shaderc::shaderc)
        target_link_libraries(VulkanRenderer PUBLIC unofficial::shaderc::shaderc)
    endif()
    if(TARGET imgui::imgui)
        target_link_libraries(VulkanRenderer PUBLIC imgui::imgui)
    endif()
    if(TARGET assimp::assimp)
        target_link_libraries(VulkanRenderer PUBLIC assimp::assimp)
    endif()
    # Подключение stb через include directories
    target_include_directories(VulkanRenderer PRIVATE ${Stb_INCLUDE_DIR})


    # OptiX поддержка
    if(BUILD_WITH_OPTIX AND OptiX_FOUND)
        target_include_directories(VulkanRenderer PRIVATE ${OptiX_INCLUDE_DIRS})
        target_link_libraries(VulkanRenderer PRIVATE ${OptiX_LIBRARIES})
        target_compile_definitions(VulkanRenderer PUBLIC VULKAN_RENDERER_OPTIX_SUPPORT)
        # OptiX требует CUDA, поэтому добавляем макрос только если CUDA включен
        if(BUILD_WITH_CUDA)
            target_compile_definitions(VulkanRenderer PUBLIC CUDA_VULKAN_INTEROP_SUPPORTED)
        endif()
    endif()

    # DLSS поддержка
    if(BUILD_WITH_DLSS AND DLSS_FOUND)
        target_include_directories(VulkanRenderer PRIVATE ${DLSS_INCLUDE_DIRS})
        target_link_libraries(VulkanRenderer PRIVATE ${DLSS_LIBRARIES})
        target_compile_definitions(VulkanRenderer PUBLIC VULKAN_RENDERER_DLSS_SUPPORT)
    endif()

    # FSR поддержка
    if(BUILD_WITH_FSR AND FSR_FOUND)
        target_include_directories(VulkanRenderer PRIVATE ${FSR_INCLUDE_DIRS})
        target_link_libraries(VulkanRenderer PRIVATE ${FSR_LIBRARIES})
        target_compile_definitions(VulkanRenderer PUBLIC VULKAN_RENDERER_FSR_SUPPORT)
    endif()

    # Определения компилятора для Vulkan рендерера
    target_compile_definitions(VulkanRenderer PUBLIC VULKAN_RENDERER_BUILD)

    # Настройки CUDA для .cu файлов выполняются в configure_cuda_target()

    # Установка Vulkan рендерера
    install(TARGETS VulkanRenderer
        RUNTIME DESTINATION bin
        LIBRARY DESTINATION lib
        ARCHIVE DESTINATION lib
    )

    # Создание демо для Vulkan рендерера
    if(BUILD_EXAMPLES)
        add_executable(VulkanRenderer_Demo
            examples/vulkan_demo.cpp
        )
        target_link_libraries(VulkanRenderer_Demo PRIVATE
            VulkanRenderer
            HyperEngine
        )

        # Добавляем OptiX include directories если OptiX доступен
        if(BUILD_WITH_OPTIX AND OptiX_FOUND)
            target_include_directories(VulkanRenderer_Demo PRIVATE ${OptiX_INCLUDE_DIRS})
            message(STATUS "VulkanRenderer_Demo: добавлены OptiX include directories")
        endif()

        # Добавляем CUDA библиотеки к VulkanRenderer_Demo если CUDA доступна
        if(CUDA_AVAILABLE OR CUDA_RUNTIME_ONLY OR USING_COMPILED_CUDA_OBJECTS OR USING_COMPILED_CUDA_LIBRARY)
            find_package(CUDAToolkit REQUIRED)
            target_link_libraries(VulkanRenderer_Demo PRIVATE CUDA::cudart CUDA::cuda_driver)
            message(STATUS "VulkanRenderer_Demo: добавлены CUDA библиотеки")
        endif()

        # Базовое демо новой Vulkan архитектуры (этап 2.1)
        add_executable(VulkanBasic_Demo
            examples/vulkan_basic_demo.cpp
        )
        target_link_libraries(VulkanBasic_Demo PRIVATE
            VulkanRenderer
            HyperEngine
        )

        # Добавляем CUDA библиотеки к VulkanBasic_Demo если CUDA доступна
        if(CUDA_AVAILABLE OR CUDA_RUNTIME_ONLY OR USING_COMPILED_CUDA_OBJECTS OR USING_COMPILED_CUDA_LIBRARY)
            target_link_libraries(VulkanBasic_Demo PRIVATE CUDA::cudart CUDA::cuda_driver)
            message(STATUS "VulkanBasic_Demo: добавлены CUDA библиотеки")
        endif()

        # CUDA-Vulkan Interop Demo (требует CUDA)
        if(CUDA_AVAILABLE OR CUDA_RUNTIME_ONLY)
            add_executable(CudaVulkanInterop_Demo
                examples/cuda_vulkan_interop_demo.cpp
            )
            target_link_libraries(CudaVulkanInterop_Demo PRIVATE
                VulkanRenderer
                HyperEngine
                CUDA::cudart
                CUDA::cuda_driver
            )

            # Настройка CUDA для demo
            target_compile_definitions(CudaVulkanInterop_Demo PRIVATE CUDA_VULKAN_INTEROP_SUPPORTED=1)

            # External Memory Test - тест реальных external memory handles
            add_executable(ExternalMemory_Test
                examples/test_external_memory.cpp
            )
            target_link_libraries(ExternalMemory_Test PRIVATE
                VulkanRenderer
                HyperEngine
                CUDA::cudart
                CUDA::cuda_driver
            )
            target_compile_definitions(ExternalMemory_Test PRIVATE CUDA_VULKAN_INTEROP_SUPPORTED=1)

            # FlashGS Demo - демонстрация 3D Gaussian Splatting
            add_executable(FlashGS_Demo
                examples/flashgs_demo.cpp
            )
            target_link_libraries(FlashGS_Demo PRIVATE
                VulkanRenderer
                HyperEngine
                CUDA::cudart
                CUDA::cuda_driver
            )

            # Настройка CUDA для FlashGS demo
            target_compile_definitions(FlashGS_Demo PRIVATE CUDA_VULKAN_INTEROP_SUPPORTED=1)

            # OptiX Ray Tracer Demo - демонстрация OptiX ray tracing
            if(BUILD_WITH_OPTIX AND OptiX_FOUND)
                add_executable(OptiXRayTracer_Demo
                    examples/optix_raytracer_demo.cpp
                )
                target_link_libraries(OptiXRayTracer_Demo PRIVATE
                    VulkanRenderer
                    HyperEngine
                    CUDA::cudart
                    CUDA::cuda_driver
                )
                target_include_directories(OptiXRayTracer_Demo PRIVATE ${OptiX_INCLUDE_DIRS})
                target_compile_definitions(OptiXRayTracer_Demo PRIVATE
                    CUDA_VULKAN_INTEROP_SUPPORTED=1
                    VULKAN_RENDERER_OPTIX_SUPPORT=1
                )
                message(STATUS "OptiX Ray Tracer Demo будет собран")
            else()
                message(STATUS "OptiX Ray Tracer Demo отключен - OptiX не доступен")
            endif()

            if(CUDA_AVAILABLE)
                message(STATUS "CUDA демо будут собраны с полной CUDA поддержкой")
            else()
                message(STATUS "CUDA демо будут собраны с runtime CUDA поддержкой (используют stubs)")
            endif()
        else()
            message(STATUS "CUDA демо отключены - CUDA не доступна или отключена")
        endif()
    endif()
endif()

# ============================================================
# Тесты
# ============================================================
if(BUILD_TESTS)
    enable_testing()

    if(BUILD_ENGINE_3D)
        add_subdirectory(tests)
    endif()

endif()

# ============================================================
# Установка заголовочных файлов
# ============================================================
if(BUILD_ENGINE_3D)
    install(DIRECTORY include/HyperEngine/ DESTINATION include/HyperEngine)
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
message(STATUS "Build Vulkan Renderer: ${BUILD_VULKAN_RENDERER}")
message(STATUS "Build with CUDA: ${BUILD_WITH_CUDA}")
message(STATUS "Build with OptiX: ${BUILD_WITH_OPTIX}")
message(STATUS "Build with DLSS: ${BUILD_WITH_DLSS}")
message(STATUS "Build with FSR: ${BUILD_WITH_FSR}")
message(STATUS "Build Examples: ${BUILD_EXAMPLES}")
message(STATUS "Build Tests: ${BUILD_TESTS}")
message(STATUS "C++ Standard: ${CMAKE_CXX_STANDARD}")
if(BUILD_WITH_CUDA)
    message(STATUS "CUDA Standard: ${CMAKE_CUDA_STANDARD}")
endif()
message(STATUS "Compiler: ${CMAKE_CXX_COMPILER_ID}")
message(STATUS "========================================")
message(STATUS "")
