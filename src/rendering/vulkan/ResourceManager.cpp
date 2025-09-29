/**
 * @file ResourceManager.cpp
 * @brief Реализация менеджера ресурсов Vulkan
 *
 * Управляет памятью, буферами и текстурами с использованием VMA.
 * Поддерживает CUDA-Vulkan interop для гибридного рендеринга.
 */

#include "HyperEngine/Vulkan/ResourceManager.h"
#include <cstring>
#include <iostream>
#include <stdexcept>
#include "HyperEngine/Core/SafeConsole.h"

#ifdef _WIN32
#include <windows.h>
#include "HyperEngine/Core/Console.h"
#include "HyperEngine/Core/SafeConsole.h"
#define VK_USE_PLATFORM_WIN32_KHR
#endif

using namespace HyperEngine::Vulkan;
using namespace HyperEngine::Core;

namespace HyperEngine::Vulkan {

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

        SAFE_PRINT_LINE("[ResourceManager] Инициализация менеджера ресурсов...");

        // Создаем VMA аллокатор
        if (!createAllocator()) {
            SAFE_ERROR("[ResourceManager] Ошибка создания VMA аллокатора");
            return false;
        }

        initialized = true;
        SAFE_PRINT_LINE("[ResourceManager] Инициализация завершена успешно");

        // Выводим статистику памяти
        auto stats = getMemoryStatistics();
        SAFE_PRINT_LINE("[ResourceManager] Доступная память: "
                        + SAFE_TO_STRING(stats.total.statistics.allocationBytes / (1024 * 1024))
                        + " MB");

        return true;

    } catch (const std::exception& e) {
        SAFE_ERROR("[ResourceManager] Ошибка инициализации: " + std::string(e.what()));
        return false;
    }
}

void ResourceManager::shutdown() {
    if (!initialized) {
        return;
    }

    SAFE_PRINT_LINE("[ResourceManager] Завершение работы менеджера ресурсов...");

    // Освобождаем все оставшиеся ресурсы (заглушка)
    SAFE_PRINT_LINE("[ResourceManager] Освобождение " + SAFE_TO_STRING(bufferAllocations.size())
                    + " буферов (заглушка)");
    bufferAllocations.clear();

    SAFE_PRINT_LINE("[ResourceManager] Освобождение " + SAFE_TO_STRING(imageAllocations.size())
                    + " изображений (заглушка)");
    imageAllocations.clear();

    // Уничтожаем аллокатор (заглушка)
    if (allocator != VK_NULL_HANDLE) {
        SAFE_PRINT_LINE("[ResourceManager] Уничтожение VMA аллокатора (заглушка)");
        allocator = VK_NULL_HANDLE;
    }

    initialized = false;
    SAFE_PRINT_LINE("[ResourceManager] Завершение работы завершено");
}

vk::Buffer ResourceManager::allocateBuffer(size_t size,
                                           vk::BufferUsageFlags usage,
                                           VmaMemoryUsage memoryUsage) {
    if (!initialized) {
        throw std::runtime_error("ResourceManager не инициализирован");
    }

    SAFE_PRINT_LINE("[ResourceManager] Создание буфера размером " + SAFE_TO_STRING(size / 1024)
                    + " KB (заглушка)");

    // TODO: Реальное создание буфера через VMA на следующих этапах
    // Пока возвращаем фиктивный буфер
    VkBuffer buffer =
        reinterpret_cast<VkBuffer>(static_cast<uintptr_t>(bufferAllocations.size() + 1));

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

    SAFE_PRINT_LINE("[ResourceManager] Создание изображения " + SAFE_TO_STRING(data.width) + "x"
                    + SAFE_TO_STRING(data.height) + " (заглушка)");

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
        SAFE_PRINT_LINE("[ResourceManager] Буфер освобожден (заглушка)");
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
        SAFE_PRINT_LINE("[ResourceManager] Изображение освобождено (заглушка)");
    }
}

void* ResourceManager::mapBuffer(vk::Buffer buffer) {
    if (!initialized) {
        return nullptr;
    }

    SAFE_PRINT_LINE("[ResourceManager] Отображение буфера (заглушка)");

    // TODO: Реальное отображение памяти через VMA на следующих этапах
    // Пока возвращаем фиктивный указатель
    return reinterpret_cast<void*>(0x1000);
}

void ResourceManager::unmapBuffer(vk::Buffer buffer) {
    if (!initialized) {
        return;
    }

    SAFE_PRINT_LINE("[ResourceManager] Отмена отображения буфера (заглушка)");

    // TODO: Реальная отмена отображения через VMA на следующих этапах
}

void ResourceManager::updateBuffer(vk::Buffer buffer,
                                   const void* data,
                                   size_t size,
                                   size_t offset) {
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
        SAFE_PRINT_LINE("[ResourceManager] Создание external memory для CUDA interop");

        // Создаем external memory из CUDA ресурса
        vk::MemoryAllocateInfo allocInfo{};
        allocInfo.allocationSize = cudaRes.size;

        // Найдем подходящий тип памяти для external memory
        allocInfo.memoryTypeIndex = findMemoryType(0xFFFFFFFF,  // Любой тип памяти
                                                   vk::MemoryPropertyFlagBits::eDeviceLocal);

        // Добавляем информацию для импорта external memory (только Windows)
#ifdef _WIN32
        // TODO: Требуется расширение VK_KHR_external_memory_win32
        // vk::ImportMemoryWin32HandleInfoKHR importInfo{};
        // importInfo.handleType = vk::ExternalMemoryHandleTypeFlagBits::eOpaqueWin32;
        // importInfo.handle = cudaRes.handle; // TODO: Получить handle из CUDA
        // allocInfo.pNext = &importInfo;
#endif

        auto memory = device.allocateMemory(allocInfo);

        SAFE_PRINT_LINE("[ResourceManager] External memory создана успешно");
        return memory;

    } catch (const std::exception& e) {
        SAFE_ERROR("[ResourceManager] Ошибка создания external memory: " + std::string(e.what()));
        return vk::DeviceMemory{};
    }
}

