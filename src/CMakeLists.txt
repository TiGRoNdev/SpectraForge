# SpectraForge Source Modules

# Подключаем модули
add_subdirectory(math)
add_subdirectory(core)
add_subdirectory(platform)

# App module (высокоуровневый фасад Engine)
add_subdirectory(app)

# Vulkan модуль
add_subdirectory(rendering)

# Performance benchmarking
add_subdirectory(performance)