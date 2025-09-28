# Основные компоненты движка HyperEngine
cmake_minimum_required(VERSION 3.16)

message(STATUS "Настройка HyperEngine_Core")

# Определить корневую директорию проекта
get_filename_component(PROJECT_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/../.." ABSOLUTE)

# Исходные файлы ядра
set(CORE_SOURCES
    Container.cpp
)

# Заголовочные файлы ядра
set(CORE_HEADERS
    ${PROJECT_ROOT}/include/HyperEngine/Core/Console.h
    ${PROJECT_ROOT}/include/HyperEngine/Core/DependencyInjection/Container.h
)

# Создание статической библиотеки
add_library(HyperEngine_Core STATIC ${CORE_SOURCES} ${CORE_HEADERS})

# Настройка include директорий
target_include_directories(HyperEngine_Core 
    PUBLIC 
        ${PROJECT_ROOT}/include
    PRIVATE 
        ${CMAKE_CURRENT_SOURCE_DIR}
)

# Требования к стандарту C++
target_compile_features(HyperEngine_Core PUBLIC cxx_std_20)

# Настройки компилятора
if(MSVC)
    target_compile_options(HyperEngine_Core PRIVATE /W4)
else()
    target_compile_options(HyperEngine_Core PRIVATE -Wall -Wextra -Wpedantic)
endif()

# Установка свойств
set_target_properties(HyperEngine_Core PROPERTIES
    OUTPUT_NAME "HyperEngine_Core"
    DEBUG_POSTFIX "_d"
    FOLDER "HyperEngine/Libraries"
)

# Создание alias
add_library(HyperEngine::Core ALIAS HyperEngine_Core)

message(STATUS "HyperEngine_Core библиотека настроена")
