cmake_minimum_required(VERSION 3.16)

# Определяем языки в зависимости от опций
set(PROJECT_LANGUAGES CXX)

# Опции сборки (определяем рано, чтобы использовать в project())
option(BUILD_ENGINE_3D "Build 3D Engine" ON)
option(BUILD_ENGINE_4D "Build 4D Engine" OFF)
option(BUILD_VULKAN_RENDERER "Build Vulkan-based Hybrid Renderer" ON)
option(BUILD_WITH_CUDA "Build with CUDA support" ON)
option(BUILD_WITH_OPTIX "Build with OptiX support" ON)
option(BUILD_WITH_DLSS "Build with DLSS support" ON)
option(BUILD_WITH_FSR "Build with FSR support" ON)
option(BUILD_EXAMPLES "Build example applications" ON)
option(BUILD_TESTS "Build test suite" ON)

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
endif()

# Опции для адаптеров рендеринга
option(ENABLE_OPENGL_BACKEND "Enable OpenGL rendering backend" ON)
option(ENABLE_VULKAN_BACKEND "Enable Vulkan rendering backend" ON)
option(DEFAULT_BACKEND_VULKAN "Use Vulkan as default backend (otherwise OpenGL)" ON)

# Создаем проект с поддерживаемыми языками
project(HyperEngine VERSION 1.0.0 LANGUAGES ${PROJECT_LANGUAGES})

# Установка стандарта C++
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

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

if(ENABLE_VULKAN_BACKEND AND BUILD_VULKAN_RENDERER)
    add_compile_definitions(HyperEngine_ENABLE_VULKAN)
    message(STATUS "Vulkan backend enabled")
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
    # Используем Vulkan из vcpkg
    find_package(Vulkan REQUIRED)
    find_package(VulkanMemoryAllocator CONFIG REQUIRED)
    find_package(spirv_cross_core CONFIG REQUIRED)
    find_package(unofficial-shaderc CONFIG REQUIRED)
    find_package(imgui CONFIG REQUIRED)
    find_package(assimp CONFIG REQUIRED)
    find_package(Stb REQUIRED)
endif()

# Поиск GLFW3 через vcpkg
find_package(glfw3 CONFIG REQUIRED)

# Поиск GLEW через vcpkg
find_package(GLEW REQUIRED)

