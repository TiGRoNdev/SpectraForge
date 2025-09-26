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

# Поиск пакетов
find_package(OpenGL REQUIRED)

# Поиск GLFW3 через vcpkg
find_package(glfw3 CONFIG REQUIRED)

# Поиск GLEW через vcpkg
find_package(GLEW REQUIRED)

# Создание библиотеки движка
file(GLOB_RECURSE ENGINE_SOURCES 
    "src/*.cpp"
    "src/*.h"
)

add_library(Engine4D STATIC ${ENGINE_SOURCES})

# Подключение библиотек к движку
target_link_libraries(Engine4D PRIVATE 
    OpenGL::GL 
    glfw
    GLEW::GLEW
)

# Создание исполняемого файла демо
add_executable(Engine4D_Demo 
    examples/demo/main.cpp
)

target_link_libraries(Engine4D_Demo 
    Engine4D
    OpenGL::GL 
    glfw
    GLEW::GLEW
)

# Установка
install(TARGETS Engine4D Engine4D_Demo
    RUNTIME DESTINATION bin
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
)

# Установка заголовочных файлов
install(DIRECTORY include/ DESTINATION include)
