# Система рендеринга SpectraForge
cmake_minimum_required(VERSION 3.16)

message(STATUS "Настройка SpectraForge_Rendering")

# Пока заглушка - будет реализовано в следующих этапах
add_library(SpectraForge_Rendering INTERFACE)

target_include_directories(SpectraForge_Rendering 
    INTERFACE ${CMAKE_SOURCE_DIR}/include
)

# Создание alias
add_library(SpectraForge::Rendering ALIAS SpectraForge_Rendering)

message(STATUS "SpectraForge_Rendering заглушка создана")
