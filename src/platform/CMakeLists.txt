# Платформо-зависимый код HyperEngine
cmake_minimum_required(VERSION 3.16)

message(STATUS "Настройка HyperEngine_Platform")

# Пока заглушка - будет реализовано в следующих этапах
add_library(HyperEngine_Platform INTERFACE)

target_include_directories(HyperEngine_Platform 
    INTERFACE ${CMAKE_SOURCE_DIR}/include
)

# Создание alias
add_library(HyperEngine::Platform ALIAS HyperEngine_Platform)

message(STATUS "HyperEngine_Platform заглушка создана")
