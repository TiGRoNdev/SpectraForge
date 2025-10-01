#!/bin/bash

# ============================================================================
# Скрипт установки всех зависимостей HyperEngine
# ============================================================================
# Версия: 2.0
# Дата: 2025-10-01
# Описание: Автоматическая установка Vulkan, CUDA, OptiX, DLSS, FSR и других
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

# ============================================================================
# 1. УСТАНОВКА СИСТЕМНЫХ ЗАВИСИМОСТЕЙ
# ============================================================================
install_system_deps() {
    print_header "Установка системных зависимостей"
    
    case $DISTRO in
        ubuntu|debian)
            print_info "Обновление списка пакетов..."
            sudo apt-get update
            
            print_info "Установка основных инструментов сборки..."
            sudo apt-get install -y \
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
                python3-pip
            
            print_info "Установка библиотек разработки..."
            sudo apt-get install -y \
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
            print_info "Обновление списка пакетов..."
            sudo dnf update -y
            
            print_info "Установка основных инструментов сборки..."
            sudo dnf install -y \
                gcc \
                gcc-c++ \
                cmake \
                git \
                wget \
                curl \
                unzip \
                tar \
                pkg-config \
                ninja-build \
                python3 \
                python3-pip
            
            print_info "Установка библиотек разработки..."
            sudo dnf install -y \
                mesa-libGL-devel \
                glfw-devel \
                glew-devel \
                libX11-devel \
                libXrandr-devel \
                libXinerama-devel \
                libXcursor-devel \
                libXi-devel
            
            print_success "Системные зависимости установлены"
            ;;
            
        arch|manjaro)
            print_info "Обновление списка пакетов..."
            sudo pacman -Syu --noconfirm
            
            print_info "Установка основных инструментов сборки..."
            sudo pacman -S --noconfirm \
                base-devel \
                cmake \
                git \
                wget \
                curl \
                unzip \
                tar \
                pkg-config \
                ninja \
                python \
                python-pip
            
            print_info "Установка библиотек разработки..."
            sudo pacman -S --noconfirm \
                mesa \
                glfw-x11 \
                glew \
                libx11 \
                libxrandr \
                libxinerama \
                libxcursor \
                libxi
            
            print_success "Системные зависимости установлены"
            ;;
            
        *)
            print_warning "Дистрибутив $DISTRO не поддерживается автоматически"
            print_info "Установите вручную: cmake, git, build-essential, OpenGL dev libs"
            ;;
    esac
}

# ============================================================================
# 2. УСТАНОВКА VULKAN SDK
# ============================================================================
install_vulkan() {
    print_header "Установка Vulkan SDK"
    
    # Проверка наличия Vulkan
    if command -v vulkaninfo &> /dev/null; then
        VULKAN_VERSION=$(vulkaninfo --summary 2>&1 | grep "Vulkan Instance Version" | awk '{print $4}')
        print_success "Vulkan уже установлен (версия: $VULKAN_VERSION)"
        return 0
    fi
    
    print_info "Загрузка и установка Vulkan SDK..."
    
    # Создание временной директории
    VULKAN_TEMP_DIR=$(mktemp -d)
    cd "$VULKAN_TEMP_DIR"
    
    # Определение последней версии Vulkan SDK
    VULKAN_VERSION="1.3.280"
    
    case $DISTRO in
        ubuntu|debian)
            # Добавление репозитория LunarG
            wget -qO - https://packages.lunarg.com/lunarg-signing-key-pub.asc | sudo apt-key add -
            sudo wget -qO /etc/apt/sources.list.d/lunarg-vulkan-${VULKAN_VERSION}-jammy.list \
                https://packages.lunarg.com/vulkan/${VULKAN_VERSION}/lunarg-vulkan-${VULKAN_VERSION}-jammy.list
            
            sudo apt-get update
            sudo apt-get install -y vulkan-sdk
            ;;
            
        fedora)
            sudo dnf install -y vulkan-loader vulkan-tools vulkan-validation-layers-devel
            ;;
            
        arch|manjaro)
            sudo pacman -S --noconfirm vulkan-devel vulkan-tools vulkan-validation-layers
            ;;
    esac
    
    # Очистка
    cd -
    rm -rf "$VULKAN_TEMP_DIR"
    
    # Проверка установки
    if command -v vulkaninfo &> /dev/null; then
        print_success "Vulkan SDK успешно установлен"
        vulkaninfo --summary 2>&1 | head -20
    else
        print_error "Не удалось установить Vulkan SDK"
        return 1
    fi
}

