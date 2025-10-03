# Legacy Examples - FreqVox Renderer

> **⚠️ LEGACY CODE**: These examples use the **legacy FreqVox renderer** and are kept for historical reference and comparison purposes.

---

## 📂 Contents

### **FreqVox Examples**
- **`freqvox_demo.cpp`** - Basic FreqVox renderer demonstration
- **`freqvox_sponza_demo.cpp`** - Full-featured Sponza scene with FreqVox renderer
- **`test_external_memory.cpp`** - External memory testing for FreqVox

---

## ⚠️ Important Notes

### **Primary Renderer**
The current primary renderer for SpectraForge is **Hybrid DWT + FreGS** (Wavelet Lifting + Frequency-Encoded Gaussian Splatting).

For new projects, use:
- **`examples/hybrid_fregs_demo.cpp`** - Hybrid DWT + FreGS demonstration

### **Why Keep Legacy Code?**
1. **Historical Reference** - Preserve original FreqVox implementation
2. **Comparison Benchmarks** - Compare performance with new renderer
3. **Research** - Study frequency-domain rendering techniques
4. **Educational** - Learn from previous approaches

---

## 🚀 FreqVox Overview

**FreqVox (Adaptive Frequency-Domain Sparse Neural-Voxel Rendering)** was an experimental renderer combining:

- 🎯 **Foveated Rendering** - Smart GPU resource distribution
- 🌊 **Frequency-Domain Shading** - DCT/FFT acceleration
- ⏱️ **Temporal Reprojection** - Frame stability
- 🧠 **Neural Upscaling** - DLSS/FSR2 integration

### **Performance (RTX 3060)**
- 🎯 FPS: 120+ @ 1080p upscaled
- 🔥 Voxels: 50,000+ active voxels
- ⚡ Speedup: 4x via foveation
- 🧠 Upscale: 2-8x boost with DLSS/FSR2

---

## 📚 Documentation

### **Legacy FreqVox Documentation**
All FreqVox documentation has been moved to:
- **`docs/legacy/freqvox/`** (28 files)

### **Current Architecture Documentation**
For the current Hybrid DWT + FreGS renderer:
- **`docs/architecture/Renderer.md`**
- **`README.md`** (main project README)

---

## 🛠️ Building Legacy Examples

### **CMake Options**
```bash
# Select FreqVox renderer (legacy)
cmake -DSPECTRAFORGE_RENDERER=FREQVOX ..

# Build all examples (including legacy)
cmake --build . --target all
```

### **Running Legacy Examples**

#### **FreqVox Demo (Basic)**
```bash
./build/FreqVox_Demo
```

#### **FreqVox Sponza Demo (Full-Featured)**
```bash
# Using provided script (recommended)
./run_freqvox_sponza.sh

# Or manually
./build/FreqVox_Sponza_Demo
```

**Features:**
- ✅ Sponza scene loading (OBJ/MTL)
- ✅ Geometry voxelization
- ✅ Full FreqVox pipeline
- ✅ Interactive camera (WASD + mouse)
- ✅ Real-time statistics

---

## ⚡ Performance Comparison

| Renderer | Resolution | FPS | Quality | Notes |
|----------|------------|-----|---------|-------|
| **FreqVox** | 1080p | 120+ | High | Legacy, voxel-based |
| **Hybrid DWT + FreGS** | 1080p | 180+ | Higher | Current, wavelet-based |
| **FreqVox** | 4K upscaled | 60-80 | High | With DLSS/FSR2 |
| **Hybrid DWT + FreGS** | 4K upscaled | 120+ | Higher | 16x coverage improvement |

**Key Improvements in Hybrid DWT + FreGS:**
- 🚀 **+50% FPS** at same quality
- ✨ **16x coverage improvement** (per-pixel vs subgroup)
- 🎯 **Better foveation alignment**
- ⚡ **Vulkan Subgroups optimization**

---

## 📖 Migration Guide

### **From FreqVox to Hybrid DWT + FreGS**

If you have code using FreqVox, here's how to migrate:

#### **1. Scene Representation**
```cpp
// OLD (FreqVox)
FreqVoxScene scene;
scene.voxelize(geometry);

// NEW (Hybrid DWT + FreGS)
WaveletPass waveletPass(context);
FreGSPass fregsPass(context);
```

#### **2. Rendering Pipeline**
```cpp
// OLD (FreqVox)
freqvoxRenderer.render(scene, camera);

// NEW (Hybrid DWT + FreGS)
waveletPass.execute(cmd, inputImage, subbands);
fregsPass.execute(cmd, subbands, gaussians, outputImage);
```

#### **3. Upscaling**
```cpp
// OLD (FreqVox)
DLSSUpscaler upscaler; // Direct instantiation

// NEW (Hybrid DWT + FreGS)
auto upscaler = UpscalerFactory::create(UpscalerType::AUTO, gpuVendorId);
upscaler->initialize(context, config);
```

---

## 🔗 Related Files

### **Legacy Scripts**
- `run_freqvox_sponza.sh` - FreqVox Sponza demo launcher (root)
- `run_freqvox_vulkan_demo.sh` - FreqVox Vulkan demo launcher (root)

### **Legacy Benchmarks**
- `freqvox_bench` - FreqVox benchmark binary (root)

### **Current Examples**
- `examples/hybrid_fregs_demo.cpp` - **Use this for new projects!**
- `examples/optimal_renderer_demo.cpp` - OptimalRenderer3D demo
- `examples/vulkan_demo.cpp` - Vulkan backend demo

---

## 📝 Status

- **Maintenance:** ⚠️ **Not actively maintained**
- **Purpose:** Historical reference, benchmarks, research
- **Recommendation:** Use **Hybrid DWT + FreGS** for new projects

---

**Last Updated:** 2025-10-02  
**Moved to Legacy:** Phase 6 Cleanup  
**Reason:** Replaced by Hybrid DWT + FreGS renderer  
**Documentation:** `docs/legacy/freqvox/` (28 files)

