# HyperEngine - Новая модульная структура

# Core модуль
file(GLOB_RECURSE CORE_SOURCES "core/*.cpp" "core/*.h")
add_library(HyperEngine_Core STATIC ${CORE_SOURCES})

# Math модуль
file(GLOB_RECURSE MATH_SOURCES "math/*.cpp" "math/*.h")
add_library(HyperEngine_Math STATIC ${MATH_SOURCES})

# Rendering модули
file(GLOB_RECURSE RENDERING_COMMON_SOURCES "rendering/common/*.cpp" "rendering/common/*.h")
file(GLOB_RECURSE RENDERING_OPENGL_SOURCES "rendering/opengl/*.cpp" "rendering/opengl/*.h")
file(GLOB_RECURSE RENDERING_VULKAN_SOURCES "rendering/vulkan/*.cpp" "rendering/vulkan/*.h")

# Создаем библиотеки только если есть исходные файлы
if(RENDERING_COMMON_SOURCES)
    add_library(HyperEngine_RenderingCommon STATIC ${RENDERING_COMMON_SOURCES})
endif()

if(RENDERING_OPENGL_SOURCES)
    add_library(HyperEngine_OpenGL STATIC ${RENDERING_OPENGL_SOURCES})
endif()

if(RENDERING_VULKAN_SOURCES)
    add_library(HyperEngine_Vulkan STATIC ${RENDERING_VULKAN_SOURCES})
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
    add_library(HyperEngine_CUDA STATIC ${CUDA_SOURCES})
    set_property(TARGET HyperEngine_CUDA PROPERTY CUDA_SEPARABLE_COMPILATION ON)
endif()

# OptiX модуль (опционально)
if(BUILD_WITH_OPTIX)
    file(GLOB_RECURSE OPTIX_SOURCES "optix/*.cpp" "optix/*.h")
    add_library(HyperEngine_OptiX STATIC ${OPTIX_SOURCES})
endif()

# Upscaling модуль
file(GLOB_RECURSE UPSCALING_SOURCES "upscaling/*.cpp" "upscaling/*.h")
add_library(HyperEngine_Upscaling STATIC ${UPSCALING_SOURCES})

# Настройка зависимостей
target_link_libraries(HyperEngine_Core PUBLIC HyperEngine_Math)

if(TARGET HyperEngine_RenderingCommon)
    target_link_libraries(HyperEngine_RenderingCommon PUBLIC HyperEngine_Core HyperEngine_Math)
endif()

if(TARGET HyperEngine_OpenGL)
    target_link_libraries(HyperEngine_OpenGL PUBLIC HyperEngine_Core HyperEngine_Math)
    if(TARGET HyperEngine_RenderingCommon)
        target_link_libraries(HyperEngine_OpenGL PUBLIC HyperEngine_RenderingCommon)
    endif()
endif()

if(TARGET HyperEngine_Vulkan)
    target_link_libraries(HyperEngine_Vulkan PUBLIC HyperEngine_Core HyperEngine_Math)
    if(TARGET HyperEngine_RenderingCommon)
        target_link_libraries(HyperEngine_Vulkan PUBLIC HyperEngine_RenderingCommon)
    endif()
endif()

target_link_libraries(HyperEngine_Physics PUBLIC HyperEngine_Math)
target_link_libraries(HyperEngine_Input PUBLIC HyperEngine_Core glfw)

# Include директории
set(COMMON_TARGETS HyperEngine_Core HyperEngine_Math HyperEngine_Physics HyperEngine_Input HyperEngine_Upscaling)

# Добавляем условные цели
if(TARGET HyperEngine_RenderingCommon)
    list(APPEND COMMON_TARGETS HyperEngine_RenderingCommon)
endif()
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
    target_compile_features(HyperEngine_CUDA PUBLIC cxx_std_20)
endif()

if(BUILD_WITH_OPTIX)
    target_include_directories(HyperEngine_OptiX
        PUBLIC ${CMAKE_SOURCE_DIR}/include
        PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}
    )
    target_compile_features(HyperEngine_OptiX PUBLIC cxx_std_20)
endif()