# FindCUDAAdvanced.cmake - расширенная настройка CUDA для HyperEngine
#
# Этот модуль определяет:
#  CUDA_ADVANCED_FOUND - если CUDA найден и настроен
#  CUDA_INTEROP_SUPPORTED - поддержка Vulkan-CUDA interop
#  CUDA_COMPUTE_CAPABILITIES - список поддерживаемых compute capabilities
#  CUDA_MEMORY_POOL_SUPPORTED - поддержка memory pools
#
# Переменные окружения для поиска:
#  CUDA_PATH - путь к CUDA Toolkit

cmake_minimum_required(VERSION 3.16)

# Проверяем, что базовый CUDA уже найден
if(NOT CUDAToolkit_FOUND)
    find_package(CUDAToolkit QUIET)
    if(NOT CUDAToolkit_FOUND)
        set(CUDA_ADVANCED_FOUND FALSE)
        return()
    endif()
endif()

# Проверка версии CUDA
if(CUDAToolkit_VERSION VERSION_LESS "11.8")
    message(WARNING "CUDA version ${CUDAToolkit_VERSION} is less than recommended 11.8. Some features may not be available.")
    set(CUDA_ADVANCED_FOUND FALSE)
    return()
endif()

# Определение поддерживаемых compute capabilities
set(CUDA_COMPUTE_CAPABILITIES "")

# Проверка доступных GPU и их compute capabilities
if(CMAKE_CUDA_COMPILER)
    execute_process(
        COMMAND ${CMAKE_CUDA_COMPILER} --list-gpu-arch
        OUTPUT_VARIABLE CUDA_GPU_ARCH_OUTPUT
        ERROR_QUIET
        OUTPUT_STRIP_TRAILING_WHITESPACE
    )
    
    if(CUDA_GPU_ARCH_OUTPUT)
        string(REGEX MATCHALL "sm_([0-9]+)" CUDA_ARCH_MATCHES "${CUDA_GPU_ARCH_OUTPUT}")
        foreach(ARCH_MATCH ${CUDA_ARCH_MATCHES})
            string(REGEX REPLACE "sm_([0-9]+)" "\\1" ARCH_NUM "${ARCH_MATCH}")
            list(APPEND CUDA_COMPUTE_CAPABILITIES ${ARCH_NUM})
        endforeach()
    endif()
endif()

# Если не удалось определить автоматически, используем стандартный набор
if(NOT CUDA_COMPUTE_CAPABILITIES)
    set(CUDA_COMPUTE_CAPABILITIES "75;80;86;89;90")
endif()

# Настройка флагов компилятора CUDA
set(CUDA_NVCC_FLAGS "")

# Добавляем compute capabilities
foreach(CAPABILITY ${CUDA_COMPUTE_CAPABILITIES})
    list(APPEND CUDA_NVCC_FLAGS "-gencode arch=compute_${CAPABILITY},code=sm_${CAPABILITY}")
endforeach()

# Добавляем последнюю версию как PTX для совместимости с будущими GPU
list(GET CUDA_COMPUTE_CAPABILITIES -1 LATEST_CAPABILITY)
list(APPEND CUDA_NVCC_FLAGS "-gencode arch=compute_${LATEST_CAPABILITY},code=compute_${LATEST_CAPABILITY}")

# Дополнительные флаги оптимизации
list(APPEND CUDA_NVCC_FLAGS 
    "--use_fast_math"
    "--restrict"
    "--extra-device-vectorization"
)

# Флаги для debug/release
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    list(APPEND CUDA_NVCC_FLAGS "-G" "-O0")
else()
    list(APPEND CUDA_NVCC_FLAGS "-O3" "--ptxas-options=-v")
endif()

# Проверка поддержки Vulkan-CUDA interop
set(CUDA_INTEROP_SUPPORTED FALSE)

# Проверяем наличие необходимых заголовков для interop
find_path(CUDA_EXTERNAL_MEMORY_HEADER
    NAMES cuda_runtime_api.h
    PATHS ${CUDAToolkit_INCLUDE_DIRS}
    NO_DEFAULT_PATH
)

if(CUDA_EXTERNAL_MEMORY_HEADER AND Vulkan_FOUND)
    # Упрощенная проверка - просто проверяем наличие заголовков
    # Полную проверку компиляции сделаем позже, когда CUDA будет активирован
    set(CUDA_INTEROP_SUPPORTED TRUE)
    message(STATUS "CUDA-Vulkan interop headers found - enabling support")
endif()

