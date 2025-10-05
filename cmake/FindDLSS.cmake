# FindDLSS.cmake - поиск NVIDIA DLSS SDK (Streamline)
#
# Этот модуль определяет:
#  DLSS_FOUND - если DLSS SDK найден
#  DLSS_INCLUDE_DIRS - директории с заголовочными файлами DLSS
#  DLSS_LIBRARIES - библиотеки DLSS для линковки
#  DLSS_VERSION - версия DLSS SDK
#
# Переменные окружения для поиска:
#  DLSS_ROOT_DIR - корневая директория DLSS SDK
#  STREAMLINE_ROOT - корневая директория Streamline SDK

cmake_minimum_required(VERSION 3.16)

# Поиск корневой директории DLSS/Streamline SDK
find_path(DLSS_ROOT_DIR
    NAMES include/sl.h
    PATHS
        ${DLSS_ROOT_DIR}
        $ENV{DLSS_ROOT_DIR}
        $ENV{STREAMLINE_ROOT}
        "C:/Program Files/NVIDIA Corporation/Streamline"
        "C:/ProgramData/NVIDIA Corporation/Streamline"
        "/usr/local/streamline"
        "/opt/streamline"
    DOC "DLSS/Streamline SDK root directory"
)

# Поиск заголовочных файлов
find_path(DLSS_INCLUDE_DIR
    NAMES sl.h
    PATHS
        ${DLSS_ROOT_DIR}/include
        ${DLSS_ROOT_DIR}/external/streamline/include
    DOC "DLSS include directory"
)

# Поиск библиотек DLSS
if(WIN32)
    set(DLSS_LIB_NAMES sl.interposer)
    set(DLSS_LIB_PATHS
        ${DLSS_ROOT_DIR}/lib/x64
        ${DLSS_ROOT_DIR}/bin/x64
        ${DLSS_ROOT_DIR}/external/streamline/lib/x64
    )
else()
    # ВАЖНО: Streamline SDK официально не поддерживает Linux
    # Для Linux используйте FSR или отключите DLSS
    set(DLSS_LIB_NAMES libsl.interposer.so sl.interposer)
    set(DLSS_LIB_PATHS
        ${DLSS_ROOT_DIR}/lib
        ${DLSS_ROOT_DIR}/lib/linux
        ${DLSS_ROOT_DIR}/external/streamline/lib
        ${DLSS_ROOT_DIR}/external/streamline/lib/linux
    )
endif()

find_library(DLSS_LIBRARY
    NAMES ${DLSS_LIB_NAMES}
    PATHS ${DLSS_LIB_PATHS}
    DOC "DLSS library"
)

# Поиск дополнительных библиотек Streamline
find_library(DLSS_DLSSG_LIBRARY
    NAMES sl.dlss_g
    PATHS ${DLSS_LIB_PATHS}
    DOC "DLSS-G library"
)

find_library(DLSS_COMMON_LIBRARY
    NAMES sl.common
    PATHS ${DLSS_LIB_PATHS}
    DOC "DLSS common library"
)

# Определение версии DLSS
if(DLSS_INCLUDE_DIR)
    file(READ "${DLSS_INCLUDE_DIR}/sl_version.h" DLSS_VERSION_HEADER LIMIT 2048)
    
    string(REGEX MATCH "#define SL_VERSION_MAJOR ([0-9]+)" DLSS_VERSION_MAJOR_MATCH "${DLSS_VERSION_HEADER}")
    string(REGEX MATCH "#define SL_VERSION_MINOR ([0-9]+)" DLSS_VERSION_MINOR_MATCH "${DLSS_VERSION_HEADER}")
    string(REGEX MATCH "#define SL_VERSION_PATCH ([0-9]+)" DLSS_VERSION_PATCH_MATCH "${DLSS_VERSION_HEADER}")
    
    if(DLSS_VERSION_MAJOR_MATCH AND DLSS_VERSION_MINOR_MATCH AND DLSS_VERSION_PATCH_MATCH)
        string(REGEX REPLACE ".*([0-9]+).*" "\\1" DLSS_VERSION_MAJOR "${DLSS_VERSION_MAJOR_MATCH}")
        string(REGEX REPLACE ".*([0-9]+).*" "\\1" DLSS_VERSION_MINOR "${DLSS_VERSION_MINOR_MATCH}")
        string(REGEX REPLACE ".*([0-9]+).*" "\\1" DLSS_VERSION_PATCH "${DLSS_VERSION_PATCH_MATCH}")
        set(DLSS_VERSION "${DLSS_VERSION_MAJOR}.${DLSS_VERSION_MINOR}.${DLSS_VERSION_PATCH}")
    else()
        set(DLSS_VERSION "Unknown")
    endif()
endif()

# Установка переменных результата
include(FindPackageHandleStandardArgs)

# Для Linux: DLSS_LIBRARY опциональна, так как Streamline не поддерживает Linux
if(UNIX AND NOT APPLE)
    find_package_handle_standard_args(DLSS
        FOUND_VAR DLSS_FOUND
        REQUIRED_VARS DLSS_ROOT_DIR DLSS_INCLUDE_DIR
        VERSION_VAR DLSS_VERSION
    )
    
    # Для Linux: если библиотеки нет, все равно считаем "найденным" с предупреждением
    if(DLSS_INCLUDE_DIR AND NOT DLSS_LIBRARY)
        message(WARNING "DLSS headers found, but library missing. DLSS not officially supported on Linux. Consider using FSR instead.")
        set(DLSS_FOUND FALSE)
    endif()
else()
    find_package_handle_standard_args(DLSS
        FOUND_VAR DLSS_FOUND
        REQUIRED_VARS DLSS_ROOT_DIR DLSS_INCLUDE_DIR DLSS_LIBRARY
        VERSION_VAR DLSS_VERSION
    )
endif()

if(DLSS_FOUND)
    set(DLSS_INCLUDE_DIRS ${DLSS_INCLUDE_DIR})
    set(DLSS_LIBRARIES ${DLSS_LIBRARY})
    
    # Добавление дополнительных библиотек если найдены
    if(DLSS_DLSSG_LIBRARY)
        list(APPEND DLSS_LIBRARIES ${DLSS_DLSSG_LIBRARY})
    endif()
    
    if(DLSS_COMMON_LIBRARY)
        list(APPEND DLSS_LIBRARIES ${DLSS_COMMON_LIBRARY})
    endif()
    
    # Создание imported target
    if(NOT TARGET DLSS::DLSS)
        add_library(DLSS::DLSS INTERFACE IMPORTED)
        target_include_directories(DLSS::DLSS INTERFACE ${DLSS_INCLUDE_DIRS})
        target_link_libraries(DLSS::DLSS INTERFACE ${DLSS_LIBRARIES})
        
        # Добавление определений компилятора
        target_compile_definitions(DLSS::DLSS INTERFACE 
            DLSS_AVAILABLE=1
            SL_PRODUCTION=1
        )
    endif()
    
    if(NOT DLSS_FIND_QUIETLY)
        message(STATUS "Found DLSS: ${DLSS_ROOT_DIR} (version ${DLSS_VERSION})")
    endif()
endif()

# Скрытие внутренних переменных от пользователя
mark_as_advanced(
    DLSS_ROOT_DIR
    DLSS_INCLUDE_DIR
    DLSS_LIBRARY
    DLSS_DLSSG_LIBRARY
    DLSS_COMMON_LIBRARY
)
