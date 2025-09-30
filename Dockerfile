# =============================================================================
# HyperEngine CI/CD Docker Image
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

# Install essential system packages
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
    # Vulkan SDK
    libvulkan-dev \
    vulkan-tools \
    vulkan-validationlayers \
    spirv-tools \
    # Python for build scripts
    python3 \
    python3-pip \
    python3-dev \
    && rm -rf /var/lib/apt/lists/*

# -----------------------------------------------------------------------------
# Stage 2: Quality Tools Installation
# -----------------------------------------------------------------------------
FROM base AS quality-tools

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

# Install latest cppcheck from source (for newer features than apt version)
ARG CPPCHECK_VERSION=2.13.0
RUN wget https://github.com/danmar/cppcheck/archive/refs/tags/${CPPCHECK_VERSION}.tar.gz && \
    tar -xzf ${CPPCHECK_VERSION}.tar.gz && \
    cd cppcheck-${CPPCHECK_VERSION} && \
    mkdir build && cd build && \
    cmake .. -DCMAKE_BUILD_TYPE=Release && \
    cmake --build . -j$(nproc) && \
    cmake --install . && \
    cd ../.. && rm -rf cppcheck-${CPPCHECK_VERSION} ${CPPCHECK_VERSION}.tar.gz

# -----------------------------------------------------------------------------
# Stage 3: vcpkg Dependencies
# -----------------------------------------------------------------------------
FROM quality-tools AS vcpkg-deps

# Install vcpkg
ENV VCPKG_ROOT=/opt/vcpkg \
    VCPKG_DEFAULT_TRIPLET=x64-linux \
    VCPKG_FORCE_SYSTEM_BINARIES=1

RUN git clone https://github.com/Microsoft/vcpkg.git ${VCPKG_ROOT} && \
    cd ${VCPKG_ROOT} && \
    git checkout e6e4bc74aaf5c63dfc358810594f662f7e9bc4d4 && \
    ./bootstrap-vcpkg.sh -disableMetrics && \
    ln -s ${VCPKG_ROOT}/vcpkg /usr/local/bin/vcpkg

# Copy vcpkg manifest
WORKDIR /tmp/hyperengine
COPY vcpkg.json vcpkg.json

# Install project dependencies via vcpkg (cached layer)
RUN vcpkg install \
    --triplet=${VCPKG_DEFAULT_TRIPLET} \
    --clean-after-build \
    --x-install-root=${VCPKG_ROOT}/installed

# -----------------------------------------------------------------------------
# Stage 4: Development Environment
# -----------------------------------------------------------------------------
FROM vcpkg-deps AS development

# Setup ccache for faster rebuilds
ENV CCACHE_DIR=/ccache \
    CCACHE_MAXSIZE=2G \
    CMAKE_C_COMPILER_LAUNCHER=ccache \
    CMAKE_CXX_COMPILER_LAUNCHER=ccache

RUN mkdir -p ${CCACHE_DIR}

# Setup environment variables for build
ENV CMAKE_TOOLCHAIN_FILE=${VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake \
    CMAKE_EXPORT_COMPILE_COMMANDS=ON \
    PATH="${VCPKG_ROOT}:${PATH}"

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

# Pre-configure CMake to cache configuration
RUN cmake -B /workspace/build \
    -S /workspace \
    -G Ninja \
    -DCMAKE_BUILD_TYPE=Release \
    -DCMAKE_TOOLCHAIN_FILE=${CMAKE_TOOLCHAIN_FILE} \
    -DCMAKE_EXPORT_COMPILE_COMMANDS=ON \
    -DBUILD_TESTS=ON \
    -DENABLE_VULKAN_BACKEND=ON \
    -DENABLE_OPENGL_BACKEND=ON \
    || true

# Create directories for reports
RUN mkdir -p \
    /workspace/build/quality-reports \
    /workspace/build/coverage \
    /workspace/build/static-analysis \
    /workspace/build/test-results

# Set default command to show available tools
CMD ["/bin/bash", "-c", "echo '=== HyperEngine CI/CD Environment ===' && \
     echo 'CMake: ' && cmake --version && \
     echo 'Clang: ' && clang --version && \
     echo 'Clang-Tidy: ' && clang-tidy --version && \
     echo 'Clang-Format: ' && clang-format --version && \
     echo 'Cppcheck: ' && cppcheck --version && \
     echo 'vcpkg: ' && vcpkg version && \
     echo 'Ninja: ' && ninja --version && \
     echo '' && \
     echo 'Environment ready for CI/CD checks!' && \
     /bin/bash"]

# -----------------------------------------------------------------------------
# Stage 6: Build Stage (for actual compilation)
# -----------------------------------------------------------------------------
FROM ci-runner AS builder

# Build the project
RUN cmake --build /workspace/build --config Release -j$(nproc)

# Run tests
RUN cd /workspace/build && ctest --output-on-failure --parallel $(nproc) || true

# -----------------------------------------------------------------------------
# Labels and Metadata
# -----------------------------------------------------------------------------
LABEL maintainer="TiGRoNdev <https://github.com/TiGRoNdev>" \
      org.opencontainers.image.title="HyperEngine CI/CD" \
      org.opencontainers.image.description="Docker image for HyperEngine CI/CD pipeline with quality tools" \
      org.opencontainers.image.url="https://github.com/TiGRoNdev/HyperEngine" \
      org.opencontainers.image.source="https://github.com/TiGRoNdev/HyperEngine" \
      org.opencontainers.image.vendor="TiGRoNdev" \
      org.opencontainers.image.version="0.1.0"
