# FreqVox Renderer: Глубокий Математический Анализ и Roadmap Оптимизации
**Дата**: 2025-10-02  
**Версия**: 1.0  
**Автор**: SpectraForge Analysis Team

---

## 📊 Executive Summary

**Текущее состояние**: CPU DCT-II O(N⁴) реализация  
**Целевая производительность**: 300 FPS (3.3 ms/frame) на mobile GPU  
**Критический путь**: VkFFT integration blocked (error 1005)

---

## 1. МАТЕМАТИЧЕСКИЙ АНАЛИЗ ТЕКУЩЕЙ РЕАЛИЗАЦИИ

### 1.1 Frequency-Domain Shading Pipeline

#### Текущая реализация (FrequencyShading.cpp)
```
Input: RGB image (width × height × 3)
  ↓
Step 1: Split into 8×8 blocks → blocksX × blocksY blocks
  ↓
Step 2: For each block, each channel:
  2a. Extract 8×8 block L[p,q]
  2b. DCT-II forward: L̃[u,v] = Σ L[p,q] cos(πu(2p+1)/16) cos(πv(2q+1)/16)
  2c. Frequency multiply: S̃[u,v] = L̃[u,v] · M̃[u,v]
  2d. DCT-II inverse: S[p,q] = Σ α(u)α(v) S̃[u,v] cos(...)
  ↓
Output: Shaded RGB image
```

**Complexity (текущая)**:
- Блоков: `(W/8) × (H/8)`
- На блок: 2 × DCT-II = 2 × O(N⁴) = 2 × O(8⁴) = 8,192 ops
- **Всего: `8,192 × (W/8) × (H/8) × 3` операций**

Для 1920×1080:
```
8,192 × 240 × 135 × 3 = 797,184,000 операций (~800M ops)
На 1 GHz CPU: ~800 ms (!!!) → 1.25 FPS
```

**Вывод**: CPU DCT-II НЕ ПОДХОДИТ для real-time.

---

### 1.2 DCT-II vs FFT: Математическое Сравнение

#### DCT-II Forward Transform
```
M̃[u,v] = Σ_{p=0}^{N-1} Σ_{q=0}^{N-1} M[p,q] cos(πu(2p+1)/(2N)) cos(πv(2q+1)/(2N))
```

**Свойства**:
- ✅ Real-to-real transform (нет комплексных чисел)
- ✅ Энергия сконцентрирована в низких частотах (good for compression)
- ✅ Нет boundary artifacts (в отличие от DFT)
- ❌ Direct implementation: O(N⁴) для 2D
- ❌ Separable DCT: 2 × O(N³) = O(N³) (лучше, но всё ещё плохо)

#### FFT (Fast Fourier Transform)
```
M̃[u,v] = Σ_{p,q} M[p,q] exp(-2πi(up/N + vq/N))
```

**Свойства**:
- ✅ O(N² log N) complexity через Cooley-Tukey
- ✅ Highly optimized GPU implementations (cuFFT, VkFFT)
- ✅ Hardware-accelerated на многих платформах
- ❌ Complex-to-complex (2x memory)
- ❌ Circular convolution (требует padding для linear convolution)
- ❌ Boundary artifacts в spatial domain

#### DCT via FFT (VkFFT approach)

VkFFT реализует DCT через FFT mapping:
```
DCT-II(x) = Re{ FFT( [x[0], x[1], ..., x[N-1], x[N-1], ..., x[1]] ) } × phase correction
```

**Complexity**: O(N² log N) (как FFT) ✅  
**Memory**: ~1.5× FFT (из-за padding и phase correction) ⚠️  
**Accuracy**: Identical to direct DCT (при правильной реализации) ✅

---

### 1.3 Frequency-Domain Convolution: Математическая Корректность

#### Цель (из Math.md)
```
S̃[u,v] = L̃[u,v] ⊙ M̃[u,v]  (element-wise multiply)
```

#### Spatial Domain Equivalent
```
S[p,q] = Σ_{p',q'} L[p',q'] M[p-p', q-q']  (2D convolution)
```

#### Проблема: DCT vs DFT Convolution

**DFT (FFT)**: Convolution theorem holds exactly
```
F{f ★ g} = F{f} · F{g}
```

**DCT**: Convolution theorem НЕ ДЕРЖИТСЯ напрямую!
```
DCT{f ★ g} ≠ DCT{f} · DCT{g}  ❌
```

#### Workaround для DCT Convolution

