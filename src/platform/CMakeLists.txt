# Платформо-зависимый код SpectraForge
cmake_minimum_required(VERSION 3.16)

message(STATUS "Настройка SpectraForge_Platform")

# Пока заглушка - будет реализовано в следующих этапах
add_library(SpectraForge_Platform INTERFACE)

target_include_directories(SpectraForge_Platform 
    INTERFACE ${CMAKE_SOURCE_DIR}/include
)

# Создание alias
add_library(SpectraForge::Platform ALIAS SpectraForge_Platform)

message(STATUS "SpectraForge_Platform заглушка создана")
