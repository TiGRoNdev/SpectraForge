cmake_minimum_required(VERSION 3.16)
project(Engine4D VERSION 1.0.0 LANGUAGES CXX)

# Установка стандарта C++
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Настройки компилятора
if(MSVC)
    add_compile_options(/W4)
else()
    add_compile_options(-Wall -Wextra -Wpedantic)
endif()

# Включение директорий
include_directories(${CMAKE_SOURCE_DIR}/src)
include_directories(${CMAKE_SOURCE_DIR}/include)

# Включение GLEW
find_package(GLEW REQUIRED)                                                                                             
target_link_libraries(Engine4D PRIVATE GLEW::GLEW) 

# Создание библиотеки движка
file(GLOB_RECURSE ENGINE_SOURCES 
    "src/*.cpp"
    "src/*.h"
)

add_library(Engine4D STATIC ${ENGINE_SOURCES})

# Создание исполняемого файла демо
add_executable(Engine4D_Demo 
    examples/demo/main.cpp
)

target_link_libraries(Engine4D_Demo Engine4D)

# Установка
install(TARGETS Engine4D Engine4D_Demo
    RUNTIME DESTINATION bin
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
)

# Установка заголовочных файлов
install(DIRECTORY include/ DESTINATION include)