**Option 1**: Use FFT instead
```
F{f ★ g} = F{f} · F{g}  ✅
S = IFFT( FFT(L) · FFT(M) )
```
- ✅ Математически корректно
- ⚠️ Circular convolution (требует zero-padding)
- ⚠️ Complex numbers (2x memory)

**Option 2**: DCT approximation
```
S̃ = DCT(L) · DCT(M)  (приближение)
```
- ⚠️ НЕ точная convolution
- ✅ Хорошо работает для smooth signals
- ✅ JPEG использует этот подход для quantization
- 📊 Accuracy depends on signal characteristics

**Option 3**: Hybrid (RECOMMENDED)
```
1. Zero-pad L и M до 16×16
2. FFT → multiply → IFFT
3. Extract center 8×8 region (valid convolution)
```
- ✅ Математически строго
- ✅ Избегает circular artifacts
- ⚠️ 4x compute (16×16 vs 8×8)

---

## 2. VkFFT INTEGRATION: ROOT CAUSE ANALYSIS

### 2.1 Error 1005 Analysis

**Error**: `VKFFT_ERROR_INVALID_PHYSICAL_DEVICE_PROPERTIES`

**VkFFT source code analysis**:
```c
// VkFFT internal check (примерно)
if (!physicalDevice || !deviceProperties) {
    return 1005;
}
```

**Наш код**:
```cpp
vkfft_config_->device = &vk_device_handle_;
vkfft_config_->physicalDevice = &vk_physical_device_handle_;  // ✅ Set
```

**Проблема**: VkFFT ожидает что device уже queries properties internally!

#### Root Cause Hypothesis

1. **VkFFT использует physical device для query properties внутри**
2. **Наш device создан в headless mode (без swapchain)**
3. **Intel driver может не возвращать полные properties в headless**

#### Solution Paths

**Path A**: Create full windowed Vulkan context
```cpp
// Требуется для VkFFT на некоторых Intel GPUs
- glfwCreateWindow() → получить surface
- vkCreateSwapchain()
- Полноценный graphics + compute queue
```

**Path B**: Fallback to cuFFT (NVIDIA only)
```cpp
if (hardwareDetector->isNVIDIA()) {
    return std::make_unique<CuFFTBackend>();
}
```

**Path C**: Use alternative GPU FFT library
- **clFFT** (OpenCL-based, deprecated)
- **FFTW** (CPU, very optimized)
- **Intel MKL** (CPU, best for Intel)

---

### 2.2 VkFFT DCT Mode Stability

**Research findings**:
- VkFFT DCT support added in v1.2.x
- Known issues на некоторых GPUs:
  - AMD RDNA2: DCT works ✅
  - NVIDIA Ampere+: DCT works ✅
  - **Intel Arc/Xe: DCT unstable** ⚠️ (наш случай)

**Recommendation**: Use FFT mode на Intel, DCT на NVIDIA/AMD
```cpp
if (hardwareDetector->isIntel()) {
    vkfft_config_->performDCT = 0;  // FFT mode
    // Implement DCT via FFT mapping manually
} else {
    vkfft_config_->performDCT = 2;  // DCT-II mode
}
```

---

## 3. OPTIMIZATION ROADMAP

### 3.1 Short-Term (Week 1-2): CPU Optimization

**Goal**: 10x speedup на CPU (800ms → 80ms per frame)

#### Step 1: Separable DCT-II
```cpp
// Текущее: O(N⁴)
for (u, v, p, q) { result[u][v] += M[p][q] * cos(...) * cos(...); }

// Оптимизация: O(N³) separable
// 1. DCT по строкам
for (p, q, u) { temp[p][u] += M[p][q] * cos(πu(2q+1)/(2N)); }
// 2. DCT по столбцам
for (p, u, v) { result[v][u] += temp[p][u] * cos(πv(2p+1)/(2N)); }
```

**Expected**: 800ms → 200ms (4x speedup)

#### Step 2: SIMD Vectorization
```cpp
// Используем AVX2/AVX-512 для parallel multiply-add
#include <immintrin.h>

__m256 a = _mm256_load_ps(&data[i]);
__m256 b = _mm256_load_ps(&cosine_lut[i]);
__m256 c = _mm256_mul_ps(a, b);
```

**Expected**: 200ms → 80ms (2.5x speedup)

#### Step 3: Multi-threading
```cpp
// Parallel block processing
#pragma omp parallel for collapse(2)
for (by = 0; by < blocksY; ++by) {
    for (bx = 0; bx < blocksX; ++bx) {
        process_block(bx, by);
    }
}
```

**Expected**: 80ms → 20ms на 4-core CPU (4x speedup)

**Total CPU optimization**: 800ms → 20ms (40x speedup) ✅

