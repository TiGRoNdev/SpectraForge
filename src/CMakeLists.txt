# SpectraForge - Новая модульная структура

# Core модуль
file(GLOB_RECURSE CORE_SOURCES "core/*.cpp" "core/*.h")
add_library(SpectraForge_Core STATIC ${CORE_SOURCES})
# App модуль (высокоуровневый фасад)
file(GLOB_RECURSE APP_SOURCES "app/*.cpp" "app/*.h")
if(APP_SOURCES)
    add_library(SpectraForge_App STATIC ${APP_SOURCES})
    target_link_libraries(SpectraForge_App
        PUBLIC
            SpectraForge_Core
            SpectraForge_Math
    )
    target_include_directories(SpectraForge_App
        PUBLIC ${CMAKE_SOURCE_DIR}/include
        PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}
    )
    target_compile_features(SpectraForge_App PUBLIC cxx_std_20)
endif()

# Link Vulkan (Core uses VulkanContextImpl with vk* symbols)
find_package(Vulkan REQUIRED)
target_link_libraries(SpectraForge_Core PUBLIC Vulkan::Vulkan)

# Link VMA if available (VMAMemoryManager uses VMA APIs)
if(TARGET VulkanMemoryAllocator::VulkanMemoryAllocator)
    target_link_libraries(SpectraForge_Core PUBLIC VulkanMemoryAllocator::VulkanMemoryAllocator)
endif()

# Math модуль
add_subdirectory(math)

# Rendering модули
file(GLOB_RECURSE RENDERING_COMMON_SOURCES "rendering/common/*.cpp" "rendering/common/*.h")
file(GLOB_RECURSE RENDERING_OPENGL_SOURCES "rendering/opengl/*.cpp" "rendering/opengl/*.h")
file(GLOB_RECURSE FREQVOX_SOURCES "rendering/freqvox/*.cpp" "rendering/freqvox/*.h")

# Создаем библиотеки только если есть исходные файлы
if(RENDERING_COMMON_SOURCES)
    add_library(SpectraForge_RenderingCommon STATIC ${RENDERING_COMMON_SOURCES})
endif()

if(RENDERING_OPENGL_SOURCES)
    add_library(SpectraForge_OpenGL STATIC ${RENDERING_OPENGL_SOURCES})
endif()

# FreqVox модуль (опционально)
if(ENABLE_FREQVOX AND FREQVOX_SOURCES)
    add_library(SpectraForge_FreqVox STATIC 
        ${FREQVOX_SOURCES}
        rendering/StrategyFactory.cpp
    )
    target_link_libraries(SpectraForge_FreqVox PUBLIC
        SpectraForge_Core
        SpectraForge_Math
    )
    target_include_directories(SpectraForge_FreqVox
        PUBLIC ${CMAKE_SOURCE_DIR}/include
        PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}
    )
    target_compile_features(SpectraForge_FreqVox PUBLIC cxx_std_20)

    # Опционально объявляем доступность CUDA для бэкенда cuFFT
    if(BUILD_WITH_CUDA)
        find_package(CUDAToolkit QUIET)
        if(CUDAToolkit_FOUND)
            target_compile_definitions(SpectraForge_FreqVox PUBLIC HYPERENGINE_CUDA_AVAILABLE=1)
            target_include_directories(SpectraForge_FreqVox PRIVATE ${CUDAToolkit_INCLUDE_DIRS})
            target_link_libraries(SpectraForge_FreqVox PRIVATE CUDA::cudart CUDA::cufft)
            message(STATUS "FreqVox: cuFFT backend enabled")
        endif()
    endif()

    # VkFFT backend для DCT-II через Vulkan (строго по FreqVox Renderer Math.md)
    if(BUILD_VULKAN_RENDERER)
        # Ищем VkFFT (header-only библиотека из vcpkg)
        find_path(VKFFT_INCLUDE_DIR
            NAMES vkFFT.h
            PATHS 
                ${CMAKE_CURRENT_SOURCE_DIR}/../external/VkFFT
                ${CMAKE_BINARY_DIR}/vcpkg_installed/${VCPKG_TARGET_TRIPLET}/include
                ${VCPKG_INSTALLED_DIR}/${VCPKG_TARGET_TRIPLET}/include
                /usr/local/include
                /usr/include
            PATH_SUFFIXES VkFFT vkFFT VkFFT/vkFFT
            NO_DEFAULT_PATH
        )
        
        # Fallback: system-wide search
        if(NOT VKFFT_INCLUDE_DIR)
            find_path(VKFFT_INCLUDE_DIR
                NAMES vkFFT.h
                PATH_SUFFIXES VkFFT vkFFT
            )
        endif()

        if(VKFFT_INCLUDE_DIR)
            target_compile_definitions(SpectraForge_FreqVox PUBLIC 
                HYPERENGINE_USE_VKFFT=1
            )
            
            # VkFFT требует glslang headers
            find_path(GLSLANG_INCLUDE_DIR
                NAMES glslang/Include/glslang_c_interface.h
                PATHS
                    ${CMAKE_BINARY_DIR}/vcpkg_installed/${VCPKG_TARGET_TRIPLET}/include
                    ${VCPKG_INSTALLED_DIR}/${VCPKG_TARGET_TRIPLET}/include
                    /usr/local/include
                    /usr/include
                NO_DEFAULT_PATH
            )
            
            if(NOT GLSLANG_INCLUDE_DIR)
                find_path(GLSLANG_INCLUDE_DIR NAMES glslang/Include/glslang_c_interface.h)
            endif()
            
            target_include_directories(SpectraForge_FreqVox PRIVATE 
                ${VKFFT_INCLUDE_DIR}
                ${GLSLANG_INCLUDE_DIR}/glslang/Include
            )
            
            # Линковка с Vulkan для VkFFT backend
            find_package(Vulkan REQUIRED)
            target_link_libraries(SpectraForge_FreqVox PRIVATE Vulkan::Vulkan)
            
            message(STATUS "✓ FreqVox: VkFFT DCT-II backend enabled")
            message(STATUS "  VkFFT header: ${VKFFT_INCLUDE_DIR}/vkFFT.h")
            message(STATUS "  glslang header: ${GLSLANG_INCLUDE_DIR}/glslang/Include/glslang_c_interface.h")
        else()
            message(WARNING "FreqVox: VkFFT not found - DCT-II GPU backend will be unavailable")
            message(STATUS "FreqVox: Install VkFFT via vcpkg: vcpkg install vkfft")
        endif()
    else()
        message(STATUS "FreqVox: VkFFT backend disabled (BUILD_VULKAN_RENDERER=OFF)")
    endif()