# ============================================================================
# 3. УСТАНОВКА CUDA TOOLKIT
# ============================================================================
install_cuda() {
    print_header "Установка NVIDIA CUDA Toolkit"
    
    # Проверка наличия NVIDIA GPU
    if ! lspci | grep -i nvidia > /dev/null; then
        print_warning "NVIDIA GPU не обнаружен, пропуск установки CUDA"
        return 0
    fi
    
    # Проверка наличия CUDA
    if command -v nvcc &> /dev/null; then
        CUDA_VERSION=$(nvcc --version | grep "release" | awk '{print $5}' | cut -d',' -f1)
        print_success "CUDA уже установлен (версия: $CUDA_VERSION)"
        return 0
    fi
    
    print_info "Загрузка и установка CUDA Toolkit..."
    
    # Версия CUDA для установки
    CUDA_VERSION_MAJOR="12"
    CUDA_VERSION_MINOR="3"
    
    case $DISTRO in
        ubuntu)
            # Определение кодового имени Ubuntu
            UBUNTU_CODENAME=$(lsb_release -cs)
            
            print_info "Загрузка CUDA для Ubuntu ${UBUNTU_CODENAME}..."
            
            # Скачивание установщика
            wget https://developer.download.nvidia.com/compute/cuda/repos/ubuntu2204/x86_64/cuda-keyring_1.1-1_all.deb
            sudo dpkg -i cuda-keyring_1.1-1_all.deb
            sudo apt-get update
            sudo apt-get install -y cuda-toolkit-${CUDA_VERSION_MAJOR}-${CUDA_VERSION_MINOR}
            
            # Добавление в PATH
            echo 'export PATH=/usr/local/cuda/bin:$PATH' >> ~/.bashrc
            echo 'export LD_LIBRARY_PATH=/usr/local/cuda/lib64:$LD_LIBRARY_PATH' >> ~/.bashrc
            
            print_success "CUDA Toolkit установлен"
            print_warning "Перезапустите терминал или выполните: source ~/.bashrc"
            ;;
            
        fedora)
            print_info "Установка CUDA для Fedora..."
            sudo dnf config-manager --add-repo \
                https://developer.download.nvidia.com/compute/cuda/repos/fedora37/x86_64/cuda-fedora37.repo
            sudo dnf install -y cuda-toolkit-${CUDA_VERSION_MAJOR}-${CUDA_VERSION_MINOR}
            ;;
            
        *)
            print_warning "Автоматическая установка CUDA не поддерживается для $DISTRO"
            print_info "Скачайте CUDA вручную с: https://developer.nvidia.com/cuda-downloads"
            ;;
    esac
}

# ============================================================================
# 4. УСТАНОВКА NVIDIA OptiX SDK
# ============================================================================
install_optix() {
    print_header "Установка NVIDIA OptiX SDK"
    
    # Проверка наличия CUDA (OptiX требует CUDA)
    if ! command -v nvcc &> /dev/null; then
        print_warning "OptiX требует CUDA. Сначала установите CUDA."
        return 0
    fi
    
    # Проверка наличия OptiX
    if [ -d "/opt/optix" ] || [ -d "$HOME/optix" ]; then
        print_success "OptiX SDK уже установлен"
        return 0
    fi
    
    print_warning "OptiX SDK требует регистрации в NVIDIA Developer Program"
    print_info "Посетите: https://developer.nvidia.com/optix"
    print_info "После скачивания OptiX SDK:"
    print_info "  1. Распакуйте архив"
    print_info "  2. Установите переменную окружения: export OPTIX_ROOT=/path/to/optix"
    print_info "  3. Или переместите в /opt/optix"
    
    read -p "У вас есть OptiX SDK для установки? (y/n) " -n 1 -r
    echo
    if [[ $REPLY =~ ^[Yy]$ ]]; then
        read -p "Введите путь к архиву OptiX SDK: " OPTIX_ARCHIVE
        if [ -f "$OPTIX_ARCHIVE" ]; then
            print_info "Распаковка OptiX SDK..."
            OPTIX_DIR="$HOME/optix"
            mkdir -p "$OPTIX_DIR"
            
            # Определение типа архива и распаковка
            if [[ "$OPTIX_ARCHIVE" == *.tar.gz ]]; then
                tar -xzf "$OPTIX_ARCHIVE" -C "$OPTIX_DIR" --strip-components=1
            elif [[ "$OPTIX_ARCHIVE" == *.sh ]]; then
                bash "$OPTIX_ARCHIVE" --skip-license --prefix="$OPTIX_DIR"
            else
                print_error "Неподдерживаемый формат архива"
                return 1
            fi
            
            # Добавление в переменные окружения
            echo "export OPTIX_ROOT=$OPTIX_DIR" >> ~/.bashrc
            
            print_success "OptiX SDK установлен в $OPTIX_DIR"
            print_warning "Перезапустите терминал или выполните: source ~/.bashrc"
        else
            print_error "Файл не найден: $OPTIX_ARCHIVE"
        fi
    fi
}

