# SpectraForge Docker CI/CD Guide

## 📦 Overview

This guide explains how to use the Docker-based CI/CD infrastructure for SpectraForge development and automated testing.

## 🏗️ Multi-Stage Docker Architecture

The Dockerfile uses a multi-stage build approach for optimal caching and flexibility:

### Build Stages

1. **base** - System dependencies and base tools
2. **quality-tools** - Clang, static analysis, and quality tools
3. **vcpkg-deps** - vcpkg and all project dependencies
4. **development** - Development environment setup
5. **ci-runner** - CI/CD environment (default)
6. **builder** - Full build and test execution

## 🚀 Quick Start

### Local Development

```bash
# Start development environment
docker-compose up -d dev

# Access the container
docker exec -it hyperengine-dev bash

# Inside container - run quality checks
./scripts/quality_check.sh

# Build the project
cmake --build build --config Release -j$(nproc)

# Run tests
cd build && ctest --output-on-failure
```

### CI/CD Runner

```bash
# Build CI/CD image
docker build --target ci-runner -t hyperengine-ci:latest .

# Run quality checks
docker run --rm -v $(pwd):/workspace hyperengine-ci:latest \
  bash -c "cd /workspace && ./scripts/quality_check.sh"

# Or use docker-compose
docker-compose run --rm ci-runner ./scripts/quality_check.sh
```

### Full Build

```bash
# Build everything
docker build --target builder -t hyperengine-builder:latest .

# Or use docker-compose
docker-compose up builder
```

## 🔧 Available Services

### 1. `ci-runner` (Recommended for CI/CD)

**Purpose**: Run code quality checks, linting, and static analysis

**Features**:
- ✅ Clang 16 (clang-tidy, clang-format)
- ✅ Cppcheck 2.12+
- ✅ Include-What-You-Use (IWYU)
- ✅ Valgrind
- ✅ Coverage tools (gcovr, lcov)
- ✅ All vcpkg dependencies pre-installed
- ✅ CMake pre-configured

**Usage**:
```bash
docker-compose run --rm ci-runner bash -c "
  clang-format --version
  clang-tidy --version
  cppcheck --version
  ./scripts/quality_check.sh
"
```

### 2. `builder`

**Purpose**: Compile and test the project

**Features**:
- ✅ Pre-built project binaries
- ✅ Test execution
- ✅ Optimized with ccache

**Usage**:
```bash
docker-compose run --rm builder
```

### 3. `dev`

**Purpose**: Interactive development environment

**Features**:
- ✅ All development tools
- ✅ Shell access
- ✅ Volume mounting for live editing

**Usage**:
```bash
docker-compose up -d dev
docker exec -it hyperengine-dev bash
```

### 4. `demo`

**Purpose**: Run GUI demos (with X11 forwarding)

**Usage**:
```bash
# Allow X11 connections
xhost +local:docker

# Run demo
docker-compose up demo
```

## 📋 GitHub Actions Integration

### Example Workflow

Create `.github/workflows/docker-ci.yml`:

```yaml
name: Docker CI/CD

on:
  push:
    branches: [ main, develop ]
  pull_request:
    branches: [ main, develop ]

jobs:
  docker-quality-check:
    name: Docker Quality Checks
    runs-on: ubuntu-latest
    
    steps:
      - name: Checkout code
        uses: actions/checkout@v4
        with:
          submodules: recursive
      
      - name: Set up Docker Buildx
        uses: docker/setup-buildx-action@v3
      
      - name: Build CI image
        uses: docker/build-push-action@v5
        with:
          context: .
          target: ci-runner
          tags: hyperengine-ci:latest
          cache-from: type=gha
          cache-to: type=gha,mode=max
          load: true
      
      - name: Run Quality Checks
        run: |
          docker run --rm \
            -v ${{ github.workspace }}:/workspace \
            hyperengine-ci:latest \
            bash -c "cd /workspace && ./scripts/quality_check.sh"
      
      - name: Upload Reports
        uses: actions/upload-artifact@v4
        if: always()
        with:
          name: quality-reports
          path: build/quality-reports/

  docker-build-test:
    name: Docker Build & Test
    runs-on: ubuntu-latest
    needs: docker-quality-check
    
    steps:
      - name: Checkout code
        uses: actions/checkout@v4
      
      - name: Set up Docker Buildx
        uses: docker/setup-buildx-action@v3
      
      - name: Build and Test
        uses: docker/build-push-action@v5
        with:
          context: .
          target: builder
          tags: hyperengine-builder:latest
          cache-from: type=gha
          cache-to: type=gha,mode=max
      
      - name: Extract Test Results
        run: |
          docker create --name builder hyperengine-builder:latest
          docker cp builder:/workspace/build/Testing ./Testing
          docker rm builder
      
      - name: Upload Test Results
        uses: actions/upload-artifact@v4
        with:
          name: test-results
          path: Testing/
```

## 🔍 Quality Tools Available

### Clang Tools

