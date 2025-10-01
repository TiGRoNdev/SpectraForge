# HyperEngine - Новая модульная структура

# Core модуль
file(GLOB_RECURSE CORE_SOURCES "core/*.cpp" "core/*.h")
add_library(HyperEngine_Core STATIC ${CORE_SOURCES})

# Math модуль
add_subdirectory(math)

# Rendering модули
file(GLOB_RECURSE RENDERING_COMMON_SOURCES "rendering/common/*.cpp" "rendering/common/*.h")
file(GLOB_RECURSE RENDERING_OPENGL_SOURCES "rendering/opengl/*.cpp" "rendering/opengl/*.h")

# Создаем библиотеки только если есть исходные файлы
if(RENDERING_COMMON_SOURCES)
    add_library(HyperEngine_RenderingCommon STATIC ${RENDERING_COMMON_SOURCES})
endif()

if(RENDERING_OPENGL_SOURCES)
    add_library(HyperEngine_OpenGL STATIC ${RENDERING_OPENGL_SOURCES})
endif()

# Vulkan модуль с отдельным CMakeLists.txt
if(BUILD_VULKAN_RENDERER AND ENABLE_VULKAN_BACKEND)
    add_subdirectory(rendering/vulkan)
endif()

# Physics модуль
file(GLOB_RECURSE PHYSICS_SOURCES "physics/*.cpp" "physics/*.h")
add_library(HyperEngine_Physics STATIC ${PHYSICS_SOURCES})

# Input модуль
file(GLOB_RECURSE INPUT_SOURCES "input/*.cpp" "input/*.h")
add_library(HyperEngine_Input STATIC ${INPUT_SOURCES})

# CUDA модуль (опционально)
if(BUILD_WITH_CUDA)
    file(GLOB_RECURSE CUDA_SOURCES "cuda/*.cpp" "cuda/*.cu" "cuda/*.h")
    
    # Временно исключаем файлы, требующие Vulkan-CUDA interop
    list(FILTER CUDA_SOURCES EXCLUDE REGEX ".*CudaInterop\\.cpp$")
    list(FILTER CUDA_SOURCES EXCLUDE REGEX ".*FlashGSSplatter\\.cpp$")
    
    add_library(HyperEngine_CUDA STATIC ${CUDA_SOURCES})
    set_property(TARGET HyperEngine_CUDA PROPERTY CUDA_SEPARABLE_COMPILATION ON)
    
    # НЕ добавляем CUDA_VULKAN_INTEROP_SUPPORTED из-за несовместимости nvcc с Vulkan headers
    # TODO: Включить когда будет решена проблема с constexpr в Vulkan-Hpp
endif()

# OptiX модуль (опционально)
if(BUILD_WITH_OPTIX)
    file(GLOB_RECURSE OPTIX_SOURCES "optix/*.cpp" "optix/*.h")
    # Добавляем OptiX stubs initialization отдельно
    list(APPEND OPTIX_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/optix/optix_init.cpp")
    add_library(HyperEngine_OptiX STATIC ${OPTIX_SOURCES})
    
    # Добавляем CUDA и OptiX include directories
    find_package(CUDAToolkit REQUIRED)
    target_include_directories(HyperEngine_OptiX PUBLIC
        ${CMAKE_SOURCE_DIR}/include
        ${CUDAToolkit_INCLUDE_DIRS}
        ${OptiX_INCLUDE_DIRS}
    )
    target_link_libraries(HyperEngine_OptiX PUBLIC CUDA::cudart CUDA::cuda_driver)
endif()

# Upscaling модуль
file(GLOB_RECURSE UPSCALING_SOURCES "upscaling/*.cpp" "upscaling/*.h")
add_library(HyperEngine_Upscaling STATIC ${UPSCALING_SOURCES})

# Настройка зависимостей
target_link_libraries(HyperEngine_Core PUBLIC HyperEngine_Math)

if(TARGET HyperEngine_OpenGL)
    target_link_libraries(HyperEngine_OpenGL PUBLIC 
        HyperEngine_Core 
        HyperEngine_Math
        GLEW::GLEW
        glfw
        glm::glm
        OpenGL::GL
    )
endif()

# Vulkan зависимости настраиваются в его собственном CMakeLists.txt

target_link_libraries(HyperEngine_Physics PUBLIC HyperEngine_Math)
target_link_libraries(HyperEngine_Input PUBLIC HyperEngine_Core glfw)

# Include директории
set(COMMON_TARGETS HyperEngine_Core HyperEngine_Math HyperEngine_Physics HyperEngine_Input HyperEngine_Upscaling)

# Добавляем условные цели
if(TARGET HyperEngine_OpenGL)
    list(APPEND COMMON_TARGETS HyperEngine_OpenGL)
endif()
if(TARGET HyperEngine_Vulkan)
    list(APPEND COMMON_TARGETS HyperEngine_Vulkan)
endif()

foreach(target ${COMMON_TARGETS})
    target_include_directories(${target}
        PUBLIC ${CMAKE_SOURCE_DIR}/include
        PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}
    )
    target_compile_features(${target} PUBLIC cxx_std_20)
endforeach()

if(BUILD_WITH_CUDA)
    target_include_directories(HyperEngine_CUDA
        PUBLIC ${CMAKE_SOURCE_DIR}/include
        PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}
    )
    # Используем C++17 для CUDA из-за несовместимости nvcc с C++20 constexpr
    target_compile_features(HyperEngine_CUDA PUBLIC cxx_std_17)
    
    # Добавляем зависимости для CUDA модуля
    target_link_libraries(HyperEngine_CUDA PUBLIC 
        HyperEngine_Core 
        HyperEngine_Math
    )
    
    # Добавляем Vulkan зависимости только если Vulkan включен
    if(BUILD_VULKAN_RENDERER)
        target_link_libraries(HyperEngine_CUDA PUBLIC Vulkan::Vulkan)
        if(TARGET VulkanMemoryAllocator::VulkanMemoryAllocator)
            target_link_libraries(HyperEngine_CUDA PUBLIC VulkanMemoryAllocator::VulkanMemoryAllocator)
        endif()
    endif()
    
    # Добавляем CUDA include директории
    find_package(CUDAToolkit REQUIRED)
    target_include_directories(HyperEngine_CUDA PRIVATE ${CUDAToolkit_INCLUDE_DIRS})
    target_link_libraries(HyperEngine_CUDA PUBLIC CUDA::cudart CUDA::cuda_driver)
endif()

if(BUILD_WITH_OPTIX)
    target_include_directories(HyperEngine_OptiX
        PUBLIC ${CMAKE_SOURCE_DIR}/include
        PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}
    )
    target_compile_features(HyperEngine_OptiX PUBLIC cxx_std_20)
endif()