# ============================================================================
# 5. УСТАНОВКА NVIDIA DLSS SDK (Streamline)
# ============================================================================
install_dlss() {
    print_header "Установка NVIDIA DLSS SDK (Streamline)"
    
    # Проверка наличия NVIDIA RTX GPU
    if ! lspci | grep -i "nvidia.*rtx" > /dev/null; then
        print_warning "DLSS требует NVIDIA RTX GPU, пропуск установки"
        return 0
    fi
    
    print_warning "DLSS SDK (Streamline) требует регистрации в NVIDIA Developer Program"
    print_info "Посетите: https://developer.nvidia.com/rtx/streamline"
    print_info "После скачивания Streamline SDK:"
    print_info "  1. Распакуйте архив"
    print_info "  2. Установите переменную окружения: export STREAMLINE_ROOT=/path/to/streamline"
    
    read -p "У вас есть Streamline SDK для установки? (y/n) " -n 1 -r
    echo
    if [[ $REPLY =~ ^[Yy]$ ]]; then
        read -p "Введите путь к архиву Streamline SDK: " STREAMLINE_ARCHIVE
        if [ -f "$STREAMLINE_ARCHIVE" ]; then
            print_info "Распаковка Streamline SDK..."
            STREAMLINE_DIR="$HOME/streamline"
            mkdir -p "$STREAMLINE_DIR"
            
            unzip -q "$STREAMLINE_ARCHIVE" -d "$STREAMLINE_DIR"
            
            echo "export STREAMLINE_ROOT=$STREAMLINE_DIR" >> ~/.bashrc
            echo "export DLSS_ROOT_DIR=$STREAMLINE_DIR" >> ~/.bashrc
            
            print_success "Streamline SDK установлен в $STREAMLINE_DIR"
            print_warning "Перезапустите терминал или выполните: source ~/.bashrc"
        else
            print_error "Файл не найден: $STREAMLINE_ARCHIVE"
        fi
    fi
}

# ============================================================================
# 6. УСТАНОВКА AMD FSR SDK
# ============================================================================
install_fsr() {
    print_header "Установка AMD FidelityFX Super Resolution SDK"
    
    # Проверка наличия FSR
    if [ -d "$HOME/FidelityFX-FSR2" ]; then
        print_success "FSR SDK уже установлен"
        return 0
    fi
    
    print_info "Клонирование FidelityFX FSR2 SDK..."
    
    cd "$HOME"
    git clone --recursive https://github.com/GPUOpen-Effects/FidelityFX-FSR2.git
    
    if [ -d "FidelityFX-FSR2" ]; then
        echo "export FSR_ROOT_DIR=$HOME/FidelityFX-FSR2" >> ~/.bashrc
        echo "export FIDELITYFX_ROOT=$HOME/FidelityFX-FSR2" >> ~/.bashrc
        
        print_success "FSR SDK установлен в $HOME/FidelityFX-FSR2"
        print_warning "Перезапустите терминал или выполните: source ~/.bashrc"
    else
        print_error "Не удалось клонировать FSR SDK"
        return 1
    fi
}

# ============================================================================
# 7. УСТАНОВКА VCPKG И ЗАВИСИМОСТЕЙ
# ============================================================================
install_vcpkg() {
    print_header "Установка vcpkg и зависимостей проекта"
    
    # Проверка наличия vcpkg
    if [ ! -d "$HOME/vcpkg" ]; then
        print_info "Клонирование vcpkg..."
        cd "$HOME"
        git clone https://github.com/Microsoft/vcpkg.git
        cd vcpkg
        ./bootstrap-vcpkg.sh
        
        # Добавление в PATH
        echo "export VCPKG_ROOT=$HOME/vcpkg" >> ~/.bashrc
        echo 'export PATH=$VCPKG_ROOT:$PATH' >> ~/.bashrc
        
        print_success "vcpkg установлен"
    else
        print_success "vcpkg уже установлен"
    fi
    
    # Установка зависимостей из vcpkg.json
    print_info "Установка зависимостей проекта через vcpkg..."
    
    VCPKG_ROOT="$HOME/vcpkg"
    PROJECT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
    
    cd "$PROJECT_DIR"
    
    # Установка зависимостей
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
}