endif()

# Vulkan модуль с отдельным CMakeLists.txt
if(BUILD_VULKAN_RENDERER AND ENABLE_VULKAN_BACKEND)
    add_subdirectory(rendering/vulkan)
endif()

# Physics модуль
file(GLOB_RECURSE PHYSICS_SOURCES "physics/*.cpp" "physics/*.h")
add_library(SpectraForge_Physics STATIC ${PHYSICS_SOURCES})

# Input модуль
file(GLOB_RECURSE INPUT_SOURCES "input/*.cpp" "input/*.h")
add_library(SpectraForge_Input STATIC ${INPUT_SOURCES})

# CUDA модуль (опционально)
if(BUILD_WITH_CUDA)
    file(GLOB_RECURSE CUDA_SOURCES "cuda/*.cpp" "cuda/*.cu" "cuda/*.h")
    
    # Временно исключаем файлы, требующие Vulkan-CUDA interop
    list(FILTER CUDA_SOURCES EXCLUDE REGEX ".*CudaInterop\\.cpp$")
    # FlashGSSplatter теперь включен - CUDA headers доступны
    # list(FILTER CUDA_SOURCES EXCLUDE REGEX ".*FlashGSSplatter\\.cpp$")
    
    add_library(SpectraForge_CUDA STATIC ${CUDA_SOURCES})
    set_property(TARGET SpectraForge_CUDA PROPERTY CUDA_SEPARABLE_COMPILATION ON)
    
    # НЕ добавляем CUDA_VULKAN_INTEROP_SUPPORTED из-за несовместимости nvcc с Vulkan headers
    # TODO: Включить когда будет решена проблема с constexpr в Vulkan-Hpp
endif()

