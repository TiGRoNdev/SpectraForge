/**
 * @file FoveatedSelector.cpp
 * @brief Foveated voxel selection согласно Math.md раздел 3
 * 
 * Implements Gaussian weighting:
 * w_i = exp(-φ_i² / 2σ²)
 * 
 * где φ_i - angular distance от gaze center,
 * σ - foveal radius (стандартное отклонение).
 */

#include "SpectraForge/Rendering/FreqVox/FoveatedSelector.h"
#include "SpectraForge/Core/SafeConsole.h"
#include <cmath>
#include <algorithm>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

namespace SpectraForge::Rendering::FreqVox {

/**
 * @brief Convert degrees to radians
 */
static inline float deg_to_rad(float deg) { 
    return deg * static_cast<float>(M_PI) / 180.0f; 
}

/**
 * @brief Compute angular distance между двумя normalized directions
 * @param dir1 First direction (normalized)
 * @param dir2 Second direction (normalized)
 * @return Angular distance в радианах [0, π]
 * 
 * Использует: φ = acos(dir1 · dir2)
 */
static inline float angular_distance(const Math::Vector3& dir1, const Math::Vector3& dir2) {
    // Dot product (clamped для numerical stability)
    const float dot = std::clamp(
        dir1.x * dir2.x + dir1.y * dir2.y + dir1.z * dir2.z,
        -1.0f,
        1.0f
    );
    
    // Angular distance
    return std::acos(dot);
}

/**
 * @brief Compute Gaussian foveation weight
 * @param phi_rad Angular distance в радианах
 * @param sigma_rad σ в радианах
 * @return Weight w = exp(-φ² / 2σ²) ∈ [0, 1]
 */
static inline float gaussian_weight(float phi_rad, float sigma_rad) {
    const float sigma_sq = sigma_rad * sigma_rad;
    const float phi_sq = phi_rad * phi_rad;
    return std::exp(-phi_sq / (2.0f * sigma_sq));
}

void FoveatedSelector::select(const std::vector<Voxel>& all_voxels,
                              const FoveatedParams& params,
                              std::vector<Voxel>& out_selected,
                              float& out_effective_count) const {
    out_selected.clear();
    out_selected.reserve(all_voxels.size());
    out_effective_count = 0.0f;

    // Early exit если нет вокселей
    if (all_voxels.empty()) {
        return;
    }

    // Normalize gaze direction (на случай если пользователь передал ненормализованный)
    const float gaze_length = std::sqrt(
        params.gaze_center.x * params.gaze_center.x +
        params.gaze_center.y * params.gaze_center.y +
        params.gaze_center.z * params.gaze_center.z
    );
    
    if (gaze_length < 1e-6f) {
        SAFE_WARNING("[FoveatedSelector] Invalid gaze direction (near-zero), using default Z+");
        // Fallback: все веса = 1
        for (const auto& v : all_voxels) {
            out_selected.push_back(v);
            out_effective_count += 1.0f;
        }
        return;
    }
    
    const Math::Vector3 gaze_normalized{
        params.gaze_center.x / gaze_length,
        params.gaze_center.y / gaze_length,
        params.gaze_center.z / gaze_length
    };
    
    // Convert σ to radians
    const float sigma_rad = deg_to_rad(params.foveal_sigma_deg);
    
    // Process each voxel
    for (const auto& voxel : all_voxels) {
        // Compute direction from eye to voxel
        Math::Vector3 to_voxel{
            voxel.position.x - params.eye_position.x,
            voxel.position.y - params.eye_position.y,
            voxel.position.z - params.eye_position.z
        };
        
        // Distance check (far plane culling)
        const float dist_sq = to_voxel.x * to_voxel.x + 
                             to_voxel.y * to_voxel.y + 
                             to_voxel.z * to_voxel.z;
        
        if (dist_sq > params.far_plane * params.far_plane) {
            continue; // Cull voxel за far plane
        }
        
        if (dist_sq < 1e-6f) {
            // Voxel совпадает с eye position (degenerate case)
            continue;
        }
        
        // Normalize direction
        const float dist = std::sqrt(dist_sq);
        to_voxel.x /= dist;
        to_voxel.y /= dist;
        to_voxel.z /= dist;
        
        // Compute angular distance φ
        const float phi_rad = angular_distance(to_voxel, gaze_normalized);
        
        // Compute Gaussian weight: w = exp(-φ² / 2σ²)
        const float weight = gaussian_weight(phi_rad, sigma_rad);
        
        // Threshold optimization: skip voxels с малым весом
        if (weight < params.weight_threshold) {
            continue;
        }
        
        // Include voxel with weight
        out_selected.push_back(voxel);
        out_effective_count += weight;
    }
    
    // Debug info
    if (out_selected.size() < all_voxels.size()) {
        const float culled_percent = 100.0f * (1.0f - static_cast<float>(out_selected.size()) / all_voxels.size());
        SAFE_PRINT_LINE("[FoveatedSelector] Culled " + 
                       std::to_string(static_cast<int>(culled_percent)) + 
                       "% voxels (kept " + std::to_string(out_selected.size()) + 
                       " / " + std::to_string(all_voxels.size()) + ")");
    }
}

}  // namespace SpectraForge::Rendering::FreqVox


