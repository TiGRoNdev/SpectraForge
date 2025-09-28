# Основной CMakeLists для новой структуры HyperEngine
cmake_minimum_required(VERSION 3.16)

message(STATUS "Настройка новой модульной структуры HyperEngine")

# Математическая библиотека (независимая)
add_subdirectory(math)

# Основные компоненты движка
add_subdirectory(core)

# Система рендеринга
add_subdirectory(rendering)

# Физическая система
add_subdirectory(physics)

# Система ввода
add_subdirectory(input)

# Платформо-зависимый код
add_subdirectory(platform)

message(STATUS "Все модули HyperEngine настроены")
