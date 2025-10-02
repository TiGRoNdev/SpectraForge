/**
 * @file NeuralUpscaling.cpp
 */

#include "HyperEngine/Rendering/FreqVox/NeuralUpscaling.h"
#include "HyperEngine/Core/SafeConsole.h"
#include <algorithm>
#include <cmath>

namespace HyperEngine::Rendering::FreqVox {

NeuralUpscaler::NeuralUpscaler() {
    SAFE_PRINT_LINE("[NeuralUpscaler] Создан");
}

NeuralUpscaler::~NeuralUpscaler() {
    shutdown();
}

bool NeuralUpscaler::initialize(UpscalerType type,
                                uint32_t input_width,
                                uint32_t input_height,
                                const NeuralUpscalingParams& params) {
    if (initialized_) {
        SAFE_WARNING("[NeuralUpscaler] Уже инициализирован");
        return true;
    }

    input_width_ = input_width;
    input_height_ = input_height;
    params_ = params;

    // Вычисляем выходное разрешение
    output_width_ = static_cast<uint32_t>(input_width_ * params_.upscaleFactor);
    output_height_ = static_cast<uint32_t>(input_height_ * params_.upscaleFactor);

    // Выбор апскейлера
    if (type == UpscalerType::Auto) {
        active_type_ = selectBestUpscaler();
    } else {
        active_type_ = type;
    }

    // TODO: Инициализация конкретного бэкенда (DLSS/FSR2/TinyCNN)
    // Сейчас - заглушка с билинейной интерполяцией

    initialized_ = true;
    SAFE_PRINT_LINE("[NeuralUpscaler] Инициализирован");
    return true;
}

void NeuralUpscaler::shutdown() {
    if (!initialized_) return;

    // TODO: Очистка ресурсов DLSS/FSR2/TinyCNN

    initialized_ = false;
    SAFE_PRINT_LINE("[NeuralUpscaler] Завершение работы");
}

bool NeuralUpscaler::upscale(const std::vector<float>& input_image,
                             std::vector<float>& output_image,
                             const std::vector<float>* motion_vectors) {
    if (!initialized_) {
        SAFE_ERROR("[NeuralUpscaler] Не инициализирован");
        return false;
    }

    (void)motion_vectors; // TODO: использовать для DLSS/FSR2

    size_t expected_input_size = input_width_ * input_height_ * 3; // RGB
    if (input_image.size() != expected_input_size) {
        SAFE_ERROR("[NeuralUpscaler] Некорректный размер входного изображения");
        return false;
    }

    // Выбор алгоритма апскейлинга
    switch (active_type_) {
        case UpscalerType::DLSS:
            // TODO: DLSS через Streamline
            SAFE_WARNING("[NeuralUpscaler] DLSS не реализован, fallback на Bilinear");
            return upscaleBilinear(input_image, output_image);

        case UpscalerType::FSR2:
            // TODO: FSR2 интеграция
            SAFE_WARNING("[NeuralUpscaler] FSR2 не реализован, fallback на Bilinear");
            return upscaleBilinear(input_image, output_image);

        case UpscalerType::TinyCNN:
            // TODO: Tiny CNN inference
            SAFE_WARNING("[NeuralUpscaler] TinyCNN не реализован, fallback на Bilinear");
            return upscaleBilinear(input_image, output_image);

        case UpscalerType::Bilinear:
        case UpscalerType::Auto:
        default:
            return upscaleBilinear(input_image, output_image);
    }
}

std::string NeuralUpscaler::getActiveName() const {
    switch (active_type_) {
        case UpscalerType::DLSS:     return "DLSS";
        case UpscalerType::FSR2:     return "FSR2";
        case UpscalerType::TinyCNN:  return "TinyCNN";
        case UpscalerType::Bilinear: return "Bilinear";
        case UpscalerType::Auto:     return "Auto";
        default:                     return "Unknown";
    }
}

UpscalerType NeuralUpscaler::selectBestUpscaler() const {
    // TODO: Детект оборудования и выбор лучшего апскейлера
    // - NVIDIA RTX: DLSS
    // - AMD: FSR2
    // - Fallback: TinyCNN или Bilinear

    SAFE_PRINT_LINE("[NeuralUpscaler] Автовыбор: Bilinear (fallback)");
    return UpscalerType::Bilinear;
}

bool NeuralUpscaler::upscaleBilinear(const std::vector<float>& input, 
                                     std::vector<float>& output) {
    output.resize(output_width_ * output_height_ * 3);

    float x_ratio = static_cast<float>(input_width_) / output_width_;
    float y_ratio = static_cast<float>(input_height_) / output_height_;

    for (uint32_t y = 0; y < output_height_; ++y) {
        for (uint32_t x = 0; x < output_width_; ++x) {
            float src_x = x * x_ratio;
            float src_y = y * y_ratio;

            uint32_t x1 = static_cast<uint32_t>(src_x);
            uint32_t y1 = static_cast<uint32_t>(src_y);
            uint32_t x2 = std::min(x1 + 1, input_width_ - 1);
            uint32_t y2 = std::min(y1 + 1, input_height_ - 1);

            float fx = src_x - x1;
            float fy = src_y - y1;

            // Билинейная интерполяция для каждого RGB канала
            for (int c = 0; c < 3; ++c) {
                float p11 = input[(y1 * input_width_ + x1) * 3 + c];
                float p12 = input[(y1 * input_width_ + x2) * 3 + c];
                float p21 = input[(y2 * input_width_ + x1) * 3 + c];
                float p22 = input[(y2 * input_width_ + x2) * 3 + c];

                float p1 = p11 * (1.0f - fx) + p12 * fx;
                float p2 = p21 * (1.0f - fx) + p22 * fx;
                float p = p1 * (1.0f - fy) + p2 * fy;

                output[(y * output_width_ + x) * 3 + c] = p;
            }
        }
    }

    return true;
}

} // namespace HyperEngine::Rendering::FreqVox

