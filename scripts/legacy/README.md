# Legacy Scripts & Test Files

> **⚠️ OBSOLETE**: These files are no longer actively used and are kept for historical reference only.

---

## 📂 Contents

### **Legacy FreqVox Scripts**
- **`run_freqvox_sponza.sh`** - Launcher for FreqVox Sponza demo
- **`run_freqvox_vulkan_demo.sh`** - Launcher for FreqVox Vulkan demo
- **`freqvox_bench`** - FreqVox benchmark binary (112KB)

### **Obsolete Test Files**
- **`test_glfw_vulkan_hpp_conflict.cpp`** - GLFW/Vulkan-HPP conflict test (fixed in Phase 2)
- **`test_vulkan_fix.sh`** - Vulkan fix validation script (no longer needed)

---

## ⚠️ Important Notes

### **Current Scripts**
For current build and run scripts, use:
- **`build_linux.sh`** - Main Linux build script (root)
- **`scripts/build_*.sh`** - Build automation scripts
- **`scripts/run_*.sh`** - Current demo launchers

### **Current Examples**
For current demo applications, use:
- **`examples/hybrid_fregs_demo.cpp`** - Hybrid DWT + FreGS demo
- **`examples/optimal_renderer_demo.cpp`** - OptimalRenderer3D demo
- **`examples/vulkan_demo.cpp`** - Vulkan backend demo

---

## 🚀 FreqVox Scripts Usage (Legacy)

### **FreqVox Sponza Demo**
```bash
# From project root (legacy, not recommended)
./scripts/legacy/run_freqvox_sponza.sh

# Recommended: Use legacy examples directly
cd build
./FreqVox_Sponza_Demo  # If built with FREQVOX renderer
```

### **FreqVox Vulkan Demo**
```bash
# From project root (legacy, not recommended)
./scripts/legacy/run_freqvox_vulkan_demo.sh

# Recommended: Use legacy examples directly
cd build
./FreqVox_Demo  # If built with FREQVOX renderer
```

### **FreqVox Benchmark**
```bash
# Run legacy benchmark
./scripts/legacy/freqvox_bench

# Note: This binary was compiled with legacy FreqVox renderer
# For current benchmarks, use hybrid_fregs_demo with --benchmark flag
```

---

## 🔧 Obsolete Test Files

### **test_glfw_vulkan_hpp_conflict.cpp**
**Purpose:** Tested GLFW and Vulkan-HPP header conflict  
**Status:** ✅ **FIXED** in Phase 2  
**Resolution:** Proper header order and include guards  
**Replacement:** No longer needed, conflict resolved

### **test_vulkan_fix.sh**
**Purpose:** Validated Vulkan initialization and presentation  
**Status:** ✅ **FIXED** in Phase 2  
**Resolution:** Vulkan 1.3 context properly initialized  
**Replacement:** Unit tests in `tests/unit/VulkanContext_Test.cpp`

---

## 📝 Why Moved to Legacy?

### **FreqVox Scripts** (3 files)
- **Reason:** FreqVox renderer replaced by Hybrid DWT + FreGS
- **Status:** Legacy/Experimental
- **Recommendation:** Use `hybrid_fregs_demo` instead
- **Kept for:** Historical reference, benchmarks

### **Test Files** (2 files)
- **Reason:** Issues fixed, tests no longer needed
- **Status:** Obsolete
- **Recommendation:** Use current unit tests in `tests/unit/`
- **Kept for:** Historical reference, documentation of fixes

---

## 🔗 Related Files

### **Current FreqVox Examples**
- `examples/legacy/freqvox_demo.cpp`
- `examples/legacy/freqvox_sponza_demo.cpp`
- `examples/legacy/README.md` (detailed FreqVox documentation)

### **Current Demo Scripts**
- `build_linux.sh` (root) - Main build script
- `scripts/build_*.sh` - Build automation
- `scripts/run_*.sh` - Current demo launchers

### **Current Tests**
- `tests/unit/*_Test.cpp` - All unit tests (65+ tests, 100% coverage)
- `tests/integration/` - Integration tests (if any)

---

## 📊 File Sizes

| File | Size | Type |
|------|------|------|
| `freqvox_bench` | 112 KB | Binary (executable) |
| `run_freqvox_sponza.sh` | 3.1 KB | Shell script |
| `run_freqvox_vulkan_demo.sh` | 1.2 KB | Shell script |
| `test_glfw_vulkan_hpp_conflict.cpp` | 1.6 KB | C++ source |
| `test_vulkan_fix.sh` | 2.1 KB | Shell script |
| **Total** | **~120 KB** | 5 files |

---

## 🗑️ Removal Policy

These files are kept for historical reference. Consider permanent removal in future versions if:
1. No active use for 6+ months
2. No reference in documentation
3. Team consensus on removal

---

**Last Updated:** 2025-10-02  
**Moved to Legacy:** Phase 6 Cleanup  
**Reason:** Obsolete/Replaced by current implementation  
**Status:** ⚠️ Not maintained, historical reference only

