# Физическая система SpectraForge
cmake_minimum_required(VERSION 3.16)

message(STATUS "Настройка SpectraForge_Physics")

# Пока заглушка - будет реализовано в следующих этапах
add_library(SpectraForge_Physics INTERFACE)

target_include_directories(SpectraForge_Physics 
    INTERFACE ${CMAKE_SOURCE_DIR}/include
)

# Создание alias
add_library(SpectraForge::Physics ALIAS SpectraForge_Physics)

message(STATUS "SpectraForge_Physics заглушка создана")
