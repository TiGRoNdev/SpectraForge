/**
 * @file FoveatedSelector.h
 * @brief Фовеированная выборка и LOD для FreqVox
 */

#pragma once

#include <vector>
#include <memory>
#include "HyperEngine/Rendering/FreqVox/FreqVoxTypes.h"

namespace HyperEngine::Rendering::FreqVox {

/**
 * @brief Параметры фовеированной выборки
 */
struct FoveatedParams {
    float sigma_deg = 5.0f;  ///< параметр гаусса в градусах
    float fine_radius_m = 10.0f;
    float fine_cone_deg = 10.0f;
};

/**
 * @brief Селектор активных вокселей с учетом фовеации и LOD
 */
class FoveatedSelector {
  public:
    FoveatedSelector() = default;
    ~FoveatedSelector() = default;

    /**
     * @brief Выбирает подмножество вокселей и рассчитывает эффективный вес
     */
    void select(const std::vector<Voxel>& all_voxels,
                const FoveatedParams& params,
                std::vector<Voxel>& out_selected,
                float& out_effective_count) const;
};

}  // namespace HyperEngine::Rendering::FreqVox