vk::Buffer ResourceManager::createSharedBuffer(size_t size, vk::BufferUsageFlags usage) {
    try {
        SAFE_PRINT_LINE("[ResourceManager] Создание shared буфера размером " + SAFE_TO_STRING(size)
                        + " байт");

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
        allocInfo.memoryTypeIndex = findMemoryType(memRequirements.memoryTypeBits,
                                                   vk::MemoryPropertyFlagBits::eDeviceLocal);

        vk::ExportMemoryAllocateInfo exportInfo{};
        exportInfo.handleTypes = vk::ExternalMemoryHandleTypeFlagBits::eOpaqueWin32;
        allocInfo.pNext = &exportInfo;

        auto memory = device.allocateMemory(allocInfo);

        // Привязываем буфер к памяти
        device.bindBufferMemory(buffer, memory, 0);

        // Сохраняем информацию об аллокации
        AllocationInfo allocInfo_cache{};
        allocInfo_cache.allocation = VK_NULL_HANDLE;  // Для external memory VMA не используется
        allocInfo_cache.size = size;
        bufferAllocations[buffer] = allocInfo_cache;

        SAFE_PRINT_LINE("[ResourceManager] Shared буфер создан успешно");
        return buffer;

    } catch (const std::exception& e) {
        SAFE_ERROR("[ResourceManager] Ошибка создания shared буфера: " + std::string(e.what()));
        return vk::Buffer{};
    }
}

cudaExternalMemory_t ResourceManager::exportMemoryToCUDA(vk::DeviceMemory memory) {
    try {
        SAFE_PRINT_LINE("[ResourceManager] Экспорт Vulkan памяти в CUDA");

        // Получаем Windows handle от Vulkan memory (только Windows)
#ifdef _WIN32
        // TODO: Требуется расширение VK_KHR_external_memory_win32
        // vk::MemoryGetWin32HandleInfoKHR handleInfo{};
        // handleInfo.memory = memory;
        // handleInfo.handleType = vk::ExternalMemoryHandleTypeFlagBits::eOpaqueWin32;
        //
        // HANDLE handle = device.getMemoryWin32HandleKHR(handleInfo);

        SAFE_PRINT_LINE(
            "[ResourceManager] Windows CUDA interop не поддерживается (требуется "
            "VK_KHR_external_memory_win32)");
        return cudaExternalMemory_t{};
#else
        SAFE_PRINT_LINE("[ResourceManager] CUDA interop не поддерживается на данной платформе");
        return cudaExternalMemory_t{};
#endif

        // Остальной код временно закомментирован
        //
        // // Создаем CUDA external memory
        // cudaExternalMemoryHandleDesc memHandleDesc{};
        // memHandleDesc.type = cudaExternalMemoryHandleTypeOpaqueWin32;
        // memHandleDesc.handle.win32.handle = handle;
        // memHandleDesc.size = 0; // Размер будет определен автоматически
        //
        // cudaExternalMemory_t extMem;
        // cudaError_t result = cudaImportExternalMemory(&extMem, &memHandleDesc);
        //
        // if (result != cudaSuccess) {
        //     SAFE_ERROR( "[ResourceManager] Ошибка импорта памяти в CUDA: "
        //               << cudaGetErrorString(result));
        //     return nullptr;
        // }
        //
        // SAFE_PRINT_LINE("[ResourceManager] Память успешно экспортирована в CUDA");
        // return extMem;

    } catch (const std::exception& e) {
        SAFE_ERROR("[ResourceManager] Ошибка экспорта памяти в CUDA: " + std::string(e.what()));
        return nullptr;
    }
}
#endif

VmaTotalStatistics ResourceManager::getMemoryStatistics() const {
    VmaTotalStatistics stats{};

    SAFE_PRINT_LINE("[ResourceManager] Получение статистики памяти (заглушка)");

    // TODO: Реальная статистика через VMA на следующих этапах
    // Пока возвращаем пустую статистику

    return stats;
}

// Приватные методы

bool ResourceManager::createAllocator() {
    SAFE_PRINT_LINE("[ResourceManager] VMA аллокатор создан (заглушка для этапа 2.1)");

    // TODO: Реальная инициализация VMA на следующих этапах
    // Пока просто возвращаем успех
    allocator = reinterpret_cast<VmaAllocator>(0x1);  // Фиктивный указатель
    return true;
}

uint32_t ResourceManager::findMemoryType(uint32_t typeFilter, vk::MemoryPropertyFlags properties) {
    auto memProperties = physicalDevice.getMemoryProperties();

    for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++) {
        if ((typeFilter & (1 << i))
            && (memProperties.memoryTypes[i].propertyFlags & properties) == properties) {
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

}  // namespace HyperEngine::Vulkan
