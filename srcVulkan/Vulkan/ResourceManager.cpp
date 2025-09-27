/**
 * @file ResourceManager.cpp
 * @brief Реализация менеджера ресурсов Vulkan
 * 
 * Управляет памятью, буферами и текстурами с использованием VMA.
 * Поддерживает CUDA-Vulkan interop для гибридного рендеринга.
 */

#include "Engine3D/Vulkan/ResourceManager.h"
#include <iostream>
#include <stdexcept>
#include <cstring>

using namespace Engine3D::Vulkan;

namespace Engine3D::Vulkan {

ResourceManager::ResourceManager() {
    // Инициализация в init()
}

ResourceManager::~ResourceManager() {
    if (initialized) {
        shutdown();
    }
}

bool ResourceManager::init(vk::PhysicalDevice physDevice, vk::Device logDevice, vk::Instance inst) {
    try {
        this->physicalDevice = physDevice;
        this->device = logDevice;
        this->instance = inst;
        
        std::cout << "[ResourceManager] Инициализация менеджера ресурсов..." << std::endl;
        
        // Создаем VMA аллокатор
        if (!createAllocator()) {
            std::cerr << "[ResourceManager] Ошибка создания VMA аллокатора" << std::endl;
            return false;
        }
        
        initialized = true;
        std::cout << "[ResourceManager] Инициализация завершена успешно" << std::endl;
        
        // Выводим статистику памяти
        auto stats = getMemoryStatistics();
        std::cout << "[ResourceManager] Доступная память: " 
                  << (stats.total.statistics.allocationBytes / (1024 * 1024)) << " MB" << std::endl;
        
        return true;
        
    } catch (const std::exception& e) {
        std::cerr << "[ResourceManager] Ошибка инициализации: " << e.what() << std::endl;
        return false;
    }
}

void ResourceManager::shutdown() {
    if (!initialized) {
        return;
    }
    
    std::cout << "[ResourceManager] Завершение работы менеджера ресурсов..." << std::endl;
    
    // Освобождаем все оставшиеся ресурсы (заглушка)
    std::cout << "[ResourceManager] Освобождение " << bufferAllocations.size() << " буферов (заглушка)" << std::endl;
    bufferAllocations.clear();
    
    std::cout << "[ResourceManager] Освобождение " << imageAllocations.size() << " изображений (заглушка)" << std::endl;
    imageAllocations.clear();
    
    // Уничтожаем аллокатор (заглушка)
    if (allocator != VK_NULL_HANDLE) {
        std::cout << "[ResourceManager] Уничтожение VMA аллокатора (заглушка)" << std::endl;
        allocator = VK_NULL_HANDLE;
    }
    
    initialized = false;
    std::cout << "[ResourceManager] Завершение работы завершено" << std::endl;
}

vk::Buffer ResourceManager::allocateBuffer(size_t size, vk::BufferUsageFlags usage, VmaMemoryUsage memoryUsage) {
    if (!initialized) {
        throw std::runtime_error("ResourceManager не инициализирован");
    }
    
    std::cout << "[ResourceManager] Создание буфера размером " << (size / 1024) << " KB (заглушка)" << std::endl;
    
    // TODO: Реальное создание буфера через VMA на следующих этапах
    // Пока возвращаем фиктивный буфер
    VkBuffer buffer = reinterpret_cast<VkBuffer>(static_cast<uintptr_t>(bufferAllocations.size() + 1));
    
    // Сохраняем информацию об аллокации (заглушка)
    AllocationInfo info;
    info.allocation = reinterpret_cast<VmaAllocation>(0x1);
    info.size = size;
    
    bufferAllocations[buffer] = info;
    
    return vk::Buffer(buffer);
}

vk::Image ResourceManager::createTexture(const ImageData& data) {
    if (!initialized) {
        throw std::runtime_error("ResourceManager не инициализирован");
    }
    
    std::cout << "[ResourceManager] Создание изображения " << data.width << "x" << data.height << " (заглушка)" << std::endl;
    
    // TODO: Реальное создание изображения через VMA на следующих этапах
    // Пока возвращаем фиктивное изображение
    VkImage image = reinterpret_cast<VkImage>(static_cast<uintptr_t>(imageAllocations.size() + 1));
    
    // Сохраняем информацию об аллокации (заглушка)
    AllocationInfo info;
    info.allocation = reinterpret_cast<VmaAllocation>(0x1);
    info.size = data.dataSize;
    
    imageAllocations[image] = info;
    
    return vk::Image(image);
}

void ResourceManager::freeBuffer(vk::Buffer buffer) {
    if (!initialized) {
        return;
    }
    
    VkBuffer vkBuffer = static_cast<VkBuffer>(buffer);
    auto it = bufferAllocations.find(vkBuffer);
    
    if (it != bufferAllocations.end()) {
        bufferAllocations.erase(it);
        std::cout << "[ResourceManager] Буфер освобожден (заглушка)" << std::endl;
    }
}

void ResourceManager::freeImage(vk::Image image) {
    if (!initialized) {
        return;
    }
    
    VkImage vkImage = static_cast<VkImage>(image);
    auto it = imageAllocations.find(vkImage);
    
    if (it != imageAllocations.end()) {
        imageAllocations.erase(it);
        std::cout << "[ResourceManager] Изображение освобождено (заглушка)" << std::endl;
    }
}

void* ResourceManager::mapBuffer(vk::Buffer buffer) {
    if (!initialized) {
        return nullptr;
    }
    
    std::cout << "[ResourceManager] Отображение буфера (заглушка)" << std::endl;
    
    // TODO: Реальное отображение памяти через VMA на следующих этапах
    // Пока возвращаем фиктивный указатель
    return reinterpret_cast<void*>(0x1000);
}

void ResourceManager::unmapBuffer(vk::Buffer buffer) {
    if (!initialized) {
        return;
    }
    
    std::cout << "[ResourceManager] Отмена отображения буфера (заглушка)" << std::endl;
    
    // TODO: Реальная отмена отображения через VMA на следующих этапах
}

void ResourceManager::updateBuffer(vk::Buffer buffer, const void* data, size_t size, size_t offset) {
    if (!initialized || !data) {
        return;
    }
    
    void* mappedData = mapBuffer(buffer);
    if (mappedData) {
        std::memcpy(static_cast<char*>(mappedData) + offset, data, size);
        unmapBuffer(buffer);
    }
}

#ifdef VULKAN_RENDERER_CUDA_SUPPORT
vk::DeviceMemory ResourceManager::manageInterop(const CUDAResource& cudaRes) {
    try {
        std::cout << "[ResourceManager] Создание external memory для CUDA interop" << std::endl;
        
        // Создаем external memory из CUDA ресурса
        vk::MemoryAllocateInfo allocInfo{};
        allocInfo.allocationSize = cudaRes.size;
        
        // Найдем подходящий тип памяти для external memory
        allocInfo.memoryTypeIndex = findMemoryType(
            0xFFFFFFFF, // Любой тип памяти
            vk::MemoryPropertyFlagBits::eDeviceLocal);
        
        // Добавляем информацию для импорта external memory
        vk::ImportMemoryWin32HandleInfoKHR importInfo{};
        importInfo.handleType = vk::ExternalMemoryHandleTypeFlagBits::eOpaqueWin32;
        // importInfo.handle = cudaRes.handle; // TODO: Получить handle из CUDA
        allocInfo.pNext = &importInfo;
        
        auto memory = device.allocateMemory(allocInfo);
        
        std::cout << "[ResourceManager] External memory создана успешно" << std::endl;
        return memory;
        
    } catch (const std::exception& e) {
        std::cerr << "[ResourceManager] Ошибка создания external memory: " << e.what() << std::endl;
        return vk::DeviceMemory{};
    }
}

vk::Buffer ResourceManager::createSharedBuffer(size_t size, vk::BufferUsageFlags usage) {
    try {
        std::cout << "[ResourceManager] Создание shared буфера размером " << size << " байт" << std::endl;
        
        // Создаем буфер с external memory support
        vk::BufferCreateInfo bufferInfo{};
        bufferInfo.size = size;
        bufferInfo.usage = usage;
        bufferInfo.sharingMode = vk::SharingMode::eExclusive;
        
        // Добавляем external memory support
        vk::ExternalMemoryBufferCreateInfo extMemInfo{};
        extMemInfo.handleTypes = vk::ExternalMemoryHandleTypeFlagBits::eOpaqueWin32;
        bufferInfo.pNext = &extMemInfo;
        
        auto buffer = device.createBuffer(bufferInfo);
        
        // Получаем требования к памяти
        auto memRequirements = device.getBufferMemoryRequirements(buffer);
        
        // Создаем external memory
        vk::MemoryAllocateInfo allocInfo{};
        allocInfo.allocationSize = memRequirements.size;
        allocInfo.memoryTypeIndex = findMemoryType(
            memRequirements.memoryTypeBits,
            vk::MemoryPropertyFlagBits::eDeviceLocal);
        
        vk::ExportMemoryAllocateInfo exportInfo{};
        exportInfo.handleTypes = vk::ExternalMemoryHandleTypeFlagBits::eOpaqueWin32;
        allocInfo.pNext = &exportInfo;
        
        auto memory = device.allocateMemory(allocInfo);
        
        // Привязываем буфер к памяти
        device.bindBufferMemory(buffer, memory, 0);
        
        // Сохраняем информацию об аллокации
        AllocationInfo allocInfo_cache{};
        allocInfo_cache.allocation = VK_NULL_HANDLE; // Для external memory VMA не используется
        allocInfo_cache.size = size;
        bufferAllocations[buffer] = allocInfo_cache;
        
        std::cout << "[ResourceManager] Shared буфер создан успешно" << std::endl;
        return buffer;
        
    } catch (const std::exception& e) {
        std::cerr << "[ResourceManager] Ошибка создания shared буфера: " << e.what() << std::endl;
        return vk::Buffer{};
    }
}

cudaExternalMemory_t ResourceManager::exportMemoryToCUDA(vk::DeviceMemory memory) {
    try {
        std::cout << "[ResourceManager] Экспорт Vulkan памяти в CUDA" << std::endl;
        
        // Получаем Windows handle от Vulkan memory
        vk::MemoryGetWin32HandleInfoKHR handleInfo{};
        handleInfo.memory = memory;
        handleInfo.handleType = vk::ExternalMemoryHandleTypeFlagBits::eOpaqueWin32;
        
        HANDLE handle = device.getMemoryWin32HandleKHR(handleInfo);
        
        // Создаем CUDA external memory
        cudaExternalMemoryHandleDesc memHandleDesc{};
        memHandleDesc.type = cudaExternalMemoryHandleTypeOpaqueWin32;
        memHandleDesc.handle.win32.handle = handle;
        memHandleDesc.size = 0; // Размер будет определен автоматически
        
        cudaExternalMemory_t extMem;
        cudaError_t result = cudaImportExternalMemory(&extMem, &memHandleDesc);
        
        if (result != cudaSuccess) {
            std::cerr << "[ResourceManager] Ошибка импорта памяти в CUDA: " 
                      << cudaGetErrorString(result) << std::endl;
            return nullptr;
        }
        
        std::cout << "[ResourceManager] Память успешно экспортирована в CUDA" << std::endl;
        return extMem;
        
    } catch (const std::exception& e) {
        std::cerr << "[ResourceManager] Ошибка экспорта памяти в CUDA: " << e.what() << std::endl;
        return nullptr;
    }
}
#endif

VmaTotalStatistics ResourceManager::getMemoryStatistics() const {
    VmaTotalStatistics stats{};
    
    std::cout << "[ResourceManager] Получение статистики памяти (заглушка)" << std::endl;
    
    // TODO: Реальная статистика через VMA на следующих этапах
    // Пока возвращаем пустую статистику
    
    return stats;
}

// Приватные методы

bool ResourceManager::createAllocator() {
    std::cout << "[ResourceManager] VMA аллокатор создан (заглушка для этапа 2.1)" << std::endl;
    
    // TODO: Реальная инициализация VMA на следующих этапах
    // Пока просто возвращаем успех
    allocator = reinterpret_cast<VmaAllocator>(0x1); // Фиктивный указатель
    return true;
}

uint32_t ResourceManager::findMemoryType(uint32_t typeFilter, vk::MemoryPropertyFlags properties) {
    auto memProperties = physicalDevice.getMemoryProperties();
    
    for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++) {
        if ((typeFilter & (1 << i)) && 
            (memProperties.memoryTypes[i].propertyFlags & properties) == properties) {
            return i;
        }
    }
    
    throw std::runtime_error("Не найден подходящий тип памяти");
}

vk::CommandBuffer ResourceManager::beginSingleTimeCommands() {
    // TODO: Реализация single-time command buffer
    // Требует command pool, который будет создан позже
    return vk::CommandBuffer{};
}

void ResourceManager::endSingleTimeCommands(vk::CommandBuffer commandBuffer) {
    // TODO: Завершение single-time command buffer
}

} // namespace Engine3D::Vulkan
