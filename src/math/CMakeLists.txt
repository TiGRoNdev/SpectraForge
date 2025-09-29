# Математическая библиотека HyperEngine
cmake_minimum_required(VERSION 3.16)

message(STATUS "Настройка HyperEngine_Math")

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
    ${PROJECT_ROOT}/include/HyperEngine/Math/Vector2.h
    ${PROJECT_ROOT}/include/HyperEngine/Math/Vector3.h
    ${PROJECT_ROOT}/include/HyperEngine/Math/Vector4.h
    ${PROJECT_ROOT}/include/HyperEngine/Math/Matrix4.h
    ${PROJECT_ROOT}/include/HyperEngine/Math/Quaternion.h
    ${PROJECT_ROOT}/include/HyperEngine/Math/Math.h
    ${PROJECT_ROOT}/include/HyperEngine/Math/MathConstants.h
)

# Создание статической библиотеки
add_library(HyperEngine_Math STATIC ${MATH_SOURCES} ${MATH_HEADERS})

# Настройка include директорий
target_include_directories(HyperEngine_Math 
    PUBLIC 
        ${PROJECT_ROOT}/include
    PRIVATE 
        ${CMAKE_CURRENT_SOURCE_DIR}
)

# Требования к стандарту C++
target_compile_features(HyperEngine_Math PUBLIC cxx_std_20)

# Настройки компилятора
if(MSVC)
    target_compile_options(HyperEngine_Math PRIVATE /W4)
else()
    target_compile_options(HyperEngine_Math PRIVATE -Wall -Wextra -Wpedantic)
endif()

# Установка свойств
set_target_properties(HyperEngine_Math PROPERTIES
    OUTPUT_NAME "HyperEngine_Math"
    DEBUG_POSTFIX "_d"
    FOLDER "HyperEngine/Libraries"
)

message(STATUS "HyperEngine_Math библиотека настроена")

# Создание alias для удобства
add_library(HyperEngine::Math ALIAS HyperEngine_Math)
