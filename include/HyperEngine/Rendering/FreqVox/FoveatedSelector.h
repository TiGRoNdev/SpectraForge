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


