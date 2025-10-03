#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

// ЭТО ЛОМАЕТ GLFW:
#include <vulkan/vulkan.hpp>

#include <iostream>

int main() {
    std::cout << "=== Test: GLFW + vulkan.hpp Conflict ===" << std::endl;
    
    if (!glfwInit()) {
        std::cerr << "ERROR: glfwInit failed" << std::endl;
        return 1;
    }
    
    std::cout << "✓ glfwInit OK" << std::endl;
    
    if (!glfwVulkanSupported()) {
        std::cerr << "ERROR: Vulkan not supported" << std::endl;
        glfwTerminate();
        return 1;
    }
    
    std::cout << "✓ glfwVulkanSupported OK" << std::endl;
    
    // Попробуем получить расширения ПОСЛЕ включения vulkan.hpp
    uint32_t count = 999;
    const char** exts = glfwGetRequiredInstanceExtensions(&count);
    
    std::cout << "glfwGetRequiredInstanceExtensions ПОСЛЕ включения vulkan.hpp:" << std::endl;
    std::cout << "  Pointer: " << static_cast<const void*>(exts) << std::endl;
    std::cout << "  Count: " << count << std::endl;
    
    if (!exts || count == 0) {
        std::cerr << "❌ FAIL: glfwGetRequiredInstanceExtensions вернул NULL/0" << std::endl;
        std::cerr << "   Причина: vulkan.hpp ПЕРЕОПРЕДЕЛИЛ функции, сломав GLFW!" << std::endl;
        glfwTerminate();
        return 1;
    }
    
    std::cout << "✅ SUCCESS: Extensions получены" << std::endl;
    for (uint32_t i = 0; i < count; ++i) {
        std::cout << "    - " << exts[i] << std::endl;
    }
    
    glfwTerminate();
    return 0;
}

