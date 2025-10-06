#include "SpectraForge/rendering/FrameOutput.h"

#include <cstdio>
#include <png.h>

namespace spectraforge {
namespace rendering {

bool save_rgba8_to_png(const std::string& filename,
                       uint32_t width,
                       uint32_t height,
                       const std::vector<uint8_t>& rgba8) {
    if (rgba8.size() < static_cast<size_t>(width) * static_cast<size_t>(height) * 4) {
        return false;
    }

    FILE* fp = fopen(filename.c_str(), "wb");
    if (!fp) {
        return false;
    }

    png_structp png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, nullptr, nullptr, nullptr);
    if (!png_ptr) {
        fclose(fp);
        return false;
    }

    png_infop info_ptr = png_create_info_struct(png_ptr);
    if (!info_ptr) {
        png_destroy_write_struct(&png_ptr, nullptr);
        fclose(fp);
        return false;
    }

    if (setjmp(png_jmpbuf(png_ptr))) {
        png_destroy_write_struct(&png_ptr, &info_ptr);
        fclose(fp);
        return false;
    }

    png_init_io(png_ptr, fp);
    png_set_IHDR(png_ptr, info_ptr, width, height, 8, PNG_COLOR_TYPE_RGBA, PNG_INTERLACE_NONE,
                 PNG_COMPRESSION_TYPE_DEFAULT, PNG_FILTER_TYPE_DEFAULT);
    png_write_info(png_ptr, info_ptr);

    std::vector<png_bytep> row_ptrs(height);
    for (uint32_t y = 0; y < height; ++y) {
        row_ptrs[y] = (png_bytep)(&rgba8[static_cast<size_t>(y) * width * 4]);
    }
    png_write_image(png_ptr, row_ptrs.data());
    png_write_end(png_ptr, nullptr);
    png_destroy_write_struct(&png_ptr, &info_ptr);
    fclose(fp);
    return true;
}

} // namespace rendering
} // namespace spectraforge


