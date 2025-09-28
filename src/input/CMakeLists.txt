# Система ввода HyperEngine
cmake_minimum_required(VERSION 3.16)

message(STATUS "Настройка HyperEngine_Input")

# Пока заглушка - будет реализовано в следующих этапах
add_library(HyperEngine_Input INTERFACE)

target_include_directories(HyperEngine_Input 
    INTERFACE ${CMAKE_SOURCE_DIR}/include
)

# Создание alias
add_library(HyperEngine::Input ALIAS HyperEngine_Input)

message(STATUS "HyperEngine_Input заглушка создана")
