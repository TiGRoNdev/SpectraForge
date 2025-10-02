/**
 * @file FoveatedSelector.cpp
 */

#include "HyperEngine/Rendering/FreqVox/FoveatedSelector.h"
#include <cmath>

namespace HyperEngine::Rendering::FreqVox {

static inline float deg_to_rad(float deg) { return deg * 3.14159265358979323846f / 180.0f; }

void FoveatedSelector::select(const std::vector<Voxel>& all_voxels,
                              const FoveatedParams& /*params*/,
                              std::vector<Voxel>& out_selected,
                              float& out_effective_count) const {
    out_selected.clear();
    out_selected.reserve(all_voxels.size());
    out_effective_count = 0.0f;

    // Заглушка: на данном этапе возвращаем всё как есть (без камеры)
    // Реальная реализация будет зависеть от камеры/взгляда и расстояний
    for (const auto& v : all_voxels) {
        out_selected.push_back(v);
        out_effective_count += 1.0f;  // вес=1 (дальше добавим w_i)
    }
}

}  // namespace HyperEngine::Rendering::FreqVox


