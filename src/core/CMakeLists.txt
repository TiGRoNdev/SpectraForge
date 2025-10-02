# Основные компоненты движка SpectraForge
cmake_minimum_required(VERSION 3.16)

message(STATUS "Настройка SpectraForge_Core")

# Определить корневую директорию проекта
get_filename_component(PROJECT_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/../.." ABSOLUTE)

# Исходные файлы ядра
set(CORE_SOURCES
    Container.cpp
)

# Заголовочные файлы ядра
set(CORE_HEADERS
    ${PROJECT_ROOT}/include/SpectraForge/Core/Console.h
    ${PROJECT_ROOT}/include/SpectraForge/Core/DependencyInjection/Container.h
)

# Создание статической библиотеки
add_library(SpectraForge_Core STATIC ${CORE_SOURCES} ${CORE_HEADERS})

# Настройка include директорий
target_include_directories(SpectraForge_Core 
    PUBLIC 
        ${PROJECT_ROOT}/include
    PRIVATE 
        ${CMAKE_CURRENT_SOURCE_DIR}
)

# Требования к стандарту C++
target_compile_features(SpectraForge_Core PUBLIC cxx_std_20)

# Настройки компилятора
if(MSVC)
    target_compile_options(SpectraForge_Core PRIVATE /W4)
else()
    target_compile_options(SpectraForge_Core PRIVATE -Wall -Wextra -Wpedantic)
endif()

# Установка свойств
set_target_properties(SpectraForge_Core PROPERTIES
    OUTPUT_NAME "SpectraForge_Core"
    DEBUG_POSTFIX "_d"
    FOLDER "SpectraForge/Libraries"
)

# Создание alias
add_library(SpectraForge::Core ALIAS SpectraForge_Core)

message(STATUS "SpectraForge_Core библиотека настроена")
