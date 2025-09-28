# Основные компоненты движка HyperEngine
cmake_minimum_required(VERSION 3.16)

message(STATUS "Настройка HyperEngine_Core")

# Пока заглушка - будет реализовано в следующих этапах
# Создаем пустую библиотеку для валидации структуры
add_library(HyperEngine_Core INTERFACE)

target_include_directories(HyperEngine_Core 
    INTERFACE ${CMAKE_SOURCE_DIR}/include
)

# Создание alias
add_library(HyperEngine::Core ALIAS HyperEngine_Core)

message(STATUS "HyperEngine_Core заглушка создана")
