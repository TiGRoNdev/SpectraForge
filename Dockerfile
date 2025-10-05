# =============================================================================
# SpectraForge CI/CD Docker Image (No vcpkg - System Packages Only)
# Multi-stage build for optimal caching and size optimization
# =============================================================================

# -----------------------------------------------------------------------------
# Stage 1: Base System Dependencies
# -----------------------------------------------------------------------------
FROM ubuntu:22.04 AS base

# Prevent interactive prompts during package installation
ENV DEBIAN_FRONTEND=noninteractive \
    TZ=UTC \
    LANG=C.UTF-8 \
    LC_ALL=C.UTF-8

# Install essential system packages and core dependencies
RUN apt-get update && apt-get install -y --no-install-recommends \
    # Build essentials
    build-essential \
    cmake \
    ninja-build \
    ccache \
    pkg-config \
    git \
    wget \
    curl \
    ca-certificates \
    gnupg \
    software-properties-common \
    # Graphics and rendering libraries
    libgl1-mesa-dev \
    libglu1-mesa-dev \
    libx11-dev \
    libxrandr-dev \
    libxinerama-dev \
    libxcursor-dev \
    libxi-dev \
    libxext-dev \
    # Vulkan SDK and tools
    libvulkan-dev \
    vulkan-tools \
    vulkan-validationlayers \
    spirv-tools \
    glslang-tools \
    # Core libraries
    zlib1g-dev \
    libssl-dev \
    libbz2-dev \
    liblzma-dev \
    # Python for build scripts
    python3 \
    python3-pip \
    python3-dev \
    && rm -rf /var/lib/apt/lists/*

# -----------------------------------------------------------------------------
# Stage 2: Project Dependencies
# -----------------------------------------------------------------------------
FROM base AS project-deps

# Install all project-specific libraries
RUN apt-get update && apt-get install -y --no-install-recommends \
    # GLFW3 - Window and input handling
    libglfw3-dev \
    # GLEW - OpenGL extension loading
    libglew-dev \
    # GLM - Mathematics library
    libglm-dev \
    # Assimp - 3D model loading
    libassimp-dev \
    # FreeImage/STB alternative - Image loading
    libfreeimage-dev \
    libpng-dev \
    libjpeg-dev \
    # Google Test - Testing framework
    libgtest-dev \
    libgmock-dev \
    # Additional Wayland support
    libwayland-dev \
    libxkbcommon-dev \
    && rm -rf /var/lib/apt/lists/*

# Build and install Google Test (Ubuntu 22.04 needs manual build)
RUN cd /usr/src/gtest && \
    cmake . -DBUILD_SHARED_LIBS=ON && \
    cmake --build . -j$(nproc) && \
    cp lib/*.so /usr/lib/ || true

# Install ImGui from source (not available as Ubuntu package)
RUN git clone --depth 1 --branch v1.89.9 https://github.com/ocornut/imgui.git /opt/imgui && \
    mkdir -p /usr/local/include/imgui && \
    cp /opt/imgui/*.h /usr/local/include/imgui/ && \
    cp /opt/imgui/*.cpp /usr/local/include/imgui/ && \
    cp -r /opt/imgui/backends /usr/local/include/imgui/ && \
    cp -r /opt/imgui/misc /usr/local/include/imgui/

# Install Vulkan Memory Allocator (header-only library)
RUN git clone --depth 1 --branch v3.0.1 https://github.com/GPUOpen-LibrariesAndSDKs/VulkanMemoryAllocator.git /opt/vma && \
    mkdir -p /usr/local/include/vma && \
    cp /opt/vma/include/vk_mem_alloc.h /usr/local/include/vma/

# Install STB (header-only libraries)
RUN git clone --depth 1 https://github.com/nothings/stb.git /opt/stb && \
    mkdir -p /usr/local/include/stb && \
    cp /opt/stb/*.h /usr/local/include/stb/

# Install Vulkan-Hpp (C++ bindings for Vulkan)
# Используем более новую версию с поддержкой video encoding типов
RUN git clone --depth 1 --branch v1.3.296 https://github.com/KhronosGroup/Vulkan-Headers.git /opt/vulkan-headers && \
    mkdir -p /usr/local/include/vulkan && \
    cp -r /opt/vulkan-headers/include/vulkan/*.hpp /usr/local/include/vulkan/ || true && \
    cp -r /opt/vulkan-headers/include/vulkan/*.h /usr/local/include/vulkan/ || true && \
    cp -r /opt/vulkan-headers/include/vk_video /usr/local/include/ || true

# -----------------------------------------------------------------------------
# Stage 3: Quality Tools Installation
# -----------------------------------------------------------------------------
FROM project-deps AS quality-tools

# Install LLVM/Clang toolchain (version 16)
RUN wget https://apt.llvm.org/llvm.sh && \
    chmod +x llvm.sh && \
    ./llvm.sh 16 && \
    rm llvm.sh && \
    apt-get update && apt-get install -y --no-install-recommends \
    clang-16 \
    clang-tidy-16 \
    clang-format-16 \
    clang-tools-16 \
    lldb-16 \
    lld-16 \
    libc++-16-dev \
    libc++abi-16-dev \
    && rm -rf /var/lib/apt/lists/*

# Create symlinks for clang tools
RUN update-alternatives --install /usr/bin/clang clang /usr/bin/clang-16 100 && \
    update-alternatives --install /usr/bin/clang++ clang++ /usr/bin/clang++-16 100 && \
    update-alternatives --install /usr/bin/clang-tidy clang-tidy /usr/bin/clang-tidy-16 100 && \
    update-alternatives --install /usr/bin/clang-format clang-format /usr/bin/clang-format-16 100

# Install static analysis and quality tools
RUN apt-get update && apt-get install -y --no-install-recommends \
    # Static analysis
    cppcheck \
    iwyu \
    valgrind \
    # Memory leak detection
    libasan6 \
    libubsan1 \
    libtsan0 \
    # Code coverage
    gcovr \
    lcov \
    # Documentation
    doxygen \
    graphviz \
    && rm -rf /var/lib/apt/lists/*

# Install Python-based quality tools
RUN pip3 install --no-cache-dir \
    cpplint \
    lizard \
    cmakelint \
    cmake-format \
    pyyaml

# Note: Using cppcheck from Ubuntu repos (v2.7) instead of building from source for faster builds
# If newer version needed, can build from source by uncommenting below:
# ARG CPPCHECK_VERSION=2.13.0
# RUN wget https://github.com/danmar/cppcheck/archive/refs/tags/${CPPCHECK_VERSION}.tar.gz && \
#     tar -xzf ${CPPCHECK_VERSION}.tar.gz && \
#     cd cppcheck-${CPPCHECK_VERSION} && \
#     mkdir build && cd build && \
#     cmake .. -DCMAKE_BUILD_TYPE=Release && \
#     cmake --build . -j$(nproc) && \
#     cmake --install . && \
#     cd ../.. && rm -rf cppcheck-${CPPCHECK_VERSION} ${CPPCHECK_VERSION}.tar.gz

# -----------------------------------------------------------------------------
# Stage 4: Development Environment
# -----------------------------------------------------------------------------
FROM quality-tools AS development

# Setup ccache for faster rebuilds
ENV CCACHE_DIR=/ccache \
    CCACHE_MAXSIZE=2G \
    CMAKE_C_COMPILER_LAUNCHER=ccache \
    CMAKE_CXX_COMPILER_LAUNCHER=ccache

RUN mkdir -p ${CCACHE_DIR}

# Setup environment variables for build (no vcpkg toolchain needed)
ENV CMAKE_EXPORT_COMPILE_COMMANDS=ON \
    PKG_CONFIG_PATH=/usr/lib/x86_64-linux-gnu/pkgconfig:/usr/local/lib/pkgconfig

# Create working directory
WORKDIR /workspace

# Copy project files
COPY CMakeLists.txt CMakePresets.json ./
COPY cmake/ ./cmake/
COPY src/ ./src/
COPY include/ ./include/
COPY tests/ ./tests/
COPY examples/ ./examples/
COPY shaders/ ./shaders/
COPY scripts/ ./scripts/
COPY .clang-format .clang-tidy cppcheck.cfg ./

# Make scripts executable
RUN chmod +x scripts/*.sh scripts/*.bat || true

# -----------------------------------------------------------------------------
# Stage 5: CI/CD Runner (Final Stage for GitHub Actions)
# -----------------------------------------------------------------------------
FROM development AS ci-runner

# Pre-configure CMake to cache configuration (without vcpkg toolchain)
RUN cmake -B /workspace/build \
    -S /workspace \
    -G Ninja \
    -DCMAKE_BUILD_TYPE=Release \
    -DCMAKE_EXPORT_COMPILE_COMMANDS=ON \
    -DBUILD_TESTS=ON \
    -DENABLE_VULKAN_BACKEND=ON \
    -DENABLE_OPENGL_BACKEND=ON \
    -DBUILD_WITH_CUDA=OFF \
    -DBUILD_WITH_OPTIX=OFF \
    -DBUILD_WITH_DLSS=OFF \
    -DBUILD_WITH_FSR=OFF \
    || true

# Create directories for reports
RUN mkdir -p \
    /workspace/build/quality-reports \
    /workspace/build/coverage \
    /workspace/build/static-analysis \
    /workspace/build/test-results

# Set default command to show available tools
CMD ["/bin/bash", "-c", "echo '=== SpectraForge CI/CD Environment (No vcpkg) ===' && \
     echo 'CMake: ' && cmake --version && \
     echo 'Clang: ' && clang --version && \
     echo 'Clang-Tidy: ' && clang-tidy --version && \
     echo 'Clang-Format: ' && clang-format --version && \
     echo 'Cppcheck: ' && cppcheck --version && \
     echo 'Ninja: ' && ninja --version && \
     echo 'GTest: ' && pkg-config --modversion gtest || echo 'N/A' && \
     echo 'GLFW: ' && pkg-config --modversion glfw3 || echo 'N/A' && \
     echo 'GLM: ' && pkg-config --modversion glm || echo 'N/A' && \
     echo '' && \
     echo 'Environment ready for CI/CD checks!' && \
     /bin/bash"]

# -----------------------------------------------------------------------------
# Stage 6: Build Stage (for actual compilation)
# -----------------------------------------------------------------------------
FROM ci-runner AS builder

# Build the project
RUN cmake --build /workspace/build --config Release -j$(nproc) || true

# Run tests
RUN cd /workspace/build && ctest --output-on-failure --parallel $(nproc) || true

# -----------------------------------------------------------------------------
# Labels and Metadata
# -----------------------------------------------------------------------------
LABEL maintainer="TiGRoNdev <https://github.com/TiGRoNdev>" \
      org.opencontainers.image.title="SpectraForge CI/CD (System Packages)" \
      org.opencontainers.image.description="Docker image for SpectraForge CI/CD using system packages (no vcpkg)" \
      org.opencontainers.image.url="https://github.com/TiGRoNdev/SpectraForge" \
      org.opencontainers.image.source="https://github.com/TiGRoNdev/SpectraForge" \
      org.opencontainers.image.vendor="TiGRoNdev" \
      org.opencontainers.image.version="0.1.0"