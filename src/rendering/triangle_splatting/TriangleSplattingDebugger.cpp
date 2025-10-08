#include <SpectraForge/Rendering/RenderPass/TriangleSplatting/TriangleSplattingDebugger.h>
#include <SpectraForge/Core/Console.h>
#include <fstream>
#include <vector>
#include <cstring>

using SpectraForge::Core::Console;

namespace spectraforge {
namespace rendering {

void TriangleSplattingDebugger::setDebugMode(uint32_t mode) {
    debugMode_ = mode;
    
    std::string modeStr;
    switch (mode) {
        case 0: modeStr = "Normal"; break;
        case 1: modeStr = "SDF Visualization"; break;
        case 2: modeStr = "Barycentric Coordinates"; break;
        case 3: modeStr = "Depth Buffer"; break;
        default: modeStr = "Unknown"; break;
    }
    
    Console::info("Debug mode set to: " + modeStr);
}

void TriangleSplattingDebugger::enableWireframe(bool enable) {
    wireframeEnabled_ = enable;
    Console::info(std::string("Wireframe mode ") + (enable ? "enabled" : "disabled"));
}

void TriangleSplattingDebugger::setBackgroundColor(const glm::vec4& color) {
    backgroundColor_ = color;
    Console::info("Background color set");
}

bool TriangleSplattingDebugger::saveFrameToPPM(const std::string& filename,
                                               vk::Image image,
                                               vk::Device device,
                                               VmaAllocator allocator,
                                               vk::CommandPool commandPool,
                                               vk::Queue queue,
                                               uint32_t width,
                                               uint32_t height) {
    // Создаем staging buffer
    const vk::DeviceSize bufferSize = static_cast<vk::DeviceSize>(width) * height * 4; // RGBA8
    
    VkBufferCreateInfo bufferInfo = {};
    bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferInfo.size = bufferSize;
    bufferInfo.usage = VK_BUFFER_USAGE_TRANSFER_DST_BIT;
    bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    
    VmaAllocationCreateInfo allocInfo = {};
    allocInfo.usage = VMA_MEMORY_USAGE_GPU_TO_CPU;
    
    VkBuffer vkBuffer;
    VmaAllocation allocation;
    VkResult result = vmaCreateBuffer(allocator, &bufferInfo, &allocInfo, &vkBuffer, &allocation, nullptr);
    
    if (result != VK_SUCCESS) {
        Console::error("Failed to create staging buffer for screenshot");
        return false;
    }
    
    vk::Buffer stagingBuffer(vkBuffer);
    
    // Копируем image в buffer
    if (!copyImageToBuffer(image, stagingBuffer, device, commandPool, queue, width, height)) {
        vmaDestroyBuffer(allocator, vkBuffer, allocation);
        return false;
    }
    
    // Читаем данные из buffer
    void* data;
    vmaMapMemory(allocator, allocation, &data);
    
    // Сохраняем в PPM формат
    std::ofstream file(filename, std::ios::binary);
    if (!file.is_open()) {
        Console::error("Failed to open file for writing: " + filename);
        vmaUnmapMemory(allocator, allocation);
        vmaDestroyBuffer(allocator, vkBuffer, allocation);
        return false;
    }
    
    // PPM header
    file << "P6\n" << width << " " << height << "\n255\n";
    
    // Write pixel data (convert RGBA to RGB)
    const uint8_t* pixels = static_cast<const uint8_t*>(data);
    for (uint32_t i = 0; i < width * height; ++i) {
        file.write(reinterpret_cast<const char*>(&pixels[i * 4]), 3); // RGB only
    }
    
    file.close();
    
    vmaUnmapMemory(allocator, allocation);
    vmaDestroyBuffer(allocator, vkBuffer, allocation);
    
    Console::info("Screenshot saved to: " + filename);
    
    return true;
}

bool TriangleSplattingDebugger::saveFrameToPNG(const std::string& filename,
                                               vk::Image image,
                                               vk::Device device,
                                               VmaAllocator allocator,
                                               vk::CommandPool commandPool,
                                               vk::Queue queue,
                                               uint32_t width,
                                               uint32_t height) {
    // PNG saving requires libpng
    // Для упрощения, используем PPM вместо PNG
    Console::info("PNG saving not implemented, using PPM instead");
    return saveFrameToPPM(filename + ".ppm", image, device, allocator, commandPool, queue, width, height);
}

bool TriangleSplattingDebugger::copyImageToBuffer(vk::Image image,
                                                  vk::Buffer buffer,
                                                  vk::Device device,
                                                  vk::CommandPool commandPool,
                                                  vk::Queue queue,
                                                  uint32_t width,
                                                  uint32_t height) {
    // Allocate command buffer
    vk::CommandBufferAllocateInfo allocInfo;
    allocInfo.level = vk::CommandBufferLevel::ePrimary;
    allocInfo.commandPool = commandPool;
    allocInfo.commandBufferCount = 1;
    
    vk::CommandBuffer cmd = device.allocateCommandBuffers(allocInfo)[0];
    
    vk::CommandBufferBeginInfo beginInfo;
    beginInfo.flags = vk::CommandBufferUsageFlagBits::eOneTimeSubmit;
    
    cmd.begin(beginInfo);
    
    // Transition image layout to transfer source
    vk::ImageMemoryBarrier barrier;
    barrier.oldLayout = vk::ImageLayout::eGeneral;
    barrier.newLayout = vk::ImageLayout::eTransferSrcOptimal;
    barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.image = image;
    barrier.subresourceRange.aspectMask = vk::ImageAspectFlagBits::eColor;
    barrier.subresourceRange.baseMipLevel = 0;
    barrier.subresourceRange.levelCount = 1;
    barrier.subresourceRange.baseArrayLayer = 0;
    barrier.subresourceRange.layerCount = 1;
    barrier.srcAccessMask = vk::AccessFlagBits::eShaderWrite;
    barrier.dstAccessMask = vk::AccessFlagBits::eTransferRead;
    
    cmd.pipelineBarrier(
        vk::PipelineStageFlagBits::eComputeShader,
        vk::PipelineStageFlagBits::eTransfer,
        vk::DependencyFlags(),
        0, nullptr,
        0, nullptr,
        1, &barrier
    );
    
    // Copy image to buffer
    vk::BufferImageCopy region;
    region.bufferOffset = 0;
    region.bufferRowLength = 0;
    region.bufferImageHeight = 0;
    region.imageSubresource.aspectMask = vk::ImageAspectFlagBits::eColor;
    region.imageSubresource.mipLevel = 0;
    region.imageSubresource.baseArrayLayer = 0;
    region.imageSubresource.layerCount = 1;
    region.imageOffset = vk::Offset3D{0, 0, 0};
    region.imageExtent = vk::Extent3D{width, height, 1};
    
    cmd.copyImageToBuffer(image, vk::ImageLayout::eTransferSrcOptimal, buffer, 1, &region);
    
    // Transition back to general layout
    barrier.oldLayout = vk::ImageLayout::eTransferSrcOptimal;
    barrier.newLayout = vk::ImageLayout::eGeneral;
    barrier.srcAccessMask = vk::AccessFlagBits::eTransferRead;
    barrier.dstAccessMask = vk::AccessFlagBits::eShaderWrite;
    
    cmd.pipelineBarrier(
        vk::PipelineStageFlagBits::eTransfer,
        vk::PipelineStageFlagBits::eComputeShader,
        vk::DependencyFlags(),
        0, nullptr,
        0, nullptr,
        1, &barrier
    );
    
    cmd.end();
    
    // Submit and wait
    vk::SubmitInfo submitInfo;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &cmd;
    
    queue.submit(1, &submitInfo, vk::Fence());
    queue.waitIdle();
    
    device.freeCommandBuffers(commandPool, 1, &cmd);
    
    return true;
}

} // namespace rendering
} // namespace spectraforge

