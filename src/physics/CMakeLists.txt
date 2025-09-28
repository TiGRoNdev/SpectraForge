# Физическая система HyperEngine
cmake_minimum_required(VERSION 3.16)

message(STATUS "Настройка HyperEngine_Physics")

# Пока заглушка - будет реализовано в следующих этапах
add_library(HyperEngine_Physics INTERFACE)

target_include_directories(HyperEngine_Physics 
    INTERFACE ${CMAKE_SOURCE_DIR}/include
)

# Создание alias
add_library(HyperEngine::Physics ALIAS HyperEngine_Physics)

message(STATUS "HyperEngine_Physics заглушка создана")
