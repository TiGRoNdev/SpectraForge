/**
 * @file TemporalReprojection.cpp
 */

#include "HyperEngine/Rendering/FreqVox/TemporalReprojection.h"
#include "HyperEngine/Core/SafeConsole.h"
#include <algorithm>
#include <cmath>

namespace HyperEngine::Rendering::FreqVox {

TemporalReprojection::TemporalReprojection() {
    SAFE_PRINT_LINE("[TemporalReprojection] Создан");
}

TemporalReprojection::~TemporalReprojection() {
    shutdown();
}

bool TemporalReprojection::initialize(uint32_t width, uint32_t height, 
                                      const TemporalReprojectionParams& params) {
    if (initialized_) {
        SAFE_WARNING("[TemporalReprojection] Уже инициализирован");
        return true;
    }

    width_ = width;
    height_ = height;
    params_ = params;

    size_t buffer_size = width * height;
    previous_frame_.resize(buffer_size * 3, 0.0f); // RGB
    previous_depth_.resize(buffer_size, 0.0f);

    initialized_ = true;
    hasHistory_ = false;

    SAFE_PRINT_LINE("[TemporalReprojection] Инициализирован");
    return true;
}

void TemporalReprojection::shutdown() {
    if (!initialized_) return;

    previous_frame_.clear();
    previous_depth_.clear();
    initialized_ = false;
    hasHistory_ = false;

    SAFE_PRINT_LINE("[TemporalReprojection] Завершение работы");
}

bool TemporalReprojection::reproject(const std::vector<float>& current_frame,
                                     const std::vector<Math::Vector3>& motion_vectors,
                                     const std::vector<float>& depth_buffer,
                                     std::vector<float>& output_frame) {
    if (!initialized_) {
        SAFE_ERROR("[TemporalReprojection] Не инициализирован");
        return false;
    }

    size_t pixel_count = width_ * height_;
    if (current_frame.size() != pixel_count * 3 ||
        motion_vectors.size() != pixel_count ||
        depth_buffer.size() != pixel_count) {
        SAFE_ERROR("[TemporalReprojection] Несоответствие размеров буферов");
        return false;
    }

    output_frame.resize(pixel_count * 3);

    // Если нет истории - просто копируем текущий кадр
    if (!hasHistory_) {
        output_frame = current_frame;
        previous_frame_ = current_frame;
        previous_depth_ = depth_buffer;
        hasHistory_ = true;
        return true;
    }

    // Темпоральная репроекция с motion vectors
    for (size_t y = 0; y < height_; ++y) {
        for (size_t x = 0; x < width_; ++x) {
            size_t idx = y * width_ + x;
            const Math::Vector3& mv = motion_vectors[idx];

            // Вычисляем координаты предыдущего кадра
            int prev_x = static_cast<int>(x) - static_cast<int>(mv.x);
            int prev_y = static_cast<int>(y) - static_cast<int>(mv.y);

            // Проверка границ
            if (prev_x >= 0 && prev_x < static_cast<int>(width_) &&
                prev_y >= 0 && prev_y < static_cast<int>(height_)) {
                
                size_t prev_idx = prev_y * width_ + prev_x;
                float depth_diff = std::abs(depth_buffer[idx] - previous_depth_[prev_idx]);

                // Валидация motion vector
                if (validateMotionVector(mv, depth_buffer[idx], previous_depth_[prev_idx])) {
                    float blend = calculateBlendFactor(mv, depth_diff);

                    // Смешивание RGB каналов
                    for (int c = 0; c < 3; ++c) {
                        float curr = current_frame[idx * 3 + c];
                        float prev = previous_frame_[prev_idx * 3 + c];
                        output_frame[idx * 3 + c] = blend * curr + (1.0f - blend) * prev;
                    }
                    continue;
                }
            }

            // Fallback: используем только текущий кадр
            for (int c = 0; c < 3; ++c) {
                output_frame[idx * 3 + c] = current_frame[idx * 3 + c];
            }
        }
    }

    // Обновляем историю
    previous_frame_ = output_frame;
    previous_depth_ = depth_buffer;

    return true;
}

void TemporalReprojection::resetHistory() {
    hasHistory_ = false;
    std::fill(previous_frame_.begin(), previous_frame_.end(), 0.0f);
    std::fill(previous_depth_.begin(), previous_depth_.end(), 0.0f);
    SAFE_PRINT_LINE("[TemporalReprojection] История сброшена");
}

void TemporalReprojection::updateParams(const TemporalReprojectionParams& params) {
    params_ = params;
    SAFE_PRINT_LINE("[TemporalReprojection] Параметры обновлены");
}

bool TemporalReprojection::validateMotionVector(const Math::Vector3& mv, 
                                                float current_depth, 
                                                float prev_depth) const {
    // Проверка величины motion vector
    float mv_magnitude = std::sqrt(mv.x * mv.x + mv.y * mv.y);
    if (mv_magnitude > params_.motionVectorThreshold) {
        return false; // Слишком большое движение
    }

    // Проверка изменения глубины
    float depth_diff = std::abs(current_depth - prev_depth);
    if (depth_diff > params_.depthChangeThreshold) {
        return false; // Слишком большое изменение глубины
    }

    return true;
}

float TemporalReprojection::calculateBlendFactor(const Math::Vector3& mv, 
                                                 float depth_diff) const {
    // Адаптивный blend factor на основе motion vector и depth diff
    float mv_factor = std::sqrt(mv.x * mv.x + mv.y * mv.y) / params_.motionVectorThreshold;
    float depth_factor = depth_diff / params_.depthChangeThreshold;
    
    float combined_factor = std::max(mv_factor, depth_factor);
    combined_factor = std::clamp(combined_factor, 0.0f, 1.0f);

    // Интерполяция между базовым blend factor и 1.0 (только текущий кадр)
    return params_.blendFactor + (1.0f - params_.blendFactor) * combined_factor;
}

} // namespace HyperEngine::Rendering::FreqVox

