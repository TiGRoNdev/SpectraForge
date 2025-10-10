#pragma once

#include <vulkan/vulkan.hpp>
#include <vk_mem_alloc.h>
#include <glm/glm.hpp>
#include <vector>

#include "SpectraForge/Rendering/Utils/DynamicBuffer.h"

namespace spectraforge {
namespace rendering {

/**
 * @brief Данные инстанса для GPU (SSBO): модельная матрица и цвет RGBA.
 */
struct InstanceDataGPU {
    alignas(16) float model[16];
    alignas(16) float color[4];
};

/**
 * @brief Push-constants для камеры (uView/uProj), соответствуют шейдеру.
 */
struct CameraPushConstants {
    glm::mat4 uView;
    glm::mat4 uProj;
};

/**
 * @brief Проход инстансинга: управляет SSBO инстансов и дескрипторами.
 *        Пайплайн создаётся и привязывается внешним кодом.
 */
class InstancedMeshPass {
public:
    /** @brief Инициализация ресурсов: layout/set и SSBO. */
    bool initialize(vk::Device device, VmaAllocator allocator);

    /** @brief Освобождение ресурсов. */
    void cleanup();

    /** @brief Обновление буфера инстансов (CPU→GPU). */
    void upload_instances(const std::vector<InstanceDataGPU>& cpuInstances);

    /** @brief Гарантировать вместимость SSBO не меньше bytes. */
    bool ensure_capacity(size_t bytes);

    /** @brief Синхронизировать хост-записи перед шейдерным чтением (barrier). */
    void barrier_host_writes(vk::CommandBuffer cmd) const;

    /**
     * @brief Привязка дескриптора и пуш-констант перед draw.
     * @param cmd Командный буфер
     * @param pc Push-constants камеры
     * @param pipelineLayout Layout графического пайплайна (содержит set=0 и push-constants)
     * @param setIndex Индекс сета в layout (обычно 0)
     */
    void bind_and_push(vk::CommandBuffer cmd,
                       const CameraPushConstants& pc,
                       vk::PipelineLayout pipelineLayout,
                       uint32_t setIndex = 0) const;

    // Getters
    vk::DescriptorSetLayout get_descriptor_set_layout() const { return setLayout_; }
    vk::DescriptorSet get_descriptor_set() const { return descriptorSet_; }
    vk::Buffer get_instance_buffer() const;
    size_t get_capacity_bytes() const { return instanceBufferCapacity_; }

private:
    bool create_descriptors();
    bool create_instance_buffer(size_t capacityBytes);

private:
    vk::Device device_;

    vk::DescriptorSetLayout setLayout_;
    vk::DescriptorPool descriptorPool_;
    vk::DescriptorSet descriptorSet_;

    utils::DynamicBuffer instanceBuffer_;
    size_t instanceBufferCapacity_ = 0;
};

} // namespace rendering
} // namespace spectraforge


