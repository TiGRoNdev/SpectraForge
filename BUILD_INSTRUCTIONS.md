# Build Instructions for 4D Engine (Vulkan Forward+)

## Prerequisites

- Visual Studio 2019 or 2022 (with C++ support)
- CMake 3.16 or later  
- Git (for vcpkg)
- **Vulkan SDK 1.3.0 or later** - Download from https://vulkan.lunarg.com/

## Option 1: Build with vcpkg (Recommended)

### 1. Install vcpkg

```bash
git clone https://github.com/Microsoft/vcpkg.git
cd vcpkg
.\bootstrap-vcpkg.bat
```

### 2. Install Vulkan SDK

Download and install Vulkan SDK from https://vulkan.lunarg.com/
Make sure the `VULKAN_SDK` environment variable is set.

### 3. Install dependencies

```bash
.\vcpkg\vcpkg.exe install glfw3:x64-windows vulkan:x64-windows vulkan-memory-allocator:x64-windows glm:x64-windows
```

### 4. Compile shaders

```bash
# Compile Vulkan shaders
.\compile_shaders.bat
```

### 5. Build the project

```bash
# Use the provided script
.\build_with_vcpkg.bat

# Or manually:
mkdir build-vcpkg
cd build-vcpkg
cmake .. -DCMAKE_TOOLCHAIN_FILE=../vcpkg/scripts/buildsystems/vcpkg.cmake -G "Visual Studio 17 2022" -A x64
cmake --build . --config Release
```

## Option 2: Build with system packages

### 1. Install dependencies manually

You need to install Vulkan SDK, GLFW3, VMA, and GLM on your system.

### 2. Build the project

```bash
mkdir build
cd build
cmake .. -G "Visual Studio 17 2022" -A x64
cmake --build . --config Release
```

## Option 3: Use CMake Presets

```bash
# Configure with vcpkg
cmake --preset windows-vcpkg

# Build
cmake --build --preset windows-vcpkg
```

## Troubleshooting

### Vulkan SDK not found

If you get Vulkan-related errors:

1. Make sure Vulkan SDK is properly installed
2. Check that `VULKAN_SDK` environment variable is set
3. Restart your command prompt/IDE after installing Vulkan SDK

### Shader compilation failed

If shader compilation fails:

1. Make sure Vulkan SDK is installed and `VULKAN_SDK` is set
2. Check that `glslc.exe` exists in `%VULKAN_SDK%\Bin\`
3. Run `.\compile_shaders.bat` manually to see detailed error messages

### GLFW3 not found

If you get "Could not find a package configuration file provided by glfw3", try:

1. Make sure vcpkg is properly installed
2. Use the vcpkg toolchain file: `-DCMAKE_TOOLCHAIN_FILE=../vcpkg/scripts/buildsystems/vcpkg.cmake`
3. Check that GLFW3 is installed: `.\vcpkg\vcpkg.exe list`

## Project Structure

- `src/` - Source code
- `include/` - Header files
- `examples/` - Example applications
- `cmake/` - CMake modules
- `build/` - Build output (created during build)
- `vcpkg/` - vcpkg installation (if using vcpkg)

## Output

After successful build, you'll find:
- `Engine4D.lib` - Static library with Vulkan Forward+ rendering
- `Engine4D_Demo.exe` - Demo application
- `shaders/compiled/` - Compiled SPIR-V shaders

## New Features in Vulkan Forward+ Version

- **Forward+ Rendering**: Tile-based deferred lighting for better performance with many lights
- **4D Support**: Enhanced 4D geometry rendering with proper projections
- **Compute Shaders**: GPU-accelerated light culling
- **Modern Graphics**: Vulkan API for better performance and control
- **PBR Materials**: Physically-based rendering support