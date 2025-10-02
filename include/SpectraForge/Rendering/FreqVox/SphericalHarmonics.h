/**
 * @file SphericalHarmonics.h
 * @brief Spherical Harmonics utilities для FreqVox (L=0,1,2)
 * 
 * Implements SH encoding/evaluation согласно FreqVox Math.md раздел 1:
 * 
 * L_i(ω) = Σ_{ℓ=0}^{2} Σ_{m=-ℓ}^{ℓ} c_{i,ℓ,m} Y_ℓ^m(ω)
 * 
 * Где:
 * - ℓ=0: 1 коэффициент (DC component)
 * - ℓ=1: 3 коэффициента (linear terms)
 * - ℓ=2: 5 коэффициентов (quadratic terms)
 * Итого: 9 коэффициентов
 */

#pragma once

#include "SpectraForge/Rendering/FreqVox/FreqVoxTypes.h"
#include "SpectraForge/Math/Vector3.h"
#include <array>
#include <cmath>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

namespace SpectraForge::Rendering::FreqVox {

/**
 * @brief SH basis function constants (precomputed normalization factors)
 * 
 * Согласно ортонормализованным SH basis:
 * ∫ Y_ℓ^m(ω) Y_ℓ'^m'(ω) dω = δ_{ℓℓ'} δ_{mm'}
 */
struct SHBasisConstants {
    // L=0 (DC)
    static constexpr float Y00 = 0.282095f;  // 1/(2√π)
    
    // L=1 (Linear)
    static constexpr float Y1 = 0.488603f;   // √(3/(4π))
    
    // L=2 (Quadratic)
    static constexpr float Y20 = 0.315392f;  // √(5/(16π))
    static constexpr float Y21 = 1.092548f;  // √(15/(4π))
    static constexpr float Y22 = 0.546274f;  // √(15/(16π))
};

/**
 * @brief Spherical Harmonics utilities
 */
class SphericalHarmonics {
public:
    /**
     * @brief Evaluate SH basis functions для направления ω
     * @param direction Normalized direction vector (x, y, z)
     * @return 9 SH basis values [Y_0^0, Y_1^{-1}, Y_1^0, Y_1^1, ...]
     * 
     * Порядок коэффициентов:
     * [0] = Y_0^0       (DC)
     * [1] = Y_1^{-1}    (y)
     * [2] = Y_1^0       (z)
     * [3] = Y_1^1       (x)
     * [4] = Y_2^{-2}    (xy)
     * [5] = Y_2^{-1}    (yz)
     * [6] = Y_2^0       (3z²-1)
     * [7] = Y_2^1       (xz)
     * [8] = Y_2^2       (x²-y²)
     */
    static std::array<float, 9> evaluate_basis(const Math::Vector3& direction) {
        const float x = direction.x;
        const float y = direction.y;
        const float z = direction.z;
        
        const float x2 = x * x;
        const float y2 = y * y;
        const float z2 = z * z;
        
        using SHC = SHBasisConstants;
        
        return {
            // L=0
            SHC::Y00,                                    // [0] Y_0^0
            
            // L=1
            SHC::Y1 * y,                                 // [1] Y_1^{-1}
            SHC::Y1 * z,                                 // [2] Y_1^0
            SHC::Y1 * x,                                 // [3] Y_1^1
            
            // L=2
            SHC::Y21 * x * y,                            // [4] Y_2^{-2}
            SHC::Y21 * y * z,                            // [5] Y_2^{-1}
            SHC::Y20 * (3.0f * z2 - 1.0f),              // [6] Y_2^0
            SHC::Y21 * x * z,                            // [7] Y_2^1
            SHC::Y22 * (x2 - y2)                         // [8] Y_2^2
        };
    }
    
    /**
     * @brief Evaluate SH9 lighting для направления ω
     * @param sh SH9 коэффициенты (RGB channels)
     * @param direction Normalized direction
     * @return RGB color (reconstructed radiance)
     * 
     * Вычисляет: L(ω) = Σ c_ℓ^m Y_ℓ^m(ω)
     */
    static Math::Vector3 evaluate_sh9(const SH9& sh, const Math::Vector3& direction) {
        const auto basis = evaluate_basis(direction);
        
        Math::Vector3 result{0.0f, 0.0f, 0.0f};
        
        // Dot product: color = Σ coeff × basis
        for (int i = 0; i < 9; ++i) {
            result.x += sh.r[i] * basis[i];
            result.y += sh.g[i] * basis[i];
            result.z += sh.b[i] * basis[i];
        }
        
        return result;
    }
    
    /**
     * @brief Project radiance sample на SH (для encoding)
     * @param sh Output SH9 коэффициенты (accumulation)
     * @param direction Sample direction
     * @param color Radiance value (RGB)
     * @param weight Sample weight (обычно cos(θ) для Lambert)
     * 
     * Использование (Monte Carlo integration):
     * ```
     * SH9 sh = {}; // zero init
     * for (auto& sample : samples) {
     *     project_sample(sh, sample.dir, sample.color, sample.weight);
     * }
     * normalize_sh9(sh, samples.size());
     * ```
     */
    static void project_sample(SH9& sh, 
                               const Math::Vector3& direction,
                               const Math::Vector3& color,
                               float weight = 1.0f) {
        const auto basis = evaluate_basis(direction);
        
        for (int i = 0; i < 9; ++i) {
            const float basis_weighted = basis[i] * weight;
            sh.r[i] += color.x * basis_weighted;
            sh.g[i] += color.y * basis_weighted;
            sh.b[i] += color.z * basis_weighted;
        }
    }
    
