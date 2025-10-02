/**
 * @file FoveatedSelectorTest.cpp
 * @brief Unit tests для Foveated Selector (Math.md раздел 3)
 */

#include <gtest/gtest.h>
#include "SpectraForge/Rendering/FreqVox/FoveatedSelector.h"
#include <cmath>

using namespace SpectraForge::Rendering::FreqVox;
using namespace SpectraForge::Math;

constexpr float EPSILON = 1e-5f;

/**
 * @brief Helper: Create voxel at position
 */
static Voxel make_voxel(float x, float y, float z) {
    Voxel v;
    v.position = Vector3{x, y, z};
    return v;
}

/**
 * @brief Test: Center voxel (φ=0) должен иметь weight=1
 */
TEST(FoveatedSelectorTest, CenterVoxelFullWeight) {
    FoveatedSelector selector;
    
    // Voxel в центре gaze
    std::vector<Voxel> voxels = {
        make_voxel(0.0f, 0.0f, 10.0f)  // На линии взгляда Z+
    };
    
    FoveatedParams params;
    params.gaze_center = Vector3{0.0f, 0.0f, 1.0f};  // Z+
    params.eye_position = Vector3{0.0f, 0.0f, 0.0f}; // Origin
    params.foveal_sigma_deg = 5.0f;
    params.weight_threshold = 0.0f; // No culling для теста
    
    std::vector<Voxel> selected;
    float effective_count = 0.0f;
    
    selector.select(voxels, params, selected, effective_count);
    
    // Center voxel: φ = 0 → w = exp(0) = 1
    EXPECT_EQ(selected.size(), 1);
    EXPECT_NEAR(effective_count, 1.0f, EPSILON);
}

/**
 * @brief Test: Voxel на угле σ должен иметь weight ≈ exp(-0.5)
 */
TEST(FoveatedSelectorTest, SigmaAngleWeight) {
    FoveatedSelector selector;
    
    const float sigma_deg = 5.0f;
    const float sigma_rad = sigma_deg * M_PI / 180.0f;
    
    // Voxel на угле σ от центра
    // Используем сферические координаты: (r, θ, φ)
    // θ = sigma_rad от Z+
    const float r = 10.0f;
    const float x = r * std::sin(sigma_rad);
    const float z = r * std::cos(sigma_rad);
    
    std::vector<Voxel> voxels = {
        make_voxel(x, 0.0f, z)
    };
    
    FoveatedParams params;
    params.gaze_center = Vector3{0.0f, 0.0f, 1.0f};
    params.eye_position = Vector3{0.0f, 0.0f, 0.0f};
    params.foveal_sigma_deg = sigma_deg;
    params.weight_threshold = 0.0f;
    
    std::vector<Voxel> selected;
    float effective_count = 0.0f;
    
    selector.select(voxels, params, selected, effective_count);
    
    // φ = σ → w = exp(-σ² / 2σ²) = exp(-0.5) ≈ 0.6065
    const float expected_weight = std::exp(-0.5f);
    EXPECT_EQ(selected.size(), 1);
    EXPECT_NEAR(effective_count, expected_weight, 0.01f);
}

/**
 * @brief Test: Peripheral voxel (большой φ) должен иметь малый weight
 */
TEST(FoveatedSelectorTest, PeripheralVoxelLowWeight) {
    FoveatedSelector selector;
    
    // Voxel далеко от центра (φ = 45°)
    const float angle_rad = 45.0f * M_PI / 180.0f;
    const float r = 10.0f;
    const float x = r * std::sin(angle_rad);
    const float z = r * std::cos(angle_rad);
    
    std::vector<Voxel> voxels = {
        make_voxel(x, 0.0f, z)
    };
    
    FoveatedParams params;
    params.gaze_center = Vector3{0.0f, 0.0f, 1.0f};
    params.eye_position = Vector3{0.0f, 0.0f, 0.0f};
    params.foveal_sigma_deg = 5.0f;
    params.weight_threshold = 0.0f;
    
    std::vector<Voxel> selected;
    float effective_count = 0.0f;
    
    selector.select(voxels, params, selected, effective_count);
    
    // φ = 45° >> σ = 5° → w должен быть очень мал
    EXPECT_EQ(selected.size(), 1);
    EXPECT_LT(effective_count, 0.001f); // Очень малый вес
}

/**
 * @brief Test: Weight threshold culling
 */
TEST(FoveatedSelectorTest, WeightThresholdCulling) {
    FoveatedSelector selector;
    
    // 2 voxels: один в центре, один далеко
    std::vector<Voxel> voxels = {
        make_voxel(0.0f, 0.0f, 10.0f),   // Center: w ≈ 1
        make_voxel(10.0f, 0.0f, 0.0f)    // Side: w << 1 (φ ≈ 90°)
    };
    
    FoveatedParams params;
    params.gaze_center = Vector3{0.0f, 0.0f, 1.0f};
    params.eye_position = Vector3{0.0f, 0.0f, 0.0f};
    params.foveal_sigma_deg = 5.0f;
    params.weight_threshold = 0.1f; // Cull weights < 0.1
    
    std::vector<Voxel> selected;
    float effective_count = 0.0f;
    
    selector.select(voxels, params, selected, effective_count);
    
    // Должен остаться только центральный voxel
    EXPECT_EQ(selected.size(), 1);
    EXPECT_NEAR(effective_count, 1.0f, 0.1f);
}

/**
 * @brief Test: Far plane culling
 */
