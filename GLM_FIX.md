# GLM Dependency Fix for SpectraForge

## Problem
The SpectraForge build fails with the error:
```
fatal error: glm/glm.hpp: No such file or directory
```

This happens because the GLM (OpenGL Mathematics) library is not installed on the system.

## Solution

### Install the GLM development package:
```bash
sudo apt-get install libglm-dev
```

### Alternative: Manual GLM Installation
If the package manager approach doesn't work, you can manually install GLM:

1. Download GLM from https://github.com/g-truc/glm
2. Extract the archive
3. Copy the `glm` directory to `/usr/local/include/`:
```bash
sudo cp -r glm /usr/local/include/
```

## Verification
After installing GLM, you can verify the installation:
```bash
find /usr -name "glm.hpp" 2>/dev/null
```

This should return the path to the glm.hpp file.

## For Developers
The build system now includes a custom FindGLM.cmake module that automatically detects GLM in standard locations and provides helpful error messages when it's not found.
```