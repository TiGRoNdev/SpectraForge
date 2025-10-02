/**
 * @file FrequencyShading.cpp
 * @brief Реализация frequency-domain shading pipeline
 * 
 * Согласно FreqVox Renderer Math.md раздел 2:
 * "Compute shading as 2D convolution in frequency domain:
 *  S̃_i[u,v] = L̃_i[u,v] ⊙ M̃[u,v]"
 */

#include "SpectraForge/Rendering/FreqVox/FrequencyShading.h"
#include "SpectraForge/Core/SafeConsole.h"
#include <algorithm>

namespace SpectraForge::Rendering::FreqVox {

bool FrequencyShadingPipeline::shade_image(
    std::vector<float>& image_buffer,
    uint32_t width,
    uint32_t height,
    const std::vector<float>& material_freq) {
    
    if (!backend_) {
        SAFE_ERROR("[FrequencyShading] Backend не инициализирован");
        return false;
    }

    const uint32_t blockSize = config_.blockSize;
    const uint32_t blockElements = blockSize * blockSize;

    // Проверка размера material
    if (material_freq.size() != blockElements) {
        SAFE_ERROR("[FrequencyShading] Некорректный размер material_freq для shade_image");
        return false;
    }

    // Проверка размера изображения (должно быть width * height * 3 для RGB)
    const size_t expectedSize = width * height * 3;
    if (image_buffer.size() != expectedSize) {
        SAFE_ERROR("[FrequencyShading] Некорректный размер image_buffer: получено " +
                   SpectraForge::Core::SAFE_TO_STRING(image_buffer.size()) +
                   ", ожидалось " + SpectraForge::Core::SAFE_TO_STRING(expectedSize) +
                   " (width=" + SpectraForge::Core::SAFE_TO_STRING(width) +
                   ", height=" + SpectraForge::Core::SAFE_TO_STRING(height) + ")");
        return false;
    }

    // Вычисляем количество блоков (с учетом padding для неполных блоков)
    const uint32_t blocksX = (width + blockSize - 1) / blockSize;
    const uint32_t blocksY = (height + blockSize - 1) / blockSize;

    SAFE_PRINT_LINE("[FrequencyShading] Обработка изображения " +
                    SpectraForge::Core::SAFE_TO_STRING(width) + "x" +
                    SpectraForge::Core::SAFE_TO_STRING(height) +
                    " = " + SpectraForge::Core::SAFE_TO_STRING(blocksX * blocksY) + " блоков");

    // Обрабатываем каждый канал (R, G, B) отдельно
    for (uint32_t channel = 0; channel < 3; ++channel) {
        // Обрабатываем блоки построчно
        for (uint32_t by = 0; by < blocksY; ++by) {
            for (uint32_t bx = 0; bx < blocksX; ++bx) {
                // Извлекаем блок 8×8 из изображения
                std::vector<float> block(blockElements, 0.0f);
                
                for (uint32_t py = 0; py < blockSize; ++py) {
                    for (uint32_t px = 0; px < blockSize; ++px) {
                        uint32_t globalX = bx * blockSize + px;
                        uint32_t globalY = by * blockSize + py;
                        
                        // Проверяем границы (для padding)
                        if (globalX < width && globalY < height) {
                            size_t imageIdx = (globalY * width + globalX) * 3 + channel;
                            size_t blockIdx = py * blockSize + px;
                            block[blockIdx] = image_buffer[imageIdx];
                        }
                        // Если за границами, оставляем 0.0f (padding)
                    }
                }

                // Применяем DCT → multiply → IDCT
                if (!backend_->transform_forward(block)) {
                    SAFE_ERROR("[FrequencyShading] Ошибка forward DCT в shade_image");
                    return false;
                }

                // Frequency-domain convolution: L̃ ⊙ M̃
                for (uint32_t i = 0; i < blockElements; ++i) {
                    block[i] *= material_freq[i];
                }

                if (!backend_->transform_inverse(block)) {
                    SAFE_ERROR("[FrequencyShading] Ошибка inverse DCT в shade_image");
                    return false;
                }

                // Записываем результат обратно в изображение
                for (uint32_t py = 0; py < blockSize; ++py) {
                    for (uint32_t px = 0; px < blockSize; ++px) {
                        uint32_t globalX = bx * blockSize + px;
                        uint32_t globalY = by * blockSize + py;
                        
                        if (globalX < width && globalY < height) {
                            size_t imageIdx = (globalY * width + globalX) * 3 + channel;
                            size_t blockIdx = py * blockSize + px;
                            image_buffer[imageIdx] = block[blockIdx];
                        }
                    }
                }
            }
        }
    }

    return true;
}

bool FrequencyShadingPipeline::shade_blocks(
    std::vector<float>& lighting_blocks,
    const std::vector<float>& material_freq) {
    
    if (!backend_) {
        SAFE_ERROR("[FrequencyShading] Backend не инициализирован");
        return false;
    }

    const uint32_t blockSize = config_.blockSize;
    const uint32_t blockElements = blockSize * blockSize;
    const uint32_t batchCount = config_.batchCount;

    // Проверка размеров
    if (lighting_blocks.size() != batchCount * blockElements) {
        SAFE_ERROR("[FrequencyShading] Некорректный размер lighting_blocks: "
                   "получено " + SpectraForge::Core::SAFE_TO_STRING(lighting_blocks.size()) +
                   ", ожидалось " + SpectraForge::Core::SAFE_TO_STRING(batchCount * blockElements) +
                   " (batchCount=" + SpectraForge::Core::SAFE_TO_STRING(batchCount) +
                   ", blockElements=" + SpectraForge::Core::SAFE_TO_STRING(blockElements) + ")");
        return false;
    }

    if (material_freq.size() != blockElements) {
        SAFE_ERROR("[FrequencyShading] Некорректный размер material_freq");
        return false;
    }

    // Шаг 1: Forward DCT на lighting → L̃
    // Преобразуем lighting из spatial domain в frequency domain
    if (!backend_->transform_forward(lighting_blocks)) {
        SAFE_ERROR("[FrequencyShading] Ошибка forward DCT");
        return false;
    }

    // Шаг 2: Frequency-domain convolution L̃ ⊙ M̃
    // Element-wise multiplication для каждого блока в batch
    for (uint32_t batch = 0; batch < batchCount; ++batch) {
        // Получаем указатель на текущий блок
        float* blockStart = lighting_blocks.data() + batch * blockElements;
        
        // L̃[batch] ⊙ M̃
        for (uint32_t i = 0; i < blockElements; ++i) {
            blockStart[i] *= material_freq[i];
        }
    }

    // Шаг 3: Inverse DCT → S (shaded result в spatial domain)
    if (!backend_->transform_inverse(lighting_blocks)) {
        SAFE_ERROR("[FrequencyShading] Ошибка inverse DCT");
        return false;
    }

    return true;
}

bool FrequencyShadingPipeline::precompute_material(
    const std::vector<float>& material_spatial,
    std::vector<float>& material_freq) {
    
    if (!backend_) {
        SAFE_ERROR("[FrequencyShading] Backend не инициализирован");
        return false;
    }

    const uint32_t blockSize = config_.blockSize;
    const uint32_t blockElements = blockSize * blockSize;

    // Проверка размера входных данных
    if (material_spatial.size() != blockElements) {
        SAFE_ERROR("[FrequencyShading] Некорректный размер material_spatial");
        return false;
    }

    // Копируем материал (т.к. DCT работает in-place)
    material_freq = material_spatial;

    // Forward DCT: M[p,q] → M̃[u,v]
    // Создаем временный вектор для одного блока
    // (backend ожидает batch format, но у нас только один материал)
    std::vector<float> temp_batch(blockElements);
    std::copy(material_spatial.begin(), material_spatial.end(), temp_batch.begin());

    // Выполняем DCT
    if (!backend_->transform_forward(temp_batch)) {
        SAFE_ERROR("[FrequencyShading] Ошибка precompute DCT");
        return false;
    }

    // Копируем результат
    material_freq = std::move(temp_batch);

    SAFE_PRINT_LINE("[FrequencyShading] Material precomputed в частотной области");
    return true;
}

bool FrequencyShadingPipeline::elementwise_multiply(
    std::vector<float>& a,
    const std::vector<float>& b) {
    
    if (a.size() != b.size()) {
        SAFE_ERROR("[FrequencyShading] Несоответствие размеров для element-wise multiply");
        return false;
    }

    // a ← a ⊙ b
    for (size_t i = 0; i < a.size(); ++i) {
        a[i] *= b[i];
    }

    return true;
}

} // namespace SpectraForge::Rendering::FreqVox