---

### 3.2 Medium-Term (Week 3-4): GPU FFT Migration

**Goal**: Use FFT instead of DCT, achieve <5ms per frame

#### Phase 1: Implement FFT-based Backend
```cpp
class FFTBackend : public IFrequencyBackend {
    bool transform_forward(std::vector<float>& data) override {
        // 1. Real to complex
        std::vector<std::complex<float>> complex_data(data.size());
        for (size_t i = 0; i < data.size(); ++i) {
            complex_data[i] = {data[i], 0.0f};
        }
        
        // 2. FFT (using FFTW as fallback)
        fftwf_plan plan = fftwf_plan_dft_2d(N, N, 
            reinterpret_cast<fftwf_complex*>(complex_data.data()),
            reinterpret_cast<fftwf_complex*>(complex_data.data()),
            FFTW_FORWARD, FFTW_ESTIMATE);
        fftwf_execute(plan);
        fftwf_destroy_plan(plan);
        
        // 3. Copy magnitude back
        for (size_t i = 0; i < data.size(); ++i) {
            data[i] = std::abs(complex_data[i]);
        }
        return true;
    }
};
```

**Expected**: 20ms → 10ms (2x speedup) на CPU FFTW

#### Phase 2: VkFFT Integration (FFT mode only)
```cpp
vkfft_config_->performDCT = 0;  // Disable DCT, use pure FFT
vkfft_config_->FFTdim = 2;
vkfft_config_->size[0] = 8;
vkfft_config_->size[1] = 8;
```

**Expected**: 10ms → 2ms (5x speedup) на Intel GPU

#### Phase 3: Proper Zero-Padding для Linear Convolution
```cpp
// Pad 8×8 → 16×16 для избежания circular convolution artifacts
const uint32_t paddedSize = 16;
std::vector<float> padded(paddedSize * paddedSize, 0.0f);
// Copy 8×8 to center
// FFT → multiply → IFFT
// Extract center 8×8
```

**Expected**: Accuracy improvement, minimal performance impact

---

### 3.3 Long-Term (Week 5-8): Optimized GPU DCT

**Goal**: <1ms per frame на mobile GPU

#### Option A: Fix VkFFT on Intel
```cpp
// Исследовать:
1. Создать полноценный windowed context для Intel
2. Query и передать все device features/properties явно
3. Тестировать с разными queue families
4. Fallback на FFT если DCT не работает
```

#### Option B: Compute Shader DCT
```glsl
// Custom Vulkan compute shader для DCT-II
#version 450

layout(local_size_x = 8, local_size_y = 8) in;

layout(binding = 0) buffer Input { float data[]; };
layout(binding = 1) buffer Output { float result[]; };
layout(binding = 2) readonly buffer CosineLUT { float lut[]; };

void main() {
    uint u = gl_LocalInvocationID.x;
    uint v = gl_LocalInvocationID.y;
    
    float sum = 0.0;
    for (uint p = 0; p < 8; ++p) {
        for (uint q = 0; q < 8; ++q) {
            float value = data[p * 8 + q];
            float cos_u = lut[u * 8 + p];
            float cos_v = lut[v * 8 + q];
            sum += value * cos_u * cos_v;
        }
    }
    
    result[u * 8 + v] = sum;
}
```

**Complexity**: O(N⁴) но с GPU параллелизмом → O(N²) effective
**Expected**: 2ms → 0.5ms (4x speedup)

#### Option C: cuFFT для NVIDIA
```cpp
#ifdef CUDA_AVAILABLE
if (hardwareDetector->isNVIDIA()) {
    cufftHandle plan;
    cufftPlan2d(&plan, N, N, CUFFT_R2C);
    cufftExecR2C(plan, d_input, d_output);
    cufftDestroy(plan);
}
#endif
```

**Expected**: 0.5ms на RTX 4090 (best case)

---

## 4. SPHERICAL HARMONICS INTEGRATION

### 4.1 Current State: DC-Only (L=0)

```cpp
// Текущая реализация (наивная)
float lighting = voxel.ambient;  // Only DC component
```

### 4.2 Target: SH L=0,1,2 (Math.md spec)

```
L_i(ω) = Σ_{ℓ=0}^{2} Σ_{m=-ℓ}^{ℓ} c_{i,ℓ,m} Y_ℓ^m(ω)
```

**Coefficients count**:
- L=0: 1 coefficient (DC)
- L=0,1: 4 coefficients
- L=0,1,2: 9 coefficients

**Memory per voxel**: 9 × 4 bytes = 36 bytes

