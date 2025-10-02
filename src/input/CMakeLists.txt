# Система ввода SpectraForge
cmake_minimum_required(VERSION 3.16)

message(STATUS "Настройка SpectraForge_Input")

# Пока заглушка - будет реализовано в следующих этапах
add_library(SpectraForge_Input INTERFACE)

target_include_directories(SpectraForge_Input 
    INTERFACE ${CMAKE_SOURCE_DIR}/include
)

# Создание alias
add_library(SpectraForge::Input ALIAS SpectraForge_Input)

message(STATUS "SpectraForge_Input заглушка создана")
