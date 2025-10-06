#pragma once

#include <cstdint>
#include <string>
#include <vector>

namespace spectraforge {
namespace rendering {

/**
 * @brief Сохранить RGBA8 буфер кадра в PNG-файл.
 * @param filename Путь к выходному PNG.
 * @param width Ширина изображения в пикселях.
 * @param height Высота изображения в пикселях.
 * @param rgba8 Пиксели в формате RGBA8, длиной width*height*4.
 * @return true при успехе, иначе false.
 */
bool save_rgba8_to_png(const std::string& filename,
                       uint32_t width,
                       uint32_t height,
                       const std::vector<uint8_t>& rgba8);

} // namespace rendering
} // namespace spectraforge


