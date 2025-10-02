#!/bin/bash

# ============================================================================
# Автоматическая установка ВСЕХ зависимостей SpectraForge
# ============================================================================
# Версия: 2.0
# Дата: 2025-10-01
# Описание: Неинтерактивная установка всех компонентов
# ============================================================================

set -e  # Выход при ошибке

# Цвета для вывода
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

# Функции вывода
print_header() {
    echo -e "\n${BLUE}━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━${NC}"
    echo -e "${BLUE}  $1${NC}"
    echo -e "${BLUE}━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━${NC}\n"
}

print_success() {
    echo -e "${GREEN}✓ $1${NC}"
}

print_error() {
    echo -e "${RED}✗ $1${NC}"
}

print_warning() {
    echo -e "${YELLOW}⚠ $1${NC}"
}

print_info() {
    echo -e "${BLUE}ℹ $1${NC}"
}

# Проверка прав суперпользователя
check_sudo() {
    if [ "$EUID" -eq 0 ]; then
        print_warning "Не запускайте этот скрипт от имени root!"
        print_info "Скрипт сам запросит sudo при необходимости"
        exit 1
    fi
}

# Определение дистрибутива
detect_distro() {
    if [ -f /etc/os-release ]; then
        . /etc/os-release
        DISTRO=$ID
        DISTRO_VERSION=$VERSION_ID
    else
        print_error "Не удалось определить дистрибутив Linux"
        exit 1
    fi
    print_info "Обнаружен дистрибутив: $DISTRO $DISTRO_VERSION"
}

print_header "🚀 Автоматическая установка ВСЕХ зависимостей SpectraForge 🚀"

check_sudo
detect_distro

# ============================================================================
# 1. СИСТЕМНЫЕ ЗАВИСИМОСТИ
# ============================================================================
print_header "1/7: Установка системных зависимостей"

case $DISTRO in
    ubuntu|debian)
        print_info "Обновление списка пакетов..."
        sudo apt-get update -qq
        
        print_info "Установка основных инструментов сборки..."
        sudo DEBIAN_FRONTEND=noninteractive apt-get install -y -qq \
            build-essential \
            cmake \
            git \
            wget \
            curl \
            unzip \
            tar \
            pkg-config \
            ninja-build \
            python3 \
            python3-pip \
            libgl1-mesa-dev \
            libglfw3-dev \
            libglew-dev \
            libx11-dev \
            libxrandr-dev \
            libxinerama-dev \
            libxcursor-dev \
            libxi-dev \
            libxxf86vm-dev
        
        print_success "Системные зависимости установлены"
        ;;
        
    fedora|rhel|centos)
        sudo dnf install -y -q \
            gcc gcc-c++ cmake git wget curl unzip tar pkg-config ninja-build python3 python3-pip \
            mesa-libGL-devel glfw-devel glew-devel libX11-devel libXrandr-devel libXinerama-devel \
            libXcursor-devel libXi-devel
        print_success "Системные зависимости установлены"
        ;;
        
    arch|manjaro)
        sudo pacman -Syu --noconfirm --quiet \
            base-devel cmake git wget curl unzip tar pkg-config ninja python python-pip \
            mesa glfw-x11 glew libx11 libxrandr libxinerama libxcursor libxi
        print_success "Системные зависимости установлены"
        ;;
esac

# ============================================================================
# 2. VULKAN SDK
# ============================================================================
print_header "2/7: Установка Vulkan SDK"

if command -v vulkaninfo &> /dev/null; then
    print_success "Vulkan уже установлен"
else
    print_info "Установка Vulkan SDK..."
    
    case $DISTRO in
        ubuntu|debian)
            wget -qO - https://packages.lunarg.com/lunarg-signing-key-pub.asc | sudo apt-key add -
            sudo wget -qO /etc/apt/sources.list.d/lunarg-vulkan-1.3.280-jammy.list \
                https://packages.lunarg.com/vulkan/1.3.280/lunarg-vulkan-1.3.280-jammy.list
            sudo apt-get update -qq
            sudo DEBIAN_FRONTEND=noninteractive apt-get install -y -qq vulkan-sdk
            ;;
            
        fedora)
            sudo dnf install -y -q vulkan-loader vulkan-tools vulkan-validation-layers-devel
            ;;
            
        arch|manjaro)
            sudo pacman -S --noconfirm --quiet vulkan-devel vulkan-tools vulkan-validation-layers
            ;;
    esac
    
    print_success "Vulkan SDK установлен"
fi

# ============================================================================
# 3. CUDA TOOLKIT
# ============================================================================
print_header "3/7: Установка CUDA Toolkit"

if ! lspci | grep -i nvidia > /dev/null; then
    print_warning "NVIDIA GPU не обнаружен, пропуск CUDA"
elif command -v nvcc &> /dev/null; then
    CUDA_VERSION=$(nvcc --version | grep "release" | awk '{print $5}' | cut -d',' -f1)
    print_success "CUDA уже установлен (версия: $CUDA_VERSION)"
