# Система рендеринга HyperEngine
cmake_minimum_required(VERSION 3.16)

message(STATUS "Настройка HyperEngine_Rendering")

# Пока заглушка - будет реализовано в следующих этапах
add_library(HyperEngine_Rendering INTERFACE)

target_include_directories(HyperEngine_Rendering 
    INTERFACE ${CMAKE_SOURCE_DIR}/include
)

# Создание alias
add_library(HyperEngine::Rendering ALIAS HyperEngine_Rendering)

message(STATUS "HyperEngine_Rendering заглушка создана")