# Поиск GLM через vcpkg
find_package(glm CONFIG REQUIRED)

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
    find_package(DLSS)
    if(DLSS_FOUND)
        message(STATUS "DLSS SDK found")
    else()
        message(WARNING "DLSS SDK not found. DLSS upscaling will be disabled.")
        set(BUILD_WITH_DLSS OFF)
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
        GPUOpen::VulkanMemoryAllocator
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
# Сборка Vulkan Hybrid Renderer
# ============================================================
if(BUILD_VULKAN_RENDERER)
    message(STATUS "Building Vulkan Hybrid Renderer...")
    
    # Сбор файлов для Vulkan рендерера
    file(GLOB_RECURSE VULKAN_CPP_SOURCES "src/*.cpp")
    file(GLOB_RECURSE VULKAN_HEADERS "src/*.h")
    
    # Отдельно собираем CUDA файлы
    file(GLOB_RECURSE VULKAN_CUDA_SOURCES "src/*.cu")
    
    # Объединяем исходники
    set(VULKAN_SOURCES ${VULKAN_CPP_SOURCES} ${VULKAN_HEADERS})
    
    # Приоритет библиотекам над объектными файлами для лучшей линковки
    if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/cuda_build/cuda_kernels_partial.lib")
        # Если есть библиотека но нет отдельных объектных файлов
        set(CUDA_LIBRARY_FILE "${CMAKE_CURRENT_SOURCE_DIR}/cuda_build/cuda_kernels_partial.lib")
        list(REMOVE_ITEM VULKAN_CPP_SOURCES "${CMAKE_SOURCE_DIR}/src/CUDA/cuda_stubs.cpp")
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
        # Исключаем заглушки так как у нас есть реальные функции
        list(REMOVE_ITEM VULKAN_SOURCES "src/CUDA/cuda_stubs.cpp")
        list(REMOVE_ITEM VULKAN_SOURCES "${CMAKE_SOURCE_DIR}/src/CUDA/cuda_stubs.cpp")
        message(STATUS "Using compiled CUDA object files: ${CUDA_OBJECT_FILES}")
        set(USING_COMPILED_CUDA_OBJECTS TRUE)
    else()
        # Fallback: Добавляем CUDA файлы или заглушки
        if(CUDA_AVAILABLE AND VULKAN_CUDA_SOURCES)
            # Исключаем заглушки и добавляем реальные CUDA файлы
            list(REMOVE_ITEM VULKAN_CPP_SOURCES "${CMAKE_SOURCE_DIR}/src/CUDA/cuda_stubs.cpp")
            list(APPEND VULKAN_SOURCES ${VULKAN_CUDA_SOURCES})
            message(STATUS "Building with CUDA kernels: ${VULKAN_CUDA_SOURCES}")
        elseif(CUDA_RUNTIME_ONLY)
            # Добавляем заглушки, но с runtime поддержкой
            list(APPEND VULKAN_SOURCES "src/CUDA/cuda_stubs.cpp")
            message(STATUS "Building with CUDA stubs but with runtime libraries")
        else()
            # Добавляем заглушки CUDA если CUDA недоступна
            list(APPEND VULKAN_SOURCES "src/CUDA/cuda_stubs.cpp")
            message(STATUS "Building with CUDA stubs (CUDA not available)")
        endif()
    endif()
    
    # Создание библиотеки Vulkan рендерера
    add_library(VulkanRenderer STATIC ${VULKAN_SOURCES})
    
    # Устанавливаем CUDA файлы как CUDA язык если доступен
    if(CUDA_AVAILABLE AND VULKAN_CUDA_SOURCES)
        set_source_files_properties(${VULKAN_CUDA_SOURCES} PROPERTIES LANGUAGE CUDA)
        message(STATUS "Set CUDA language for: ${VULKAN_CUDA_SOURCES}")
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
    )
    
    # Линковка скомпилированных CUDA файлов
    if(USING_COMPILED_CUDA_OBJECTS)
        # Добавляем объектные файлы к target
        target_sources(VulkanRenderer PRIVATE ${CUDA_OBJECT_FILES})
        message(STATUS "Linked CUDA object files to VulkanRenderer")
    elseif(USING_COMPILED_CUDA_LIBRARY)
        # Линкуем библиотеку
        target_link_libraries(VulkanRenderer PUBLIC ${CUDA_LIBRARY_FILE})
        message(STATUS "Linked CUDA library to VulkanRenderer")
    endif()

    # Базовые библиотеки для Vulkan рендерера
    target_link_libraries(VulkanRenderer PUBLIC 
        Vulkan::Vulkan
        GPUOpen::VulkanMemoryAllocator
        spirv-cross-core
        unofficial::shaderc::shaderc
        imgui::imgui
        assimp::assimp
        glfw
        glm::glm
    )
    
    # Подключение stb через include directories
    target_include_directories(VulkanRenderer PRIVATE ${Stb_INCLUDE_DIR})
    
    
    # OptiX поддержка
    if(BUILD_WITH_OPTIX AND OptiX_FOUND)
        target_include_directories(VulkanRenderer PRIVATE ${OptiX_INCLUDE_DIRS})
        target_link_libraries(VulkanRenderer PRIVATE ${OptiX_LIBRARIES})
        target_compile_definitions(VulkanRenderer PUBLIC VULKAN_RENDERER_OPTIX_SUPPORT CUDA_VULKAN_INTEROP_SUPPORTED)
    endif()
    
    # DLSS поддержка
    if(BUILD_WITH_DLSS AND DLSS_FOUND)
        target_include_directories(VulkanRenderer PRIVATE ${DLSS_INCLUDE_DIRS})
        target_link_libraries(VulkanRenderer PRIVATE ${DLSS_LIBRARIES})
        target_compile_definitions(VulkanRenderer PUBLIC VULKAN_RENDERER_DLSS_SUPPORT CUDA_VULKAN_INTEROP_SUPPORTED)
    endif()
    
    # FSR поддержка
    if(BUILD_WITH_FSR AND FSR_FOUND)
        target_include_directories(VulkanRenderer PRIVATE ${FSR_INCLUDE_DIRS})
        target_link_libraries(VulkanRenderer PRIVATE ${FSR_LIBRARIES})
        target_compile_definitions(VulkanRenderer PUBLIC VULKAN_RENDERER_FSR_SUPPORT CUDA_VULKAN_INTEROP_SUPPORTED)
    endif()
    
    # Определения компилятора для Vulkan рендерера
    target_compile_definitions(VulkanRenderer PUBLIC VULKAN_RENDERER_BUILD CUDA_VULKAN_INTEROP_SUPPORTED)
    
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

