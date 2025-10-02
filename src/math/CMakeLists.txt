# Математическая библиотека SpectraForge
cmake_minimum_required(VERSION 3.16)

message(STATUS "Настройка SpectraForge_Math")

# Исходные файлы математической библиотеки
set(MATH_SOURCES
    vector/Vector3.cpp
    vector/Vector4.cpp
    matrix/Matrix4.cpp
    matrix/Quaternion.cpp
)

# Определить корневую директорию проекта
get_filename_component(PROJECT_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/../.." ABSOLUTE)

# Заголовочные файлы
set(MATH_HEADERS
    ${PROJECT_ROOT}/include/SpectraForge/Math/Vector2.h
    ${PROJECT_ROOT}/include/SpectraForge/Math/Vector3.h
    ${PROJECT_ROOT}/include/SpectraForge/Math/Vector4.h
    ${PROJECT_ROOT}/include/SpectraForge/Math/Matrix4.h
    ${PROJECT_ROOT}/include/SpectraForge/Math/Quaternion.h
    ${PROJECT_ROOT}/include/SpectraForge/Math/Math.h
    ${PROJECT_ROOT}/include/SpectraForge/Math/MathConstants.h
)

# Создание статической библиотеки
add_library(SpectraForge_Math STATIC ${MATH_SOURCES} ${MATH_HEADERS})

# Настройка include директорий
target_include_directories(SpectraForge_Math 
    PUBLIC 
        ${PROJECT_ROOT}/include
    PRIVATE 
        ${CMAKE_CURRENT_SOURCE_DIR}
)

# Требования к стандарту C++
target_compile_features(SpectraForge_Math PUBLIC cxx_std_20)

# Настройки компилятора
if(MSVC)
    target_compile_options(SpectraForge_Math PRIVATE /W4)
else()
    target_compile_options(SpectraForge_Math PRIVATE -Wall -Wextra -Wpedantic)
endif()

# Установка свойств
set_target_properties(SpectraForge_Math PROPERTIES
    OUTPUT_NAME "SpectraForge_Math"
    DEBUG_POSTFIX "_d"
    FOLDER "SpectraForge/Libraries"
)

message(STATUS "SpectraForge_Math библиотека настроена")

# Создание alias для удобства
add_library(SpectraForge::Math ALIAS SpectraForge_Math)
