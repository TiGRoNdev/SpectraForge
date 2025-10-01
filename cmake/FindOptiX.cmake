# FindOptiX.cmake - поиск NVIDIA OptiX SDK
#
# Этот модуль определяет:
#  OptiX_FOUND - если OptiX SDK найден
#  OptiX_INCLUDE_DIRS - директории с заголовочными файлами OptiX
#  OptiX_LIBRARIES - библиотеки OptiX для линковки
#  OptiX_VERSION - версия OptiX SDK
#
# Переменные окружения для поиска:
#  OptiX_ROOT_DIR - корневая директория OptiX SDK
#  OPTIX_ROOT - альтернативная переменная для корневой директории

cmake_minimum_required(VERSION 3.16)

# Поиск корневой директории OptiX SDK
find_path(OptiX_ROOT_DIR
    NAMES include/optix.h
    PATHS
        ${OptiX_ROOT_DIR}
        $ENV{OptiX_ROOT_DIR}
        $ENV{OPTIX_ROOT}
        "C:/ProgramData/NVIDIA Corporation/OptiX SDK 7.7.0"
        "C:/ProgramData/NVIDIA Corporation/OptiX SDK 7.6.0"
        "C:/ProgramData/NVIDIA Corporation/OptiX SDK 7.5.0"
        "C:/Program Files/NVIDIA Corporation/OptiX SDK 7.7.0"
        "C:/Program Files/NVIDIA Corporation/OptiX SDK 7.6.0"
        "C:/Program Files/NVIDIA Corporation/OptiX SDK 7.5.0"
        "/usr/local/optix"
        "/opt/optix"
    DOC "OptiX SDK root directory"
)

# Поиск заголовочных файлов
find_path(OptiX_INCLUDE_DIR
    NAMES optix.h
    PATHS
        ${OptiX_ROOT_DIR}/include
        ${OptiX_ROOT_DIR}/SDK/include
    DOC "OptiX include directory"
)

# OptiX 7.x является header-only SDK, библиотеки не требуются
# Все функции загружаются динамически через CUDA Driver API
set(OptiX_LIBRARY "" CACHE STRING "OptiX library (not required for OptiX 7.x)" FORCE)

# Определение версии OptiX
if(OptiX_INCLUDE_DIR)
    file(READ "${OptiX_INCLUDE_DIR}/optix.h" OptiX_HEADER_CONTENT)
    
    string(REGEX MATCH "#define OPTIX_VERSION ([0-9]+)" OptiX_VERSION_MATCH "${OptiX_HEADER_CONTENT}")
    if(OptiX_VERSION_MATCH)
        set(OptiX_VERSION_RAW ${CMAKE_MATCH_1})
        
        # Преобразование версии из формата 70700 в 7.7.0
        math(EXPR OptiX_VERSION_MAJOR "${OptiX_VERSION_RAW} / 10000")
        math(EXPR OptiX_VERSION_MINOR "(${OptiX_VERSION_RAW} % 10000) / 100")
        math(EXPR OptiX_VERSION_PATCH "${OptiX_VERSION_RAW} % 100")
        
        set(OptiX_VERSION "${OptiX_VERSION_MAJOR}.${OptiX_VERSION_MINOR}.${OptiX_VERSION_PATCH}")
    endif()
endif()

# Проверка минимальной версии
if(OptiX_FIND_VERSION)
    if(OptiX_VERSION VERSION_LESS OptiX_FIND_VERSION)
        set(OptiX_FOUND FALSE)
        if(NOT OptiX_FIND_QUIETLY)
            message(WARNING "OptiX version ${OptiX_VERSION} is less than required ${OptiX_FIND_VERSION}")
        endif()
    endif()
endif()

# Установка переменных результата
include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(OptiX
    FOUND_VAR OptiX_FOUND
    REQUIRED_VARS OptiX_ROOT_DIR OptiX_INCLUDE_DIR
    VERSION_VAR OptiX_VERSION
)

if(OptiX_FOUND)
    set(OptiX_INCLUDE_DIRS ${OptiX_INCLUDE_DIR})
    # OptiX 7.x не имеет библиотек для линковки
    set(OptiX_LIBRARIES "")
    
    # Создание imported target
    if(NOT TARGET OptiX::OptiX)
        add_library(OptiX::OptiX INTERFACE IMPORTED)
        target_include_directories(OptiX::OptiX INTERFACE ${OptiX_INCLUDE_DIRS})
        
        # OptiX 7.x не требует линковки библиотек, только заголовочные файлы
        # Все функции загружаются динамически через CUDA Driver API
        # Требуется только CUDA Driver API
        target_compile_definitions(OptiX::OptiX INTERFACE OPTIX_AVAILABLE=1)
    endif()
    
    # Дополнительные пути для примеров и утилит OptiX
    set(OptiX_SDK_DIR "${OptiX_ROOT_DIR}/SDK")
    set(OptiX_SAMPLES_DIR "${OptiX_ROOT_DIR}/SDK/optixHello")
    
    if(NOT OptiX_FIND_QUIETLY)
        message(STATUS "Found OptiX: ${OptiX_ROOT_DIR} (version ${OptiX_VERSION})")
    endif()
endif()

# Скрытие внутренних переменных от пользователя
mark_as_advanced(
    OptiX_ROOT_DIR
    OptiX_INCLUDE_DIR
)
