# SpectraForge Source Modules

# Подключаем модули
add_subdirectory(math)
add_subdirectory(core)
add_subdirectory(platform)

# App module (высокоуровневый фасад Engine)
add_subdirectory(app)

# Vulkan модуль
add_subdirectory(rendering)

# Output helpers (PNG writer)
add_subdirectory(output)

# Performance benchmarking
add_subdirectory(performance)

# Input module
add_subdirectory(input)

# Physics module
add_subdirectory(physics)

# Upscaling module
add_subdirectory(upscaling)