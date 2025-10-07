#pragma once

#include <cstdint>
#include <memory>
#include <string>

namespace SpectraForge::Rendering {

/**
 * @brief Узкий read-only доступ к ключевым объектам графического контекста
 *
 * Позволяет стратегиям рендеринга получать необходимую информацию и
 * дескрипторы, не раскрывая полный конкретный backend API. Это облегчает
 * мокирование в тестах и снижает связность с backend (например, Vulkan).
 */
class IRenderContextView {
  public:
    virtual ~IRenderContextView() = default;

    /**
     * @brief Получить идентификатор/имя текущего графического backend
     * @return Строка с именем backend (например, "Vulkan")
     */
    virtual std::string backend_name() const = 0;

    /**
     * @brief Получить текущие размеры swapchain или целевого фреймбуфера
     * @param out_width Сюда возвращается ширина в пикселях
     * @param out_height Сюда возвращается высота в пикселях
     */
    virtual void get_framebuffer_size(uint32_t& out_width, uint32_t& out_height) const = 0;

    /**
     * @brief Количество кадров в полёте (frames-in-flight)
     */
    virtual uint32_t frames_in_flight() const = 0;

    /**
     * @brief Текущий индекс кадра (ring buffer index)
     */
    virtual uint32_t current_frame_index() const = 0;
};

}  // namespace SpectraForge::Rendering


