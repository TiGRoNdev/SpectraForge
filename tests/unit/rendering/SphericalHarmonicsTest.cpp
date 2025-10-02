/**
 * @file SphericalHarmonicsTest.cpp
 * @brief Unit tests для Spherical Harmonics (L=0,1,2)
 */

#include <gtest/gtest.h>
#include "SpectraForge/Rendering/FreqVox/SphericalHarmonics.h"
#include <cmath>

using namespace SpectraForge::Rendering::FreqVox;
using namespace SpectraForge::Math;

// Tolerance для floating-point сравнений
constexpr float EPSILON = 1e-5f;

/**
 * @brief Test: SH basis orthonormality
 * 
 * Проверяем: ∫ Y_ℓ^m(ω) Y_ℓ'^m'(ω) dω = δ_{ℓℓ'} δ_{mm'}
 * 
 * Используем Monte Carlo integration с uniform sphere sampling.
 */
TEST(SphericalHarmonicsTest, BasisOrthonormality) {
    const int NUM_SAMPLES = 10000;
    const float TOLERANCE = 0.05f; // 5% error tolerance для MC integration
    
    // Accumulate dot products
    float dot_products[9][9] = {};
    
    for (int s = 0; s < NUM_SAMPLES; ++s) {
        // Uniform sphere sampling
        const float u1 = static_cast<float>(rand()) / RAND_MAX;
        const float u2 = static_cast<float>(rand()) / RAND_MAX;
        
        const float theta = std::acos(2.0f * u1 - 1.0f);
        const float phi = 2.0f * M_PI * u2;
        
        const Vector3 dir{
            std::sin(theta) * std::cos(phi),
            std::sin(theta) * std::sin(phi),
            std::cos(theta)
        };
        
        const auto basis = SphericalHarmonics::evaluate_basis(dir);
        
        // Accumulate outer product
        for (int i = 0; i < 9; ++i) {
            for (int j = 0; j < 9; ++j) {
                dot_products[i][j] += basis[i] * basis[j];
            }
        }
    }
    
    // Normalize: (4π / N) Σ f(ω)
    const float normalization = (4.0f * M_PI) / NUM_SAMPLES;
    
    for (int i = 0; i < 9; ++i) {
        for (int j = 0; j < 9; ++j) {
            dot_products[i][j] *= normalization;
            
            // Expected: δ_ij (Kronecker delta)
            const float expected = (i == j) ? 1.0f : 0.0f;
            EXPECT_NEAR(dot_products[i][j], expected, TOLERANCE)
                << "Orthonormality failed for basis " << i << " and " << j;
        }
    }
}

/**
 * @brief Test: SH reconstruction (project + evaluate)
 * 
 * Проверяем что projection → evaluation восстанавливает исходный signal.
 */
TEST(SphericalHarmonicsTest, ProjectionReconstruction) {
    // Создаем простой constant radiance
    const Vector3 constant_color{0.5f, 0.3f, 0.2f};
    
    SH9 sh = {};
    const int NUM_SAMPLES = 1000;
    
    // Project constant radiance
    for (int s = 0; s < NUM_SAMPLES; ++s) {
        // Uniform sphere sampling
        const float u1 = static_cast<float>(rand()) / RAND_MAX;
        const float u2 = static_cast<float>(rand()) / RAND_MAX;
        
        const float theta = std::acos(2.0f * u1 - 1.0f);
        const float phi = 2.0f * M_PI * u2;
        
        const Vector3 dir{
            std::sin(theta) * std::cos(phi),
            std::sin(theta) * std::sin(phi),
            std::cos(theta)
        };
        
        SphericalHarmonics::project_sample(sh, dir, constant_color);
    }
    
    SphericalHarmonics::normalize_sh9(sh, NUM_SAMPLES);
    
    // Evaluate в нескольких точках
    for (int test = 0; test < 10; ++test) {
        const float u1 = static_cast<float>(rand()) / RAND_MAX;
        const float u2 = static_cast<float>(rand()) / RAND_MAX;
        
        const float theta = std::acos(2.0f * u1 - 1.0f);
        const float phi = 2.0f * M_PI * u2;
        
        const Vector3 dir{
            std::sin(theta) * std::cos(phi),
            std::sin(theta) * std::sin(phi),
            std::cos(theta)
        };
        
        const Vector3 reconstructed = SphericalHarmonics::evaluate_sh9(sh, dir);
        
        // Для constant функции: только DC component ненулевой
        // Остальные компоненты должны быть ~0
        EXPECT_NEAR(reconstructed.x, constant_color.x, 0.1f);
        EXPECT_NEAR(reconstructed.y, constant_color.y, 0.1f);
        EXPECT_NEAR(reconstructed.z, constant_color.z, 0.1f);
    }
    
    // Для constant функции: c_0^0 = 4π * L_0 * Y_0^0
    // После normalization: (4π/N) * N * L_0 * Y_0^0 = 4π * L_0 * Y_0^0
    const float expected_coeff = 4.0f * M_PI * SHBasisConstants::Y00;
    EXPECT_NEAR(sh.r[0] / constant_color.x, expected_coeff, 0.1f);
    EXPECT_NEAR(sh.g[0] / constant_color.y, expected_coeff, 0.1f);
    EXPECT_NEAR(sh.b[0] / constant_color.z, expected_coeff, 0.1f);
}

/**
 * @brief Test: Irradiance computation (diffuse convolution)
 * 
 * Проверяем что irradiance = convolution с Lambert kernel.
 */