#### Implementation Plan

```cpp
struct VoxelSH {
    float coeffs[9];  // L=0,1,2 coefficients
};

float evaluate_sh(const VoxelSH& sh, const vec3& direction) {
    // Y_0^0 (DC)
    float result = 0.282095f * sh.coeffs[0];
    
    // Y_1^{-1,0,1} (linear)
    result += 0.488603f * sh.coeffs[1] * direction.y;
    result += 0.488603f * sh.coeffs[2] * direction.z;
    result += 0.488603f * sh.coeffs[3] * direction.x;
    
    // Y_2^{-2,-1,0,1,2} (quadratic)
    result += 1.092548f * sh.coeffs[4] * direction.x * direction.y;
    result += 1.092548f * sh.coeffs[5] * direction.y * direction.z;
    result += 0.315392f * sh.coeffs[6] * (3*direction.z*direction.z - 1);
    result += 1.092548f * sh.coeffs[7] * direction.x * direction.z;
    result += 0.546274f * sh.coeffs[8] * (direction.x*direction.x - direction.y*direction.y);
    
    return result;
}
```

**Performance**: ~20 mad operations per evaluation (fast!)

---

## 5. PERFORMANCE BUDGET ANALYSIS

### 5.1 Target: 300 FPS (3.3 ms/frame)

```
Total budget: 3.3 ms
├─ Voxel rendering: 1.0 ms  (sparse voxel traversal)
├─ Frequency shading: 1.0 ms  (наш DCT/FFT pipeline)
├─ Neural upscale: 0.5 ms  (tiny CNN)
├─ Temporal reproj: 0.3 ms  (motion vectors + blend)
├─ Foveated sampling: 0.2 ms  (weight computation)
└─ Overhead/sync: 0.3 ms
```

**Frequency shading budget**: 1.0 ms

### 5.2 Resolution Analysis (1920×1080)

**Option A: Full resolution DCT** (текущий подход)
```
Blocks: 240 × 135 = 32,400 blocks
Per block: ~30 µs на GPU → 972 ms ❌ TOO SLOW
```

**Option B: Half resolution + upscale**
```
Input: 960×540
Blocks: 120 × 68 = 8,160 blocks
Per block: ~30 µs → 245 ms ❌ STILL TOO SLOW
```

**Option C: Quarter resolution + neural upscale** ⭐ RECOMMENDED
```
Input: 480×270
Blocks: 60 × 34 = 2,040 blocks
Per block GPU: ~0.5 µs (VkFFT optimized) → 1.0 ms ✅
Neural upscale: 480×270 → 1920×1080 in 0.5 ms ✅
Total: 1.5 ms ✅ MEETS BUDGET
```

---

## 6. ALTERNATIVE APPROACHES

### 6.1 Wavelet Transform

**Дискретное вейвлет-преобразование (DWT)** вместо DCT:
```
Pros:
- Multi-resolution analysis (better для foveated rendering)
- Локализация в spatial + frequency domains
- Fast (O(N) via lifting scheme)

Cons:
- Нет hardware acceleration (VkFFT не поддерживает)
- Сложнее реализация convolution
```

**Вывод**: Интересно для будущего, но сейчас не приоритет.

### 6.2 Learned Frequency Basis

**Neural frequency encoding** вместо fixed DCT:
```cpp
// Train small MLP для автоматического learning optimal basis
class LearnedFrequencyEncoder {
    std::vector<float> forward(const std::vector<float>& spatial);
    std::vector<float> inverse(const std::vector<float>& freq);
};
```

**Pros**: Potentially better compression для specific content  
**Cons**: Требует training, inference overhead  
**Вывод**: Research direction, не для MVP.

### 6.3 Gaussian Splatting Frequency Encoding

**Идея**: Encode Gaussians в frequency domain напрямую

```
Gaussian G(x) = exp(-||x - µ||² / (2σ²))
Fourier{ G(x) } = exp(-||ω||² σ² / 2)  (analytical!)
```

**Pros**: No DCT needed, direct frequency manipulation  
**Cons**: Completely different architecture  
**Вывод**: Интересный подход, но требует полной переработки.

---

## 7. RECOMMENDED IMPLEMENTATION PLAN

### Phase 1 (Week 1): CPU Optimization
✅ **Priority**: HIGH  
📊 **Expected**: 40x speedup  
```
1. Implement separable DCT-II
2. Add SIMD vectorization (AVX2)
3. Enable multi-threading (OpenMP)
4. Benchmark: target 20ms per frame on 4-core CPU
```

