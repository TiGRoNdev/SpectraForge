# Vulkan рендерер HyperEngine
cmake_minimum_required(VERSION 3.16)

message(STATUS "Настройка HyperEngine_Vulkan")

# Создаем библиотеку Vulkan рендерера
add_library(HyperEngine_Vulkan STATIC
    VulkanRenderer.cpp
    VulkanEngine.cpp
    ResourceManager.cpp
    SceneManager.cpp
    HardwareDetector.cpp
)

# Устанавливаем стандарт C++
target_compile_features(HyperEngine_Vulkan PRIVATE cxx_std_17)

# Включаем директории
target_include_directories(HyperEngine_Vulkan
    PUBLIC ${CMAKE_SOURCE_DIR}/include
    PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}
)

# Подключаем зависимости
if(TARGET Vulkan::Vulkan)
    target_link_libraries(HyperEngine_Vulkan PUBLIC Vulkan::Vulkan)
endif()

# Опциональные зависимости Vulkan
if(TARGET VulkanMemoryAllocator::VulkanMemoryAllocator)
    target_link_libraries(HyperEngine_Vulkan PUBLIC VulkanMemoryAllocator::VulkanMemoryAllocator)
    target_compile_definitions(HyperEngine_Vulkan PRIVATE VULKAN_MEMORY_ALLOCATOR_AVAILABLE)
endif()

if(TARGET spirv_cross_core::spirv_cross_core)
    target_link_libraries(HyperEngine_Vulkan PUBLIC spirv_cross_core::spirv_cross_core)
    target_compile_definitions(HyperEngine_Vulkan PRIVATE SPIRV_CROSS_AVAILABLE)
endif()

if(TARGET unofficial::shaderc::shaderc)
    target_link_libraries(HyperEngine_Vulkan PUBLIC unofficial::shaderc::shaderc)
    target_compile_definitions(HyperEngine_Vulkan PRIVATE SHADERC_AVAILABLE)
endif()

# Подключаем математическую библиотеку
if(TARGET HyperEngine_Math)
    target_link_libraries(HyperEngine_Vulkan PUBLIC HyperEngine_Math)
endif()

# Подключаем Core библиотеку
if(TARGET HyperEngine_Core)
    target_link_libraries(HyperEngine_Vulkan PUBLIC HyperEngine_Core)
endif()

# Определения компилятора
target_compile_definitions(HyperEngine_Vulkan
    PUBLIC
        HyperEngine_ENABLE_VULKAN
        VK_USE_PLATFORM_WIN32_KHR  # Для Windows
    PRIVATE
        VULKAN_HPP_DISPATCH_LOADER_DYNAMIC=1
)

# Настройки компилятора
if(MSVC)
    target_compile_options(HyperEngine_Vulkan PRIVATE /W4)
else()
    target_compile_options(HyperEngine_Vulkan PRIVATE -Wall -Wextra -Wpedantic)
endif()

# Создание alias
add_library(HyperEngine::Vulkan ALIAS HyperEngine_Vulkan)

message(STATUS "HyperEngine_Vulkan настроен")