TEST(FoveatedSelectorTest, FarPlaneCulling) {
    FoveatedSelector selector;
    
    // 2 voxels: один близко, один за far plane
    std::vector<Voxel> voxels = {
        make_voxel(0.0f, 0.0f, 10.0f),   // Near
        make_voxel(0.0f, 0.0f, 200.0f)   // Far (>100)
    };
    
    FoveatedParams params;
    params.gaze_center = Vector3{0.0f, 0.0f, 1.0f};
    params.eye_position = Vector3{0.0f, 0.0f, 0.0f};
    params.foveal_sigma_deg = 5.0f;
    params.far_plane = 100.0f;
    params.weight_threshold = 0.0f;
    
    std::vector<Voxel> selected;
    float effective_count = 0.0f;
    
    selector.select(voxels, params, selected, effective_count);
    
    // Должен остаться только ближний voxel
    EXPECT_EQ(selected.size(), 1);
    EXPECT_FLOAT_EQ(selected[0].position.z, 10.0f);
}

/**
 * @brief Test: Gaze direction rotation
 */
TEST(FoveatedSelectorTest, GazeDirectionRotation) {
    FoveatedSelector selector;
    
    // Voxel на X+
    std::vector<Voxel> voxels = {
        make_voxel(10.0f, 0.0f, 0.0f)
    };
    
    FoveatedParams params;
    params.gaze_center = Vector3{1.0f, 0.0f, 0.0f};  // Смотрим на X+
    params.eye_position = Vector3{0.0f, 0.0f, 0.0f};
    params.foveal_sigma_deg = 5.0f;
    params.weight_threshold = 0.0f;
    
    std::vector<Voxel> selected;
    float effective_count = 0.0f;
    
    selector.select(voxels, params, selected, effective_count);
    
    // Voxel на линии взгляда → w ≈ 1
    EXPECT_EQ(selected.size(), 1);
    EXPECT_NEAR(effective_count, 1.0f, EPSILON);
}

/**
 * @brief Test: Symmetric weighting (Y+ и Y- на одном углу)
 */
TEST(FoveatedSelectorTest, SymmetricWeighting) {
    FoveatedSelector selector;
    
    const float angle_rad = 10.0f * M_PI / 180.0f;
    const float r = 10.0f;
    const float y = r * std::sin(angle_rad);
    const float z = r * std::cos(angle_rad);
    
    // 2 voxels: симметрично относительно gaze
    std::vector<Voxel> voxels = {
        make_voxel(0.0f, +y, z),  // Y+
        make_voxel(0.0f, -y, z)   // Y-
    };
    
    FoveatedParams params;
    params.gaze_center = Vector3{0.0f, 0.0f, 1.0f};
    params.eye_position = Vector3{0.0f, 0.0f, 0.0f};
    params.foveal_sigma_deg = 5.0f;
    params.weight_threshold = 0.0f;
    
    std::vector<Voxel> selected;
    float effective_count = 0.0f;
    
    selector.select(voxels, params, selected, effective_count);
    
    // Оба voxel на одинаковом угле → одинаковые веса
    EXPECT_EQ(selected.size(), 2);
    
    // effective_count = 2 * w(10°)
    const float expected_single_weight = std::exp(
        -(angle_rad * angle_rad) / (2.0f * std::pow(5.0f * M_PI / 180.0f, 2.0f))
    );
    EXPECT_NEAR(effective_count, 2.0f * expected_single_weight, 0.01f);
}

/**
 * @brief Test: Empty voxel list
 */
TEST(FoveatedSelectorTest, EmptyVoxelList) {
    FoveatedSelector selector;
    
    std::vector<Voxel> voxels; // Empty
    
    FoveatedParams params;
    params.gaze_center = Vector3{0.0f, 0.0f, 1.0f};
    params.eye_position = Vector3{0.0f, 0.0f, 0.0f};
    
    std::vector<Voxel> selected;
    float effective_count = 0.0f;
    
    selector.select(voxels, params, selected, effective_count);
    
    EXPECT_EQ(selected.size(), 0);
    EXPECT_FLOAT_EQ(effective_count, 0.0f);
}

/**
 * @brief Test: Gaussian formula verification
 * 
 * Проверяем точность формулы w = exp(-φ² / 2σ²)
 */
TEST(FoveatedSelectorTest, GaussianFormulaVerification) {
    FoveatedSelector selector;
    
    const float sigma_deg = 5.0f;
    const float sigma_rad = sigma_deg * M_PI / 180.0f;
    
    // Test various angles
    const std::vector<float> test_angles_deg = {0.0f, 2.5f, 5.0f, 10.0f, 20.0f};
    
    for (float angle_deg : test_angles_deg) {
        const float angle_rad = angle_deg * M_PI / 180.0f;
        const float r = 10.0f;
        const float x = r * std::sin(angle_rad);
        const float z = r * std::cos(angle_rad);
        
        std::vector<Voxel> voxels = {make_voxel(x, 0.0f, z)};
        
        FoveatedParams params;
        params.gaze_center = Vector3{0.0f, 0.0f, 1.0f};
        params.eye_position = Vector3{0.0f, 0.0f, 0.0f};
        params.foveal_sigma_deg = sigma_deg;
        params.weight_threshold = 0.0f;
        
        std::vector<Voxel> selected;
        float effective_count = 0.0f;
        
        selector.select(voxels, params, selected, effective_count);
        
        // Expected weight
        const float expected_weight = std::exp(
            -(angle_rad * angle_rad) / (2.0f * sigma_rad * sigma_rad)
        );
        
        EXPECT_NEAR(effective_count, expected_weight, 0.001f)
            << "Failed for angle " << angle_deg << "°";
    }
}

