# 🐳 Docker Quick Start Guide for HyperEngine

## 📋 Prerequisites

- Docker 20.10+ installed ([Install Docker](https://docs.docker.com/get-docker/))
- Docker Compose 2.0+ ([Install Docker Compose](https://docs.docker.com/compose/install/))
- 10GB+ free disk space
- (Optional) X11 server for GUI demos

## 🚀 Quick Start Commands

### 1. Build CI/CD Image (Recommended for Development)

```bash
# Build the CI/CD runner image
docker-compose build ci-runner

# Or use docker directly
docker build --target ci-runner -t hyperengine-ci:latest .
```

### 2. Run Quality Checks

```bash
# Using docker-compose
docker-compose run --rm ci-runner bash -c "./scripts/quality_check.sh"

# Or using docker directly
docker run --rm -v $(pwd):/workspace hyperengine-ci:latest \
  bash -c "cd /workspace && ./scripts/quality_check.sh"
```

### 3. Interactive Development

```bash
# Start development environment
docker-compose up -d dev

# Access the container
docker exec -it hyperengine-dev bash

# Inside the container, you can:
# - Build: cmake --build build -j$(nproc)
# - Test: cd build && ctest --output-on-failure
# - Format: clang-format -i src/**/*.cpp
```

### 4. Build Project

```bash
# Build using builder service
docker-compose build builder

# Or run a quick build
docker-compose run --rm ci-runner bash -c "
  cmake -B build -S . \
    -G Ninja \
    -DCMAKE_BUILD_TYPE=Release \
    -DCMAKE_TOOLCHAIN_FILE=/opt/vcpkg/scripts/buildsystems/vcpkg.cmake \
    -DBUILD_TESTS=ON
  cmake --build build -j\$(nproc)
"
```

### 5. Run Tests

```bash
docker-compose run --rm ci-runner bash -c "
  cd build && ctest --output-on-failure --parallel \$(nproc)
"
```

### 6. Run Demo (with GUI)

```bash
# Allow X11 connections (Linux only)
xhost +local:docker

# Run the demo
docker-compose up demo

# Revoke X11 access after use
xhost -local:docker
```

## 🔍 Available Services

| Service | Purpose | Command |
|---------|---------|---------|
| `ci-runner` | Code quality checks, analysis | `docker-compose run --rm ci-runner` |
| `builder` | Full build with tests | `docker-compose up builder` |
| `dev` | Interactive development | `docker-compose up -d dev` |
| `demo` | GUI demo execution | `docker-compose up demo` |

## 📊 Common Tasks

### Format Code

```bash
docker-compose run --rm ci-runner bash -c "
  find src include tests -name '*.cpp' -o -name '*.h' | \
  xargs clang-format -i
"
```

### Static Analysis

```bash
# clang-tidy
docker-compose run --rm ci-runner bash -c "
  cmake -B build -DCMAKE_EXPORT_COMPILE_COMMANDS=ON
  find src -name '*.cpp' | xargs clang-tidy --config-file=.clang-tidy -p build
"

# cppcheck
docker-compose run --rm ci-runner bash -c "
  cppcheck --enable=all --std=c++20 \
    --xml --xml-version=2 \
    --output-file=cppcheck-results.xml \
    src/ include/
"
```

### Memory Leak Check

```bash
docker-compose run --rm ci-runner bash -c "
  cmake -B build -DCMAKE_BUILD_TYPE=Debug
  cmake --build build
  valgrind --leak-check=full ./build/tests/unit_tests
"
```

### Code Coverage

```bash
docker-compose run --rm ci-runner bash -c "
  cmake -B build -DCMAKE_BUILD_TYPE=Debug -DENABLE_COVERAGE=ON
  cmake --build build
  cd build && ctest
  gcovr --xml --output coverage.xml
"
```

## 🛠️ Environment Variables

You can customize the build by setting environment variables:

```bash
# Example: Build with custom configuration
docker-compose run --rm \
  -e CMAKE_BUILD_TYPE=Debug \
  -e ENABLE_ASAN=ON \
  ci-runner bash -c "
    cmake -B build -DCMAKE_BUILD_TYPE=\$CMAKE_BUILD_TYPE
    cmake --build build
  "
```

## 🔧 Troubleshooting

### Problem: "Permission denied" errors

**Solution**: Run with your user ID

```bash
docker-compose run --rm --user $(id -u):$(id -g) ci-runner
```

### Problem: vcpkg download fails

**Solution**: Check internet connection or rebuild without cache

```bash
docker-compose build --no-cache ci-runner
```

### Problem: "Cannot connect to X server"

**Solution**: Allow X11 connections

```bash
xhost +local:docker
# Run your GUI application
docker-compose up demo
# Revoke access
xhost -local:docker
```

### Problem: Slow builds

**Solution**: Use ccache volume (already configured)

```bash
# Check ccache stats
docker-compose run --rm ci-runner bash -c "ccache -s"

# Clear ccache if needed
docker volume rm hyperengine_ccache
```

### Problem: Out of disk space

**Solution**: Clean Docker resources

```bash
# Remove unused containers, networks, images
docker system prune -a

# Remove all project volumes
docker-compose down -v
```

## 📈 Performance Tips

### 1. Use Build Cache

The Docker image is optimized with layer caching. Dependencies are cached separately from source code.

### 2. Parallel Builds

Always use `-j$(nproc)` for parallel compilation:

```bash
cmake --build build -j$(nproc)
```

### 3. ccache

The container has ccache pre-configured. Subsequent builds will be much faster.

### 4. Named Volumes

Use named volumes to persist build artifacts:

```yaml
volumes:
  - build-cache:/workspace/build
```

## 🔐 Security Best Practices

### 1. Don't run as root in production

Modify the Dockerfile to add a non-root user:

```dockerfile
RUN useradd -m -u 1000 builder
USER builder
```

### 2. Scan for vulnerabilities

```bash
# Using Trivy
docker run --rm -v /var/run/docker.sock:/var/run/docker.sock \
  aquasec/trivy image hyperengine-ci:latest
```

### 3. Keep images updated

```bash
# Rebuild with latest base image
docker-compose build --pull --no-cache
```

## 📚 Additional Commands

### Clean Everything

```bash
# Stop all containers
docker-compose down

# Remove all volumes
docker-compose down -v

# Remove images
docker-compose down --rmi all

# Full cleanup
docker system prune -a --volumes
```

### View Logs

```bash
# View logs from all services
docker-compose logs

# Follow logs for specific service
docker-compose logs -f ci-runner
```

### List Running Containers

```bash
docker-compose ps
```

### Enter Running Container

```bash
docker exec -it hyperengine-dev bash
```

## 🎯 GitHub Actions Integration

The Docker setup is integrated with GitHub Actions. See `.github/workflows/docker-ci.yml` for the full workflow.

**Key features:**
- ✅ Automatic image building with layer caching
- ✅ Quality checks in isolated environment
- ✅ Parallel build and test matrix
- ✅ Artifact uploads for reports
- ✅ Container registry publishing

## 📖 Further Reading

- [Full Docker CI/CD Guide](docs/DOCKER_CICD_GUIDE.md)
- [Build Instructions](BUILD_INSTRUCTIONS.md)
- [Development Guide](docs/DEVELOPER_GUIDE.md)

## 🤝 Contributing

When contributing, please ensure:

1. All Docker builds pass: `docker-compose build`
2. Quality checks pass: `docker-compose run --rm ci-runner ./scripts/quality_check.sh`
3. Tests pass: Run tests in container before submitting PR

## ❓ Need Help?

- **Documentation**: Check [docs/DOCKER_CICD_GUIDE.md](docs/DOCKER_CICD_GUIDE.md)
- **Issues**: Open an issue on [GitHub](https://github.com/TiGRoNdev/HyperEngine/issues)
- **Discussions**: Join [GitHub Discussions](https://github.com/TiGRoNdev/HyperEngine/discussions)

---

**Version**: 1.0.0  
**Last Updated**: 2025-09-30  
**Maintainer**: [TiGRoNdev](https://github.com/TiGRoNdev)