else
    print_info "Установка CUDA Toolkit..."
    
    case $DISTRO in
        ubuntu)
            wget https://developer.download.nvidia.com/compute/cuda/repos/ubuntu2204/x86_64/cuda-keyring_1.1-1_all.deb
            sudo dpkg -i cuda-keyring_1.1-1_all.deb
            sudo apt-get update -qq
            sudo DEBIAN_FRONTEND=noninteractive apt-get install -y -qq cuda-toolkit-12-3
            rm cuda-keyring_1.1-1_all.deb
            
            echo 'export PATH=/usr/local/cuda/bin:$PATH' >> ~/.bashrc
            echo 'export LD_LIBRARY_PATH=/usr/local/cuda/lib64:$LD_LIBRARY_PATH' >> ~/.bashrc
            
            print_success "CUDA Toolkit установлен"
            ;;
            
        *)
            print_warning "Автоматическая установка CUDA не поддерживается для $DISTRO"
            print_info "Установите вручную с https://developer.nvidia.com/cuda-downloads"
            ;;
    esac
fi

# ============================================================================
# 4. OPTIX SDK
# ============================================================================
print_header "4/7: OptiX SDK (требует ручной установки)"

print_warning "OptiX SDK требует регистрации в NVIDIA Developer Program"
print_info "Посетите: https://developer.nvidia.com/optix"
print_info "После скачивания установите переменную: export OPTIX_ROOT=/path/to/optix"

# ============================================================================
# 5. DLSS/STREAMLINE SDK
# ============================================================================
print_header "5/7: DLSS/Streamline SDK (требует ручной установки)"

print_warning "DLSS SDK требует регистрации в NVIDIA Developer Program"
print_info "Посетите: https://developer.nvidia.com/rtx/streamline"
print_info "После скачивания установите переменную: export STREAMLINE_ROOT=/path/to/streamline"

# ============================================================================
# 6. FSR SDK
# ============================================================================
print_header "6/7: Установка AMD FSR SDK"

if [ -d "$HOME/FidelityFX-FSR2" ]; then
    print_success "FSR SDK уже установлен"
else
    print_info "Клонирование FSR SDK..."
    cd "$HOME"
    git clone --quiet --recursive https://github.com/GPUOpen-Effects/FidelityFX-FSR2.git
    
    echo "export FSR_ROOT_DIR=$HOME/FidelityFX-FSR2" >> ~/.bashrc
    echo "export FIDELITYFX_ROOT=$HOME/FidelityFX-FSR2" >> ~/.bashrc
    
    print_success "FSR SDK установлен"
fi

# ============================================================================
# 7. VCPKG И ЗАВИСИМОСТИ
# ============================================================================
print_header "7/7: Установка vcpkg и зависимостей проекта"

if [ ! -d "$HOME/vcpkg" ]; then
    print_info "Клонирование vcpkg..."
    cd "$HOME"
    git clone --quiet https://github.com/Microsoft/vcpkg.git
    cd vcpkg
    ./bootstrap-vcpkg.sh
    
    echo "export VCPKG_ROOT=$HOME/vcpkg" >> ~/.bashrc
    echo 'export PATH=$VCPKG_ROOT:$PATH' >> ~/.bashrc
    
    print_success "vcpkg установлен"
else
    print_success "vcpkg уже установлен"
fi

print_info "Установка зависимостей проекта через vcpkg..."
VCPKG_ROOT="$HOME/vcpkg"
PROJECT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"

cd "$PROJECT_DIR"

$VCPKG_ROOT/vcpkg install \
    glfw3 \
    glew \
    vulkan \
    vulkan-memory-allocator \
    vulkan-hpp \
    spirv-cross \
    shaderc \
    imgui[vulkan-binding] \
    assimp \
    stb \
    glm \
    gtest

print_success "Зависимости vcpkg установлены"

# ============================================================================
# ПРОВЕРКА УСТАНОВКИ
# ============================================================================
print_header "Проверка установленных зависимостей"

# Проверка CMake
if command -v cmake &> /dev/null; then
    CMAKE_VERSION=$(cmake --version | head -1 | awk '{print $3}')
    print_success "CMake: $CMAKE_VERSION"
else
    print_error "CMake не установлен"
fi

# Проверка Vulkan
if command -v vulkaninfo &> /dev/null; then
    print_success "Vulkan SDK: установлен"
else
    print_warning "Vulkan SDK: не установлен"
fi

# Проверка CUDA
if command -v nvcc &> /dev/null; then
    CUDA_VERSION=$(nvcc --version | grep "release" | awk '{print $5}' | cut -d',' -f1)
    print_success "CUDA Toolkit: $CUDA_VERSION"
else
    print_warning "CUDA Toolkit: не установлен (опционально)"
fi

# Проверка vcpkg
if [ -d "$HOME/vcpkg" ]; then
    print_success "vcpkg: установлен"
else
    print_error "vcpkg: не установлен"
fi

# Проверка FSR
if [ -d "$HOME/FidelityFX-FSR2" ]; then
    print_success "FSR SDK: установлен"
else
    print_warning "FSR SDK: не установлен"
fi

# ============================================================================
# ФИНАЛЬНЫЕ ИНСТРУКЦИИ
# ============================================================================
print_header "✅ Установка завершена!"

echo -e "\n${YELLOW}Следующие шаги:${NC}"
echo "1. Перезапустите терминал или выполните: source ~/.bashrc"
echo "2. Для OptiX и DLSS: скачайте SDK вручную и установите переменные окружения"
echo "3. Соберите проект:"
echo "   cd /home/tigron/Documents/GITHUB/SpectraForge"
echo "   mkdir -p build && cd build"
echo "   cmake .. -DCMAKE_TOOLCHAIN_FILE=\$HOME/vcpkg/scripts/buildsystems/vcpkg.cmake -GNinja"
echo "   cmake --build . --config Release -j\$(nproc)"
echo ""
echo -e "${GREEN}Готово! 🎉${NC}\n"