# OptiX модуль (опционально)
if(BUILD_WITH_OPTIX)
    file(GLOB_RECURSE OPTIX_SOURCES "optix/*.cpp" "optix/*.h")
    # Добавляем OptiX stubs initialization отдельно
    list(APPEND OPTIX_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/optix/optix_init.cpp")
    add_library(SpectraForge_OptiX STATIC ${OPTIX_SOURCES})
    
    # Добавляем CUDA и OptiX include directories
    find_package(CUDAToolkit REQUIRED)
    target_include_directories(SpectraForge_OptiX PUBLIC
        ${CMAKE_SOURCE_DIR}/include
        ${CUDAToolkit_INCLUDE_DIRS}
        ${OptiX_INCLUDE_DIRS}
    )
    target_link_libraries(SpectraForge_OptiX PUBLIC CUDA::cudart CUDA::cuda_driver)
endif()

# Upscaling модуль
file(GLOB_RECURSE UPSCALING_SOURCES "upscaling/*.cpp" "upscaling/*.h")
add_library(SpectraForge_Upscaling STATIC ${UPSCALING_SOURCES})

# Настройка зависимостей
target_link_libraries(SpectraForge_Core PUBLIC SpectraForge_Math)

# Upscaling dependencies
target_link_libraries(SpectraForge_Upscaling PUBLIC
    SpectraForge_Core
    Vulkan::Vulkan
)

# DLSS support (conditional compilation)
if(BUILD_WITH_DLSS AND DLSS_FOUND)
    target_compile_definitions(SpectraForge_Upscaling PUBLIC SPECTRAFORGE_DLSS_AVAILABLE)
    target_include_directories(SpectraForge_Upscaling PRIVATE ${DLSS_INCLUDE_DIRS})
    target_link_libraries(SpectraForge_Upscaling PRIVATE ${DLSS_LIBRARIES})
    message(STATUS "SpectraForge_Upscaling: DLSS support enabled")
else()
    message(STATUS "SpectraForge_Upscaling: DLSS disabled (skeleton only)")
endif()

# FSR2 support (conditional compilation)
if(BUILD_WITH_FSR AND FSR_FOUND)
    target_compile_definitions(SpectraForge_Upscaling PUBLIC SPECTRAFORGE_FSR2_AVAILABLE)
    target_include_directories(SpectraForge_Upscaling PRIVATE ${FSR_INCLUDE_DIRS})
    target_link_libraries(SpectraForge_Upscaling PRIVATE ${FSR_LIBRARIES})
    message(STATUS "SpectraForge_Upscaling: FSR2 support enabled")
else()
    message(STATUS "SpectraForge_Upscaling: FSR2 disabled (skeleton only)")
endif()

if(TARGET SpectraForge_OpenGL)
    target_link_libraries(SpectraForge_OpenGL PUBLIC 
        SpectraForge_Core 
        SpectraForge_Math
        GLEW::GLEW
        glfw
        glm::glm
        OpenGL::GL
    )
endif()

# Vulkan зависимости настраиваются в его собственном CMakeLists.txt

target_link_libraries(SpectraForge_Physics PUBLIC SpectraForge_Math)
target_link_libraries(SpectraForge_Input PUBLIC SpectraForge_Core glfw)

# Include директории
set(COMMON_TARGETS SpectraForge_Core SpectraForge_Math SpectraForge_Physics SpectraForge_Input SpectraForge_Upscaling)

# Добавляем условные цели
if(TARGET SpectraForge_OpenGL)
    list(APPEND COMMON_TARGETS SpectraForge_OpenGL)
endif()
if(TARGET SpectraForge_Vulkan)
    list(APPEND COMMON_TARGETS SpectraForge_Vulkan)
endif()

foreach(target ${COMMON_TARGETS})
    target_include_directories(${target}
        PUBLIC ${CMAKE_SOURCE_DIR}/include
        PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}
    )
    target_compile_features(${target} PUBLIC cxx_std_20)
endforeach()

if(BUILD_WITH_CUDA)
    target_include_directories(SpectraForge_CUDA
        PUBLIC ${CMAKE_SOURCE_DIR}/include
        PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}
    )
    # Используем C++17 для CUDA из-за несовместимости nvcc с C++20 constexpr
    target_compile_features(SpectraForge_CUDA PUBLIC cxx_std_17)
    
    # Добавляем зависимости для CUDA модуля
    target_link_libraries(SpectraForge_CUDA PUBLIC 
        SpectraForge_Core 
        SpectraForge_Math
    )
    
    # Добавляем Vulkan зависимости только если Vulkan включен
    if(BUILD_VULKAN_RENDERER)
        target_link_libraries(SpectraForge_CUDA PUBLIC Vulkan::Vulkan)
        if(TARGET VulkanMemoryAllocator::VulkanMemoryAllocator)
            target_link_libraries(SpectraForge_CUDA PUBLIC VulkanMemoryAllocator::VulkanMemoryAllocator)
        endif()
    endif()
    
    # Добавляем CUDA include директории
    find_package(CUDAToolkit REQUIRED)
    target_include_directories(SpectraForge_CUDA PRIVATE ${CUDAToolkit_INCLUDE_DIRS})
    target_link_libraries(SpectraForge_CUDA PUBLIC CUDA::cudart CUDA::cuda_driver)
endif()

if(BUILD_WITH_OPTIX)
    target_include_directories(SpectraForge_OptiX
        PUBLIC ${CMAKE_SOURCE_DIR}/include
        PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}
    )
    target_compile_features(SpectraForge_OptiX PUBLIC cxx_std_20)
endif()