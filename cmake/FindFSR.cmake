# FindFSR.cmake - поиск AMD FidelityFX Super Resolution SDK
#
# Этот модуль определяет:
#  FSR_FOUND - если FSR SDK найден
#  FSR_INCLUDE_DIRS - директории с заголовочными файлами FSR
#  FSR_LIBRARIES - библиотеки FSR для линковки
#  FSR_VERSION - версия FSR SDK
#
# Переменные окружения для поиска:
#  FSR_ROOT_DIR - корневая директория FSR SDK
#  FIDELITYFX_ROOT - корневая директория FidelityFX SDK

cmake_minimum_required(VERSION 3.16)

# Поиск корневой директории FSR SDK
find_path(FSR_ROOT_DIR
    NAMES sdk/include/FidelityFX/host/ffx_fsr2.h
    PATHS
        ${FSR_ROOT_DIR}
        $ENV{FSR_ROOT_DIR}
        $ENV{FIDELITYFX_ROOT}
        "C:/Program Files/AMD/FidelityFX"
        "C:/ProgramData/AMD/FidelityFX"
        "/usr/local/fidelityfx"
        "/opt/fidelityfx"
        "${CMAKE_SOURCE_DIR}/external/FidelityFX-FSR2"
        "${CMAKE_SOURCE_DIR}/third_party/FidelityFX-FSR2"
    DOC "FSR SDK root directory"
)

# Поиск заголовочных файлов
find_path(FSR_INCLUDE_DIR
    NAMES FidelityFX/host/ffx_fsr2.h
    PATHS
        ${FSR_ROOT_DIR}/sdk/include
        ${FSR_ROOT_DIR}/include
    DOC "FSR include directory"
)

# Поиск библиотек FSR
if(WIN32)
    set(FSR_LIB_NAMES ffx_fsr2_api_x64 ffx_fsr2_api_vk_x64)
    set(FSR_LIB_PATHS
        ${FSR_ROOT_DIR}/sdk/lib
        ${FSR_ROOT_DIR}/lib/x64
        ${FSR_ROOT_DIR}/bin/x64
    )
else()
    set(FSR_LIB_NAMES ffx_fsr2_api ffx_fsr2_api_vk)
    set(FSR_LIB_PATHS
        ${FSR_ROOT_DIR}/sdk/lib
        ${FSR_ROOT_DIR}/lib
    )
endif()

find_library(FSR_API_LIBRARY
    NAMES ffx_fsr2_api_x64 ffx_fsr2_api
    PATHS ${FSR_LIB_PATHS}
    DOC "FSR API library"
)

find_library(FSR_VK_LIBRARY
    NAMES ffx_fsr2_api_vk_x64 ffx_fsr2_api_vk
    PATHS ${FSR_LIB_PATHS}
    DOC "FSR Vulkan backend library"
)

# Поиск дополнительных компонентов FSR
find_library(FSR_FRAMEINTERPOLATION_LIBRARY
    NAMES ffx_frameinterpolation_x64 ffx_frameinterpolation
    PATHS ${FSR_LIB_PATHS}
    DOC "FSR Frame Interpolation library"
)

# Определение версии FSR
if(FSR_INCLUDE_DIR)
    file(READ "${FSR_INCLUDE_DIR}/FidelityFX/host/ffx_fsr2.h" FSR_HEADER_CONTENT LIMIT 4096)
    
    string(REGEX MATCH "#define FFX_FSR2_VERSION_MAJOR ([0-9]+)" FSR_VERSION_MAJOR_MATCH "${FSR_HEADER_CONTENT}")
    string(REGEX MATCH "#define FFX_FSR2_VERSION_MINOR ([0-9]+)" FSR_VERSION_MINOR_MATCH "${FSR_HEADER_CONTENT}")
    string(REGEX MATCH "#define FFX_FSR2_VERSION_PATCH ([0-9]+)" FSR_VERSION_PATCH_MATCH "${FSR_HEADER_CONTENT}")
    
    if(FSR_VERSION_MAJOR_MATCH AND FSR_VERSION_MINOR_MATCH AND FSR_VERSION_PATCH_MATCH)
        string(REGEX REPLACE ".*([0-9]+).*" "\\1" FSR_VERSION_MAJOR "${FSR_VERSION_MAJOR_MATCH}")
        string(REGEX REPLACE ".*([0-9]+).*" "\\1" FSR_VERSION_MINOR "${FSR_VERSION_MINOR_MATCH}")
        string(REGEX REPLACE ".*([0-9]+).*" "\\1" FSR_VERSION_PATCH "${FSR_VERSION_PATCH_MATCH}")
        set(FSR_VERSION "${FSR_VERSION_MAJOR}.${FSR_VERSION_MINOR}.${FSR_VERSION_PATCH}")
    else()
        set(FSR_VERSION "2.0.0")  # Значение по умолчанию
    endif()
endif()

# Установка переменных результата
include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(FSR
    FOUND_VAR FSR_FOUND
    REQUIRED_VARS FSR_ROOT_DIR FSR_INCLUDE_DIR FSR_API_LIBRARY
    VERSION_VAR FSR_VERSION
)

if(FSR_FOUND)
    set(FSR_INCLUDE_DIRS ${FSR_INCLUDE_DIR})
    set(FSR_LIBRARIES ${FSR_API_LIBRARY})
    
    # Добавление Vulkan backend если найден
    if(FSR_VK_LIBRARY)
        list(APPEND FSR_LIBRARIES ${FSR_VK_LIBRARY})
    endif()
    
    # Добавление Frame Interpolation если найден
    if(FSR_FRAMEINTERPOLATION_LIBRARY)
        list(APPEND FSR_LIBRARIES ${FSR_FRAMEINTERPOLATION_LIBRARY})
    endif()
    
    # Создание imported target
    if(NOT TARGET FSR::FSR)
        add_library(FSR::FSR INTERFACE IMPORTED)
        target_include_directories(FSR::FSR INTERFACE ${FSR_INCLUDE_DIRS})
        target_link_libraries(FSR::FSR INTERFACE ${FSR_LIBRARIES})
        
        # Добавление определений компилятора
        target_compile_definitions(FSR::FSR INTERFACE 
            FSR_AVAILABLE=1
            FFX_FSR2_ENABLE_DEBUG_CHECKING=0
        )
        
        # Настройки для различных платформ
        if(WIN32)
            target_compile_definitions(FSR::FSR INTERFACE FFX_WINDOWS=1)
        elseif(UNIX)
            target_compile_definitions(FSR::FSR INTERFACE FFX_LINUX=1)
        endif()
    endif()
    
    # Дополнительные пути для шейдеров FSR
    set(FSR_SHADERS_DIR "${FSR_ROOT_DIR}/sdk/shaders")
    
    if(NOT FSR_FIND_QUIETLY)
        message(STATUS "Found FSR: ${FSR_ROOT_DIR} (version ${FSR_VERSION})")
    endif()
endif()

# Скрытие внутренних переменных от пользователя
mark_as_advanced(
    FSR_ROOT_DIR
    FSR_INCLUDE_DIR
    FSR_API_LIBRARY
    FSR_VK_LIBRARY
    FSR_FRAMEINTERPOLATION_LIBRARY
)
