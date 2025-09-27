cmake_minimum_required(VERSION 3.16)

# Определяем языки в зависимости от опций
set(PROJECT_LANGUAGES CXX)

# Опции сборки (определяем рано, чтобы использовать в project())
option(BUILD_ENGINE_3D "Build 3D Engine" ON)
option(BUILD_ENGINE_4D "Build 4D Engine" OFF)
option(BUILD_VULKAN_RENDERER "Build Vulkan-based Hybrid Renderer" ON)
option(BUILD_WITH_CUDA "Build with CUDA support" OFF)
option(BUILD_WITH_OPTIX "Build with OptiX support" ON)
option(BUILD_WITH_DLSS "Build with DLSS support" OFF)
option(BUILD_WITH_FSR "Build with FSR support" ON)
option(BUILD_EXAMPLES "Build example applications" ON)
option(BUILD_TESTS "Build test suite" OFF)

# Сначала создаем проект только с C++
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

# Опции сборки уже определены выше

# Включение директорий
include_directories(${CMAKE_SOURCE_DIR}/include)

# Поиск пакетов
if(BUILD_ENGINE_4D OR BUILD_VULKAN_RENDERER)
    # Используем Vulkan из vcpkg вместо системного
    find_package(VulkanHeaders CONFIG REQUIRED)
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

# Поиск CUDA Toolkit (опциональный)
if(BUILD_WITH_CUDA)
    find_package(CUDAToolkit QUIET)
    
    if(CUDAToolkit_FOUND)
        # Расширенная настройка CUDA
        find_package(CUDAAdvanced)
        
        if(CUDA_ADVANCED_FOUND)
            # Включаем CUDA язык после проверки
            enable_language(CUDA)
            message(STATUS "CUDA Toolkit found: ${CUDAToolkit_VERSION}")
        else()
            message(WARNING "CUDA Advanced configuration failed. Disabling CUDA support.")
            set(BUILD_WITH_CUDA OFF)
        endif()
    else()
        message(WARNING "CUDA Toolkit not found. Disabling CUDA support.")
        set(BUILD_WITH_CUDA OFF)
    endif()
endif()

# Добавление пути к CMake модулям
list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")

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
        GLEW::GLEW
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
    
    # Создание демо для 3D
    if(BUILD_EXAMPLES)
        # Оригинальное 3D демо
        add_executable(Engine3D_Demo 
            examples/3d_demo.cpp
        )
        target_link_libraries(Engine3D_Demo PRIVATE Engine3D)
        
        # Новое демо оптимального рендерера
        add_executable(OptimalRenderer_Demo 
            examples/optimal_renderer_demo.cpp
        )
        target_link_libraries(OptimalRenderer_Demo PRIVATE Engine3D)
        
        # Демо UTF-8 консоли
        add_executable(UTF8Console_Demo 
            examples/utf8_console_demo.cpp
        )
        target_link_libraries(UTF8Console_Demo PRIVATE Engine3D)
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
        Vulkan::Headers
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
    file(GLOB_RECURSE VULKAN_SOURCES 
        "srcVulkan/*.cpp"
        "srcVulkan/*.cu"
        "srcVulkan/*.h"
    )
    
    # Создание библиотеки Vulkan рендерера
    add_library(VulkanRenderer STATIC ${VULKAN_SOURCES})
    
    # Подключение заголовочных файлов
    target_include_directories(VulkanRenderer PUBLIC 
        ${CMAKE_SOURCE_DIR}/include
        ${CMAKE_SOURCE_DIR}/include/Engine3D
        ${CMAKE_SOURCE_DIR}/include/Engine3D/Vulkan
        ${CMAKE_SOURCE_DIR}/include/Engine3D/CUDA
        ${CMAKE_SOURCE_DIR}/include/Engine3D/OptiX
        ${CMAKE_SOURCE_DIR}/include/Engine3D/Upscaling
    )
    
    # Базовые библиотеки для Vulkan рендерера
    target_link_libraries(VulkanRenderer PUBLIC 
        Vulkan::Headers
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
    
    # CUDA поддержка
    if(BUILD_WITH_CUDA AND CUDA_ADVANCED_FOUND)
        configure_cuda_target(VulkanRenderer)
        target_compile_definitions(VulkanRenderer PUBLIC VULKAN_RENDERER_CUDA_SUPPORT)
    endif()
    
    # OptiX поддержка
    if(BUILD_WITH_OPTIX AND OptiX_FOUND)
        target_include_directories(VulkanRenderer PRIVATE ${OptiX_INCLUDE_DIRS})
        target_link_libraries(VulkanRenderer PRIVATE ${OptiX_LIBRARIES})
        target_compile_definitions(VulkanRenderer PUBLIC VULKAN_RENDERER_OPTIX_SUPPORT)
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
        target_link_libraries(VulkanRenderer_Demo PRIVATE VulkanRenderer)
        
        # Базовое демо новой Vulkan архитектуры (этап 2.1)
        add_executable(VulkanBasic_Demo 
            examples/vulkan_basic_demo.cpp
        )
        target_link_libraries(VulkanBasic_Demo PRIVATE 
            VulkanRenderer
            Engine3D
            Vulkan::Headers
            GPUOpen::VulkanMemoryAllocator
        )
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
