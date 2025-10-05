/**
 * @file VMAImplementation.cpp
 * @brief Vulkan Memory Allocator implementation file
 * 
 * This file defines VMA_IMPLEMENTATION to compile VMA functions.
 * Must be included exactly once in the project.
 * 
 * @author SpectraForge Core Team
 * @date 2025-10-03
 */

#define VMA_IMPLEMENTATION
// Use static Vulkan functions (linked at compile time)
#define VMA_STATIC_VULKAN_FUNCTIONS 1
#define VMA_DYNAMIC_VULKAN_FUNCTIONS 0

#include <vk_mem_alloc.h>