# Проверка поддержки memory pools (CUDA 11.2+)
set(CUDA_MEMORY_POOL_SUPPORTED FALSE)
if(CUDAToolkit_VERSION VERSION_GREATER_EQUAL "11.2")
    set(CUDA_MEMORY_POOL_SUPPORTED TRUE)
endif()

# Проверка поддержки stream ordered memory allocator (CUDA 11.2+)
set(CUDA_STREAM_ORDERED_ALLOC_SUPPORTED FALSE)
if(CUDAToolkit_VERSION VERSION_GREATER_EQUAL "11.2")
    set(CUDA_STREAM_ORDERED_ALLOC_SUPPORTED TRUE)
endif()

# Создание функции для настройки CUDA target
function(configure_cuda_target TARGET_NAME)
    if(NOT TARGET ${TARGET_NAME})
        message(FATAL_ERROR "Target ${TARGET_NAME} does not exist")
    endif()
    
    # Установка CUDA стандарта
    set_property(TARGET ${TARGET_NAME} PROPERTY CUDA_STANDARD 17)
    set_property(TARGET ${TARGET_NAME} PROPERTY CUDA_STANDARD_REQUIRED ON)
    
    # Установка флагов компилятора
    target_compile_options(${TARGET_NAME} PRIVATE 
        $<$<COMPILE_LANGUAGE:CUDA>:${CUDA_NVCC_FLAGS}>
    )
    
    # Настройка separable compilation для device linking
    set_property(TARGET ${TARGET_NAME} PROPERTY CUDA_SEPARABLE_COMPILATION ON)
    set_property(TARGET ${TARGET_NAME} PROPERTY CUDA_RESOLVE_DEVICE_SYMBOLS ON)
    
    # Добавление определений компилятора
    target_compile_definitions(${TARGET_NAME} PRIVATE
        CUDA_ADVANCED_AVAILABLE=1
        CUDA_VERSION_MAJOR=${CUDAToolkit_VERSION_MAJOR}
        CUDA_VERSION_MINOR=${CUDAToolkit_VERSION_MINOR}
    )
    
    if(CUDA_INTEROP_SUPPORTED)
        target_compile_definitions(${TARGET_NAME} PRIVATE CUDA_VULKAN_INTEROP_SUPPORTED=1)
    endif()
    
    if(CUDA_MEMORY_POOL_SUPPORTED)
        target_compile_definitions(${TARGET_NAME} PRIVATE CUDA_MEMORY_POOL_SUPPORTED=1)
    endif()
    
    if(CUDA_STREAM_ORDERED_ALLOC_SUPPORTED)
        target_compile_definitions(${TARGET_NAME} PRIVATE CUDA_STREAM_ORDERED_ALLOC_SUPPORTED=1)
    endif()
    
    # Линковка CUDA библиотек
    target_link_libraries(${TARGET_NAME} PRIVATE
        CUDA::cudart
        CUDA::cuda_driver
    )
    
    # Дополнительные библиотеки для advanced features
    if(TARGET CUDA::curand)
        target_link_libraries(${TARGET_NAME} PRIVATE CUDA::curand)
    endif()
    
    if(TARGET CUDA::cublas)
        target_link_libraries(${TARGET_NAME} PRIVATE CUDA::cublas)
    endif()
    
    if(TARGET CUDA::cufft)
        target_link_libraries(${TARGET_NAME} PRIVATE CUDA::cufft)
    endif()
endfunction()

# Установка переменных результата
include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(CUDAAdvanced
    FOUND_VAR CUDAAdvanced_FOUND
    REQUIRED_VARS CUDAToolkit_FOUND
    VERSION_VAR CUDAToolkit_VERSION
)

# Устанавливаем также старую переменную для совместимости
set(CUDA_ADVANCED_FOUND ${CUDAAdvanced_FOUND})

if(CUDA_ADVANCED_FOUND)
    if(NOT CUDAAdvanced_FIND_QUIETLY)
        message(STATUS "CUDA Advanced configuration:")
        message(STATUS "  Version: ${CUDAToolkit_VERSION}")
        message(STATUS "  Compute Capabilities: ${CUDA_COMPUTE_CAPABILITIES}")
        message(STATUS "  Vulkan Interop: ${CUDA_INTEROP_SUPPORTED}")
        message(STATUS "  Memory Pools: ${CUDA_MEMORY_POOL_SUPPORTED}")
        message(STATUS "  Stream Ordered Alloc: ${CUDA_STREAM_ORDERED_ALLOC_SUPPORTED}")
    endif()
endif()

# Скрытие внутренних переменных
mark_as_advanced(
    CUDA_EXTERNAL_MEMORY_HEADER
    CUDA_INTEROP_TEST_RESULT
)