```bash
# Format check
clang-format --dry-run --Werror src/**/*.cpp include/**/*.h

# Static analysis
clang-tidy --config-file=.clang-tidy src/**/*.cpp
```

### Cppcheck

```bash
cppcheck --enable=all --std=c++20 \
  --config-file=cppcheck.cfg \
  --xml --xml-version=2 \
  --output-file=cppcheck-results.xml \
  src/ include/
```

### Coverage

```bash
# Build with coverage
cmake -B build -DCMAKE_BUILD_TYPE=Debug -DENABLE_COVERAGE=ON
cmake --build build

# Run tests
cd build && ctest

# Generate report
gcovr --xml --output coverage.xml
```

### Memory Checking

```bash
# Valgrind
valgrind --leak-check=full --show-leak-kinds=all \
  ./build/tests/unit_tests

# AddressSanitizer
cmake -B build -DCMAKE_BUILD_TYPE=Debug \
  -DCMAKE_CXX_FLAGS="-fsanitize=address -fno-omit-frame-pointer"
cmake --build build
./build/tests/unit_tests
```

## 📊 Environment Variables

### Build Configuration

```bash
# CMake
CMAKE_BUILD_TYPE=Release|Debug|RelWithDebInfo
CMAKE_TOOLCHAIN_FILE=/opt/vcpkg/scripts/buildsystems/vcpkg.cmake
CMAKE_EXPORT_COMPILE_COMMANDS=ON

# vcpkg
VCPKG_ROOT=/opt/vcpkg
VCPKG_DEFAULT_TRIPLET=x64-linux

# ccache (for faster rebuilds)
CCACHE_DIR=/ccache
CCACHE_MAXSIZE=2G
CMAKE_C_COMPILER_LAUNCHER=ccache
CMAKE_CXX_COMPILER_LAUNCHER=ccache
```

## 🎯 Best Practices

### 1. **Layer Caching**

The Dockerfile is optimized for layer caching. Dependencies are installed before source code is copied.

### 2. **Use Specific Targets**

```bash
# Don't build everything if you only need quality checks
docker build --target ci-runner -t hyperengine-ci .

# Use the smallest stage for your needs
docker-compose run --rm ci-runner  # Just for checks
docker-compose run --rm builder    # For full build
```

### 3. **Volume Mounting**

```bash
# Mount source code for live editing
docker run -v $(pwd):/workspace hyperengine-dev

# Use named volumes for caches
docker volume create hyperengine-ccache
docker run -v hyperengine-ccache:/ccache hyperengine-ci
```

### 4. **GitHub Actions Caching**

```yaml
- name: Cache Docker layers
  uses: actions/cache@v3
  with:
    path: /tmp/.buildx-cache
    key: ${{ runner.os }}-buildx-${{ github.sha }}
    restore-keys: |
      ${{ runner.os }}-buildx-
```

## 🐛 Troubleshooting

### Issue: vcpkg dependencies fail to install

**Solution**: Check internet connectivity and vcpkg baseline version

```bash
docker build --no-cache --target vcpkg-deps .
```

### Issue: Permission denied on volumes

**Solution**: Fix permissions

```bash
docker-compose run --rm --user $(id -u):$(id -g) ci-runner
```

### Issue: X11 display not working for demos

**Solution**: Allow X11 connections

```bash
xhost +local:docker
docker-compose up demo
xhost -local:docker  # Revoke after use
```

### Issue: Build cache not working in GitHub Actions

**Solution**: Use BuildKit cache

```yaml
- name: Set up Docker Buildx
  uses: docker/setup-buildx-action@v3

- name: Build with cache
  uses: docker/build-push-action@v5
  with:
    cache-from: type=gha
    cache-to: type=gha,mode=max
```

## 📈 Performance Tips

### 1. **Use ccache**

Already configured in the Docker image. Subsequent builds will be much faster.

### 2. **Parallel Builds**

```bash
cmake --build build -j$(nproc)
```

### 3. **Incremental Builds**

Use volumes to persist build artifacts:

```bash
docker-compose run -v build-cache:/workspace/build ci-runner
```

## 🔐 Security Considerations

### 1. **Non-root User**

For production, run as non-root:

```dockerfile
RUN useradd -m -u 1000 builder
USER builder
```

### 2. **Scan for Vulnerabilities**

```bash
# Using Trivy
docker run --rm -v /var/run/docker.sock:/var/run/docker.sock \
  aquasec/trivy image hyperengine-ci:latest
```

### 3. **Minimal Image**

The final images only contain necessary tools. Build artifacts are not included in production images.

## 📚 Additional Resources

- [Docker Documentation](https://docs.docker.com/)
- [vcpkg Documentation](https://vcpkg.io/)
- [GitHub Actions Docker](https://docs.github.com/en/actions/creating-actions/creating-a-docker-container-action)
- [SpectraForge Build Instructions](../BUILD_INSTRUCTIONS.md)

---

**Version**: 1.0.0  
**Last Updated**: 2025-09-30  
**Maintainer**: TiGRoNdev