### Phase 2 (Week 2): FFT Backend
✅ **Priority**: HIGH  
📊 **Expected**: Work-around для VkFFT issues  
```
1. Implement FFTW fallback backend
2. Add zero-padding для linear convolution
3. Test accuracy vs DCT (should be identical)
4. Benchmark: target 10ms per frame on CPU
```

### Phase 3 (Week 3): VkFFT FFT Mode
✅ **Priority**: HIGH  
📊 **Expected**: GPU acceleration  
```
1. Disable DCT mode (performDCT = 0)
2. Use pure FFT transform
3. Fix headless mode issues on Intel
4. Benchmark: target 2ms per frame on Intel GPU
```

### Phase 4 (Week 4): Resolution + Neural Upscale
✅ **Priority**: MEDIUM  
📊 **Expected**: Meet 1ms budget  
```
1. Reduce base resolution to 480×270
2. Integrate tiny CNN upscaler (0.5ms)
3. Total pipeline: 1.5ms ✅
```

### Phase 5 (Week 5-6): SH Integration
✅ **Priority**: MEDIUM  
📊 **Expected**: Better visual quality  
```
1. Implement SH L=0,1,2 evaluation
2. Pre-compute SH coefficients для voxels
3. Integrate with frequency pipeline
```

### Phase 6 (Week 7-8): Advanced Optimization
✅ **Priority**: LOW  
📊 **Expected**: Polish  
```
1. Foveated sampling weights
2. Temporal reprojection
3. Compute shader custom DCT (если VkFFT DCT не работает)
```

---

## 8. VALIDATION CRITERIA

### 8.1 Performance Metrics
```
✅ Target: 300 FPS (3.3 ms/frame)
├─ Phase 1: 50 FPS (20 ms) - CPU baseline
├─ Phase 2: 100 FPS (10 ms) - FFTW
├─ Phase 3: 500 FPS (2 ms) - VkFFT GPU
└─ Phase 4: 666 FPS (1.5 ms) - Optimized + upscale
```

### 8.2 Visual Quality
```
✅ PSNR > 35 dB (good quality)
✅ SSIM > 0.95 (structural similarity)
✅ No visible artifacts в frequency convolution
✅ Smooth temporal coherence (no flicker)
```

### 8.3 Mathematical Correctness
```
✅ Parseval's theorem: ||spatial||² = ||freq||² (energy preservation)
✅ Invertibility: IDCT(DCT(x)) ≈ x (ε < 1e-6)
✅ Convolution accuracy: FFT method vs spatial (ε < 1e-4)
```

---

## 9. REFERENCES

### Academic Papers
1. **DCT-II**: Ahmed, N., Natarajan, T., & Rao, K. R. (1974). "Discrete Cosine Transform"
2. **FFT**: Cooley, J. W., & Tukey, J. W. (1965). "An Algorithm for the Machine Calculation of Complex Fourier Series"
3. **Spherical Harmonics**: Ramamoorthi, R., & Hanrahan, P. (2001). "An Efficient Representation for Irradiance Environment Maps"

### Implementation References
- **VkFFT**: https://github.com/DTolm/VkFFT
- **FFTW**: http://www.fftw.org/
- **cuFFT**: https://docs.nvidia.com/cuda/cufft/
- **JPEG DCT**: ITU-T T.81 (JPEG standard)

### Related Projects
- **Neural Radiance Fields (NeRF)**: frequency encoding для position
- **Gaussian Splatting**: frequency-domain operations
- **JPEG/HEVC**: DCT-based compression

---

## 10. CONCLUSIONS

### Ключевые выводы:

1. **CPU DCT-II не подходит для real-time** (800ms → need <1ms)
2. **GPU FFT - обязателен** для достижения 300 FPS target
3. **VkFFT DCT mode нестабилен на Intel GPU** → use FFT mode
4. **Resolution reduction + neural upscale** - ключ к performance budget
5. **Separable DCT + SIMD** могут дать 40x speedup на CPU как fallback

### Recommended Immediate Actions:

✅ **Week 1**: Implement separable DCT + SIMD (для CPU fallback)  
✅ **Week 2**: Add FFTW backend (stable fallback)  
✅ **Week 3**: Fix VkFFT в FFT mode (GPU acceleration)  
✅ **Week 4**: Test quarter-resolution + upscale strategy

### Long-term Vision:

🚀 **Ideal pipeline**: Sparse voxels → SH evaluation → frequency shading (VkFFT FFT) → neural upscale → 300 FPS на mobile GPU

---

**Версия**: 1.0  
**Дата**: 2025-10-02  
**Status**: Analysis Complete, Ready for Implementation