TEST(SphericalHarmonicsTest, IrradianceComputation) {
    // Простой directional light: L(ω) = max(0, ω·dir)
    const Vector3 light_dir{0.0f, 0.0f, 1.0f}; // Z-up
    const Vector3 light_color{1.0f, 1.0f, 1.0f};
    
    SH9 radiance_sh = {};
    const int NUM_SAMPLES = 5000;
    
    // Project directional light
    for (int s = 0; s < NUM_SAMPLES; ++s) {
        const float u1 = static_cast<float>(rand()) / RAND_MAX;
        const float u2 = static_cast<float>(rand()) / RAND_MAX;
        
        const float theta = std::acos(2.0f * u1 - 1.0f);
        const float phi = 2.0f * M_PI * u2;
        
        const Vector3 dir{
            std::sin(theta) * std::cos(phi),
            std::sin(theta) * std::sin(phi),
            std::cos(theta)
        };
        
        // Dot product
        const float dot = dir.x * light_dir.x + dir.y * light_dir.y + dir.z * light_dir.z;
        const float intensity = std::max(0.0f, dot);
        
        const Vector3 color = light_color * intensity;
        SphericalHarmonics::project_sample(radiance_sh, dir, color);
    }
    
    SphericalHarmonics::normalize_sh9(radiance_sh, NUM_SAMPLES);
    
    // Compute irradiance
    const SH9 irradiance_sh = SphericalHarmonics::compute_irradiance(radiance_sh);
    
    // Evaluate irradiance в направлении света (должно быть максимально)
    const Vector3 irrad_at_light = SphericalHarmonics::evaluate_sh9(irradiance_sh, light_dir);
    
    // Для directional light вверх, irradiance в Z+ должна быть ~π (для unit intensity)
    EXPECT_GT(irrad_at_light.x, 1.0f); // Должна быть положительной
    EXPECT_GT(irrad_at_light.y, 1.0f);
    EXPECT_GT(irrad_at_light.z, 1.0f);
    
    // Evaluate irradiance в противоположном направлении (должно быть ~0)
    const Vector3 opposite_dir{0.0f, 0.0f, -1.0f};
    const Vector3 irrad_opposite = SphericalHarmonics::evaluate_sh9(irradiance_sh, opposite_dir);
    
    EXPECT_LT(irrad_opposite.x, 0.5f); // Должна быть близка к 0
    EXPECT_LT(irrad_opposite.y, 0.5f);
    EXPECT_LT(irrad_opposite.z, 0.5f);
}

/**
 * @brief Test: SH basis L=0 (DC component)
 * 
 * Y_0^0 = константа (0.282095)
 */
TEST(SphericalHarmonicsTest, L0_DCComponent) {
    const Vector3 dir1{1.0f, 0.0f, 0.0f};
    const Vector3 dir2{0.0f, 1.0f, 0.0f};
    const Vector3 dir3{0.0f, 0.0f, 1.0f};
    
    const auto basis1 = SphericalHarmonics::evaluate_basis(dir1);
    const auto basis2 = SphericalHarmonics::evaluate_basis(dir2);
    const auto basis3 = SphericalHarmonics::evaluate_basis(dir3);
    
    // Y_0^0 должен быть константой
    EXPECT_FLOAT_EQ(basis1[0], SHBasisConstants::Y00);
    EXPECT_FLOAT_EQ(basis2[0], SHBasisConstants::Y00);
    EXPECT_FLOAT_EQ(basis3[0], SHBasisConstants::Y00);
}

/**
 * @brief Test: SH basis L=1 (linear terms)
 * 
 * Y_1^{-1} = 0.488603 * y
 * Y_1^0    = 0.488603 * z
 * Y_1^1    = 0.488603 * x
 */
TEST(SphericalHarmonicsTest, L1_LinearTerms) {
    const Vector3 x_axis{1.0f, 0.0f, 0.0f};
    const Vector3 y_axis{0.0f, 1.0f, 0.0f};
    const Vector3 z_axis{0.0f, 0.0f, 1.0f};
    
    const auto basis_x = SphericalHarmonics::evaluate_basis(x_axis);
    const auto basis_y = SphericalHarmonics::evaluate_basis(y_axis);
    const auto basis_z = SphericalHarmonics::evaluate_basis(z_axis);
    
    // X axis: только Y_1^1 ненулевой
    EXPECT_NEAR(basis_x[3], SHBasisConstants::Y1, EPSILON); // Y_1^1 = Y1 * x
    EXPECT_NEAR(basis_x[1], 0.0f, EPSILON);                 // Y_1^{-1} = Y1 * y
    EXPECT_NEAR(basis_x[2], 0.0f, EPSILON);                 // Y_1^0 = Y1 * z
    
    // Y axis: только Y_1^{-1} ненулевой
    EXPECT_NEAR(basis_y[1], SHBasisConstants::Y1, EPSILON);
    EXPECT_NEAR(basis_y[2], 0.0f, EPSILON);
    EXPECT_NEAR(basis_y[3], 0.0f, EPSILON);
    
    // Z axis: только Y_1^0 ненулевой
    EXPECT_NEAR(basis_z[2], SHBasisConstants::Y1, EPSILON);
    EXPECT_NEAR(basis_z[1], 0.0f, EPSILON);
    EXPECT_NEAR(basis_z[3], 0.0f, EPSILON);
}

/**
 * @brief Test: SH L=2 quadratic term Y_2^0
 * 
 * Y_2^0 = 0.315392 * (3z² - 1)
 */
TEST(SphericalHarmonicsTest, L2_QuadraticZ) {
    const Vector3 z_axis{0.0f, 0.0f, 1.0f};
    const auto basis = SphericalHarmonics::evaluate_basis(z_axis);
    
    // Y_2^0 для z=1: 0.315392 * (3*1 - 1) = 0.630784
    const float expected = SHBasisConstants::Y20 * (3.0f * 1.0f - 1.0f);
    EXPECT_NEAR(basis[6], expected, EPSILON);
}