    /**
     * @brief Normalize SH коэффициенты после Monte Carlo sampling
     * @param sh SH9 коэффициенты (будут изменены in-place)
     * @param sample_count Число samples
     * 
     * Формула: c_ℓ^m = (4π / N) Σ L(ω_i) Y_ℓ^m(ω_i)
     */
    static void normalize_sh9(SH9& sh, uint32_t sample_count) {
        if (sample_count == 0) return;
        
        const float normalization = (4.0f * M_PI) / static_cast<float>(sample_count);
        
        for (int i = 0; i < 9; ++i) {
            sh.r[i] *= normalization;
            sh.g[i] *= normalization;
            sh.b[i] *= normalization;
        }
    }
    
    /**
     * @brief Compute irradiance (diffuse convolution) from SH radiance
     * @param radiance_sh Input: radiance SH коэффициенты
     * @param irradiance_sh Output: irradiance SH коэффициенты
     * 
     * Для Lambert diffuse: E(n) = ∫ L(ω) max(0, n·ω) dω
     * 
     * В SH: применяем convolution kernel A_ℓ (Ramamoorthi & Hanrahan 2001):
     * - A_0 = π
     * - A_1 = 2π/3
     * - A_2 = π/4
     * 
     * Результат: E_ℓ^m = A_ℓ × L_ℓ^m
     */
    static SH9 compute_irradiance(const SH9& radiance_sh) {
        // Zonal harmonics для Lambert diffuse (Ramamoorthi & Hanrahan 2001)
        constexpr float A0 = M_PI;              // Band 0 (DC)
        constexpr float A1 = 2.0f * M_PI / 3.0f; // Band 1 (linear)
        constexpr float A2 = M_PI / 4.0f;       // Band 2 (quadratic)
        
        SH9 irradiance_sh;
        
        // L=0: 1 коэффициент
        irradiance_sh.r[0] = radiance_sh.r[0] * A0;
        irradiance_sh.g[0] = radiance_sh.g[0] * A0;
        irradiance_sh.b[0] = radiance_sh.b[0] * A0;
        
        // L=1: 3 коэффициента
        for (int i = 1; i <= 3; ++i) {
            irradiance_sh.r[i] = radiance_sh.r[i] * A1;
            irradiance_sh.g[i] = radiance_sh.g[i] * A1;
            irradiance_sh.b[i] = radiance_sh.b[i] * A1;
        }
        
        // L=2: 5 коэффициентов
        for (int i = 4; i <= 8; ++i) {
            irradiance_sh.r[i] = radiance_sh.r[i] * A2;
            irradiance_sh.g[i] = radiance_sh.g[i] * A2;
            irradiance_sh.b[i] = radiance_sh.b[i] * A2;
        }
        
        return irradiance_sh;
    }
    
    /**
     * @brief Rotate SH коэффициенты (для rotated objects)
     * @param sh Input SH коэффициенты
     * @param rotation Rotation matrix (3×3)
     * @return Rotated SH коэффициенты
     * 
     * NOTE: L=0 инвариантен к rotation, L=1 вращается как вектор,
     * L=2 требует rotation matrix для quadratic terms.
     * 
     * Упрощенная реализация (full rotation требует Wigner D-matrices).
     */
    static SH9 rotate_sh9(const SH9& sh, const float rotation[3][3]) {
        SH9 rotated = sh;
        
        // L=0: инвариантен (DC component не меняется)
        // rotated.r[0] = sh.r[0]; (уже скопировано)
        
        // L=1: вращается как вектор [y, z, x]
        for (int c = 0; c < 3; ++c) {
            float* channel = (c == 0) ? rotated.r.data() : 
                           (c == 1) ? rotated.g.data() : rotated.b.data();
            const float* src = (c == 0) ? sh.r.data() : 
                              (c == 1) ? sh.g.data() : sh.b.data();
            
            // L=1: [Y_1^{-1}, Y_1^0, Y_1^1] = [y, z, x]
            const float y = src[1];
            const float z = src[2];
            const float x = src[3];
            
            channel[1] = rotation[1][0] * x + rotation[1][1] * y + rotation[1][2] * z;
            channel[2] = rotation[2][0] * x + rotation[2][1] * y + rotation[2][2] * z;
            channel[3] = rotation[0][0] * x + rotation[0][1] * y + rotation[0][2] * z;
        }
        
        // L=2: требует полную rotation (опускаем для простоты - нужны Wigner D-matrices)
        // TODO: реализовать полную rotation для L=2 если необходимо
        
        return rotated;
    }
};

} // namespace SpectraForge::Rendering::FreqVox