# ============================================================================
# 8. ПРОВЕРКА УСТАНОВКИ
# ============================================================================
verify_installation() {
    print_header "Проверка установленных зависимостей"
    
    # Проверка CMake
    if command -v cmake &> /dev/null; then
        CMAKE_VERSION=$(cmake --version | head -1 | awk '{print $3}')
        print_success "CMake: $CMAKE_VERSION"
    else
        print_error "CMake не установлен"
    fi
    
    # Проверка Git
    if command -v git &> /dev/null; then
        GIT_VERSION=$(git --version | awk '{print $3}')
        print_success "Git: $GIT_VERSION"
    else
        print_error "Git не установлен"
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
    
    # Проверка OptiX
    if [ -n "$OPTIX_ROOT" ] && [ -d "$OPTIX_ROOT" ]; then
        print_success "OptiX SDK: установлен"
    else
        print_warning "OptiX SDK: не установлен (опционально)"
    fi
    
    # Проверка DLSS/Streamline
    if [ -n "$STREAMLINE_ROOT" ] && [ -d "$STREAMLINE_ROOT" ]; then
        print_success "DLSS/Streamline SDK: установлен"
    else
        print_warning "DLSS/Streamline SDK: не установлен (опционально)"
    fi
    
    # Проверка FSR
    if [ -n "$FSR_ROOT_DIR" ] && [ -d "$FSR_ROOT_DIR" ]; then
        print_success "FSR SDK: установлен"
    else
        print_warning "FSR SDK: не установлен (опционально)"
    fi
    
    # Проверка vcpkg
    if [ -d "$HOME/vcpkg" ]; then
        print_success "vcpkg: установлен"
    else
        print_error "vcpkg: не установлен"
    fi
}

# ============================================================================
# ГЛАВНАЯ ФУНКЦИЯ
# ============================================================================
main() {
    print_header "🚀 Установка зависимостей HyperEngine 🚀"
    
    # Проверки
    check_sudo
    detect_distro
    
    # Вопрос о компонентах
    echo -e "\n${BLUE}Выберите компоненты для установки:${NC}"
    echo "1. Системные зависимости (обязательно)"
    echo "2. Vulkan SDK (обязательно)"
    echo "3. CUDA Toolkit (опционально, для NVIDIA GPU)"
    echo "4. OptiX SDK (опционально, требует CUDA)"
    echo "5. DLSS/Streamline SDK (опционально, требует RTX GPU)"
    echo "6. FSR SDK (опционально)"
    echo "7. vcpkg и зависимости проекта (обязательно)"
    echo "A. Установить всё"
    echo ""
    read -p "Введите номера через пробел или 'A' для установки всего: " COMPONENTS
    
    # Установка компонентов
    if [[ $COMPONENTS == *"A"* ]] || [[ $COMPONENTS == *"a"* ]]; then
        install_system_deps
        install_vulkan
        install_cuda
        install_optix
        install_dlss
        install_fsr
        install_vcpkg
    else
        [[ $COMPONENTS == *"1"* ]] && install_system_deps
        [[ $COMPONENTS == *"2"* ]] && install_vulkan
        [[ $COMPONENTS == *"3"* ]] && install_cuda
        [[ $COMPONENTS == *"4"* ]] && install_optix
        [[ $COMPONENTS == *"5"* ]] && install_dlss
        [[ $COMPONENTS == *"6"* ]] && install_fsr
        [[ $COMPONENTS == *"7"* ]] && install_vcpkg
    fi
    
    # Проверка установки
    verify_installation
    
    # Финальные инструкции
    print_header "✅ Установка завершена!"
    
    echo -e "\n${YELLOW}Следующие шаги:${NC}"
    echo "1. Перезапустите терминал или выполните: source ~/.bashrc"
    echo "2. Перейдите в директорию проекта HyperEngine"
    echo "3. Выполните сборку:"
    echo "   mkdir -p build && cd build"
    echo "   cmake .. -DCMAKE_TOOLCHAIN_FILE=\$HOME/vcpkg/scripts/buildsystems/vcpkg.cmake"
    echo "   cmake --build . --config Release -j\$(nproc)"
    echo ""
    echo -e "${GREEN}Готово! 🎉${NC}\n"
}

# Запуск главной функции
main "$@"

