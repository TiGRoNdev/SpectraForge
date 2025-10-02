# Vulkan рендерер SpectraForge
cmake_minimum_required(VERSION 3.16)

message(STATUS "Настройка SpectraForge_Vulkan")

# Создаем библиотеку Vulkan рендерера
add_library(SpectraForge_Vulkan STATIC
    VulkanRenderer.cpp
    VulkanEngine.cpp
    ResourceManager.cpp
    SceneManager.cpp
    HardwareDetector.cpp
)

# Устанавливаем стандарт C++
target_compile_features(SpectraForge_Vulkan PRIVATE cxx_std_17)

# Включаем директории
target_include_directories(SpectraForge_Vulkan
    PUBLIC
        ${CMAKE_SOURCE_DIR}/include
        ${VMA_INCLUDE_DIR}
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}
)

# Подключаем зависимости
if(TARGET Vulkan::Vulkan)
    target_link_libraries(SpectraForge_Vulkan PUBLIC Vulkan::Vulkan)
endif()

# Опциональные зависимости Vulkan
if(TARGET VulkanMemoryAllocator::VulkanMemoryAllocator)
    target_link_libraries(SpectraForge_Vulkan PUBLIC VulkanMemoryAllocator::VulkanMemoryAllocator)
    target_compile_definitions(SpectraForge_Vulkan PRIVATE VULKAN_MEMORY_ALLOCATOR_AVAILABLE)
endif()

if(TARGET spirv_cross_core::spirv_cross_core)
    target_link_libraries(SpectraForge_Vulkan PUBLIC spirv_cross_core::spirv_cross_core)
    target_compile_definitions(SpectraForge_Vulkan PRIVATE SPIRV_CROSS_AVAILABLE)
endif()

if(TARGET unofficial::shaderc::shaderc)
    target_link_libraries(SpectraForge_Vulkan PUBLIC unofficial::shaderc::shaderc)
    target_compile_definitions(SpectraForge_Vulkan PRIVATE SHADERC_AVAILABLE)
endif()

# Подключаем математическую библиотеку
if(TARGET SpectraForge_Math)
    target_link_libraries(SpectraForge_Vulkan PUBLIC SpectraForge_Math)
endif()

# Подключаем Core библиотеку
if(TARGET SpectraForge_Core)
    target_link_libraries(SpectraForge_Vulkan PUBLIC SpectraForge_Core)
endif()

# Определения компилятора
target_compile_definitions(SpectraForge_Vulkan
    PUBLIC
        SpectraForge_ENABLE_VULKAN
    PRIVATE
        VULKAN_HPP_DISPATCH_LOADER_DYNAMIC=1
)

# Платформо-зависимые определения
if(WIN32)
    target_compile_definitions(SpectraForge_Vulkan PUBLIC VK_USE_PLATFORM_WIN32_KHR)
endif()

# Настройки компилятора
if(MSVC)
    target_compile_options(SpectraForge_Vulkan PRIVATE /W4)
else()
    target_compile_options(SpectraForge_Vulkan PRIVATE -Wall -Wextra -Wpedantic)
endif()

# Создание alias
add_library(SpectraForge::Vulkan ALIAS SpectraForge_Vulkan)

message(STATUS "SpectraForge_Vulkan настроен")
