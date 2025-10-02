#!/bin/bash
# install_nvidia_sdks.sh - Автоматическая установка NVIDIA SDK для SpectraForge
# Поддержка: CUDA Toolkit, OptiX SDK, DLSS (Streamline)

set -e  # Прервать выполнение при ошибке

# Цвета для вывода
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

# Функция логирования
log_info() {
    echo -e "${BLUE}[INFO]${NC} $1"
}

log_success() {
    echo -e "${GREEN}[SUCCESS]${NC} $1"
}

log_warning() {
    echo -e "${YELLOW}[WARNING]${NC} $1"
}

log_error() {
    echo -e "${RED}[ERROR]${NC} $1"
}

# Проверка прав root
check_root() {
    if [ "$EUID" -eq 0 ]; then
        log_error "Не запускайте этот скрипт с правами root!"
        log_info "Скрипт сам запросит sudo когда необходимо"
        exit 1
    fi
}

# Проверка ОС
check_os() {
    if [ -f /etc/os-release ]; then
        . /etc/os-release
        OS=$ID
        VER=$VERSION_ID
        log_info "Обнаружена ОС: $OS $VER"
    else
        log_error "Невозможно определить ОС"
        exit 1
    fi
}

# Определение архитектуры
check_arch() {
    ARCH=$(uname -m)
    if [ "$ARCH" != "x86_64" ]; then
        log_warning "Архитектура $ARCH может не поддерживаться. Рекомендуется x86_64"
    fi
}

# Директория установки SDK
SDK_INSTALL_DIR="${HOME}/nvidia-sdks"
CUDA_VERSION="12.8.0"
CUDA_SHORT_VERSION="12-8"
OPTIX_VERSION="7.7.0"

echo "========================================"
echo "SpectraForge NVIDIA SDK Auto-Installer"
echo "========================================"
echo

check_root
check_os
check_arch

# Создание директории для SDK
log_info "Создание директории для SDK: $SDK_INSTALL_DIR"
mkdir -p "$SDK_INSTALL_DIR"
cd "$SDK_INSTALL_DIR"

# ============================================================
# Функция 1: Установка CUDA Toolkit
# ============================================================
install_cuda() {
    log_info "===== Установка CUDA Toolkit $CUDA_VERSION ====="
    
    # Проверка существующей установки
    if command -v nvcc &> /dev/null; then
        CURRENT_CUDA=$(nvcc --version | grep "release" | sed 's/.*release //' | sed 's/,.*//')
        log_warning "CUDA уже установлена: версия $CURRENT_CUDA"
        read -p "Переустановить CUDA? (y/N): " -n 1 -r
        echo
        if [[ ! $REPLY =~ ^[Yy]$ ]]; then
            log_info "Пропуск установки CUDA"
            return 0
        fi
    fi
    
    # Определение дистрибутива и загрузка
    case "$OS" in
        ubuntu)
            log_info "Установка CUDA для Ubuntu $VER"
            
            # Удаление старых репозиториев NVIDIA
            sudo apt-get --purge remove "*cuda*" "*cublas*" "*cufft*" "*cufile*" "*curand*" \
                "*cusolver*" "*cusparse*" "*gds-tools*" "*npp*" "*nvjpeg*" "nsight*" "*nvvm*" || true
            
            # Установка ключа и репозитория CUDA
            wget https://developer.download.nvidia.com/compute/cuda/repos/ubuntu2204/x86_64/cuda-keyring_1.1-1_all.deb -O cuda-keyring.deb
            sudo dpkg -i cuda-keyring.deb
            rm cuda-keyring.deb
            
            # Обновление списка пакетов
            sudo apt-get update
            
            # Установка CUDA Toolkit
            log_info "Установка CUDA Toolkit (это может занять несколько минут)..."
            sudo apt-get install -y cuda-toolkit-${CUDA_SHORT_VERSION}
            
            # Настройка переменных окружения
            if ! grep -q "CUDA_PATH" ~/.bashrc; then
                echo "" >> ~/.bashrc
                echo "# CUDA Toolkit" >> ~/.bashrc
                echo "export CUDA_PATH=/usr/local/cuda" >> ~/.bashrc
                echo "export PATH=\$CUDA_PATH/bin:\$PATH" >> ~/.bashrc
                echo "export LD_LIBRARY_PATH=\$CUDA_PATH/lib64:\$LD_LIBRARY_PATH" >> ~/.bashrc
            fi
            
            # Применение переменных окружения
            export CUDA_PATH=/usr/local/cuda
            export PATH=$CUDA_PATH/bin:$PATH
            export LD_LIBRARY_PATH=$CUDA_PATH/lib64:$LD_LIBRARY_PATH
            
            log_success "CUDA Toolkit установлен"
            nvcc --version || log_error "CUDA установлен, но nvcc не найден. Перезапустите терминал."
            ;;
            
        fedora|rhel|centos)
            log_info "Установка CUDA для $OS $VER"
            
            # Установка репозитория
            sudo dnf config-manager --add-repo \
                https://developer.download.nvidia.com/compute/cuda/repos/rhel9/x86_64/cuda-rhel9.repo
            
            # Установка CUDA
            sudo dnf clean all
            sudo dnf -y module install nvidia-driver:latest-dkms
            sudo dnf -y install cuda-toolkit-${CUDA_SHORT_VERSION}
            
            # Настройка переменных окружения
            if ! grep -q "CUDA_PATH" ~/.bashrc; then
                echo "" >> ~/.bashrc
                echo "# CUDA Toolkit" >> ~/.bashrc
                echo "export CUDA_PATH=/usr/local/cuda" >> ~/.bashrc
                echo "export PATH=\$CUDA_PATH/bin:\$PATH" >> ~/.bashrc
                echo "export LD_LIBRARY_PATH=\$CUDA_PATH/lib64:\$LD_LIBRARY_PATH" >> ~/.bashrc
            fi
            
            export CUDA_PATH=/usr/local/cuda
            export PATH=$CUDA_PATH/bin:$PATH
            export LD_LIBRARY_PATH=$CUDA_PATH/lib64:$LD_LIBRARY_PATH
            
            log_success "CUDA Toolkit установлен"
            ;;
            
        *)
            log_error "Неподдерживаемый дистрибутив: $OS"
            log_info "Установите CUDA вручную: https://developer.nvidia.com/cuda-downloads"
            return 1
            ;;
    esac
}

# ============================================================
# Функция 2: Установка OptiX SDK
# ============================================================
install_optix() {
    log_info "===== Установка OptiX SDK $OPTIX_VERSION ====="
    
    # OptiX требует ручной загрузки с сайта NVIDIA (требуется вход)
    log_warning "OptiX SDK требует ручной загрузки с сайта NVIDIA Developer"
    log_info "1. Посетите: https://developer.nvidia.com/optix/downloads"
    log_info "2. Войдите в аккаунт NVIDIA Developer (бесплатная регистрация)"
    log_info "3. Скачайте OptiX SDK $OPTIX_VERSION для Linux"
    log_info "4. Сохраните в: $SDK_INSTALL_DIR"
    
    echo
    read -p "Вы уже скачали OptiX SDK? (y/N): " -n 1 -r
    echo
    
    if [[ $REPLY =~ ^[Yy]$ ]]; then
        # Поиск установщика OptiX
        OPTIX_INSTALLER=$(find "$SDK_INSTALL_DIR" -name "NVIDIA-OptiX-SDK-${OPTIX_VERSION}*" -type f 2>/dev/null | head -n 1)
        
        if [ -z "$OPTIX_INSTALLER" ]; then
            log_error "Установщик OptiX не найден в $SDK_INSTALL_DIR"
            log_info "Ожидаемое имя файла: NVIDIA-OptiX-SDK-${OPTIX_VERSION}-linux64-x86_64.sh"
            return 1
        fi
        
        log_info "Найден установщик: $OPTIX_INSTALLER"
        
        # Установка OptiX
        chmod +x "$OPTIX_INSTALLER"
        OPTIX_INSTALL_PATH="$SDK_INSTALL_DIR/optix-${OPTIX_VERSION}"
        
        log_info "Установка OptiX в: $OPTIX_INSTALL_PATH"
        bash "$OPTIX_INSTALLER" --skip-license --prefix="$OPTIX_INSTALL_PATH"
        
        # Настройка переменных окружения
        if ! grep -q "OptiX_ROOT_DIR" ~/.bashrc; then
            echo "" >> ~/.bashrc
            echo "# OptiX SDK" >> ~/.bashrc
            echo "export OptiX_ROOT_DIR=$OPTIX_INSTALL_PATH" >> ~/.bashrc
            echo "export OPTIX_ROOT=\$OptiX_ROOT_DIR" >> ~/.bashrc
        fi
        
        export OptiX_ROOT_DIR="$OPTIX_INSTALL_PATH"
        export OPTIX_ROOT="$OptiX_ROOT_DIR"
        
        log_success "OptiX SDK установлен: $OPTIX_INSTALL_PATH"
    else
        log_warning "Пропуск установки OptiX. Скачайте вручную позже."
        log_info "После скачивания запустите: bash $OPTIX_INSTALLER --prefix=$SDK_INSTALL_DIR/optix-${OPTIX_VERSION}"
    fi
}

# ============================================================
# Функция 3: Установка DLSS SDK (Streamline)
# ============================================================
install_dlss() {
    log_info "===== Установка DLSS SDK (Streamline) ====="
    
    log_warning "DLSS SDK (Streamline) требует ручной загрузки с сайта NVIDIA"
    log_info "1. Посетите: https://developer.nvidia.com/rtx/streamline"
    log_info "2. Войдите в аккаунт NVIDIA Developer"
    log_info "3. Скачайте Streamline SDK (последняя версия)"
    log_info "4. Извлеките архив в: $SDK_INSTALL_DIR/streamline"
    
    echo
    read -p "Вы уже скачали и извлекли Streamline SDK? (y/N): " -n 1 -r
    echo
    
    if [[ $REPLY =~ ^[Yy]$ ]]; then
        # Поиск директории Streamline
        STREAMLINE_DIR=$(find "$SDK_INSTALL_DIR" -type d -name "*streamline*" -o -name "*Streamline*" 2>/dev/null | head -n 1)
        
        if [ -z "$STREAMLINE_DIR" ]; then
            log_error "Директория Streamline не найдена в $SDK_INSTALL_DIR"
            read -p "Введите полный путь к Streamline SDK: " STREAMLINE_DIR
        fi
        
        if [ -f "$STREAMLINE_DIR/include/sl.h" ]; then
            # Настройка переменных окружения
            if ! grep -q "STREAMLINE_ROOT" ~/.bashrc; then
                echo "" >> ~/.bashrc
                echo "# DLSS SDK (Streamline)" >> ~/.bashrc
                echo "export STREAMLINE_ROOT=$STREAMLINE_DIR" >> ~/.bashrc
                echo "export DLSS_ROOT_DIR=\$STREAMLINE_ROOT" >> ~/.bashrc
            fi
            
            export STREAMLINE_ROOT="$STREAMLINE_DIR"
            export DLSS_ROOT_DIR="$STREAMLINE_ROOT"
            
            log_success "DLSS SDK (Streamline) настроен: $STREAMLINE_DIR"
        else
            log_error "Файл $STREAMLINE_DIR/include/sl.h не найден"
            log_error "Проверьте правильность пути к Streamline SDK"
        fi
    else
        log_warning "Пропуск настройки DLSS SDK"
    fi
}

# ============================================================
# Функция 4: Клонирование FidelityFX FSR
# ============================================================
install_fsr() {
    log_info "===== Установка FidelityFX Super Resolution (FSR) ====="
    
    FSR_DIR="$SDK_INSTALL_DIR/FidelityFX-FSR2"
    
    if [ -d "$FSR_DIR" ]; then
        log_warning "FidelityFX FSR уже клонирован: $FSR_DIR"
        read -p "Обновить репозиторий? (y/N): " -n 1 -r
        echo
        if [[ $REPLY =~ ^[Yy]$ ]]; then
            cd "$FSR_DIR"
            git pull
            cd "$SDK_INSTALL_DIR"
        fi
    else
        log_info "Клонирование FidelityFX FSR2..."
        git clone https://github.com/GPUOpen-Effects/FidelityFX-FSR2.git "$FSR_DIR"
        
        if [ $? -eq 0 ]; then
            log_success "FidelityFX FSR2 клонирован: $FSR_DIR"
            
            # Сборка FSR2
            log_info "Сборка FidelityFX FSR2..."
            cd "$FSR_DIR"
            
            # FSR2 использует собственную систему сборки
            if [ -f "BuildFidelityFXSDK.bat" ]; then
                log_warning "Обнаружен Windows батник. На Linux требуется ручная сборка."
                log_info "Следуйте инструкциям: https://github.com/GPUOpen-Effects/FidelityFX-FSR2"
            fi
            
            cd "$SDK_INSTALL_DIR"
        else
            log_error "Ошибка клонирования FidelityFX FSR2"
            return 1
        fi
    fi
    
    # Настройка переменных окружения
    if ! grep -q "FIDELITYFX_ROOT" ~/.bashrc; then
        echo "" >> ~/.bashrc
        echo "# FidelityFX FSR SDK" >> ~/.bashrc
        echo "export FIDELITYFX_ROOT=$FSR_DIR" >> ~/.bashrc
        echo "export FSR_ROOT_DIR=\$FIDELITYFX_ROOT" >> ~/.bashrc
    fi
    
    export FIDELITYFX_ROOT="$FSR_DIR"
    export FSR_ROOT_DIR="$FIDELITYFX_ROOT"
    
    log_success "FidelityFX FSR настроен: $FSR_DIR"
}

# ============================================================
# Главное меню
# ============================================================
show_menu() {
    echo
    echo "========================================"
    echo "Выберите компоненты для установки:"
    echo "========================================"
    echo "1) CUDA Toolkit (обязательно для CUDA/OptiX)"
    echo "2) OptiX SDK (требует CUDA)"
    echo "3) DLSS SDK / Streamline (опционально)"
    echo "4) FidelityFX FSR (опционально)"
    echo "5) Установить ВСЁ"
    echo "6) Проверка установленных SDK"
    echo "0) Выход"
    echo "========================================"
    read -p "Ваш выбор: " choice
    
    case $choice in
        1) install_cuda ;;
        2) install_optix ;;
        3) install_dlss ;;
        4) install_fsr ;;
        5)
            install_cuda
            install_optix
            install_dlss
            install_fsr
            ;;
        6) check_sdks ;;
        0) 
            log_info "Выход..."
            exit 0
            ;;
        *)
            log_error "Неверный выбор"
            show_menu
            ;;
    esac
}

# ============================================================
# Проверка установленных SDK
# ============================================================
check_sdks() {
    log_info "===== Проверка установленных SDK ====="
    echo
    
    # CUDA
    if command -v nvcc &> /dev/null; then
        log_success "CUDA Toolkit: Установлен"
        nvcc --version | grep "release"
    else
        log_error "CUDA Toolkit: НЕ найден"
    fi
    
    # OptiX
    if [ -n "$OptiX_ROOT_DIR" ] && [ -f "$OptiX_ROOT_DIR/include/optix.h" ]; then
        log_success "OptiX SDK: Установлен ($OptiX_ROOT_DIR)"
    elif [ -f "/usr/local/optix/include/optix.h" ]; then
        log_success "OptiX SDK: Установлен (/usr/local/optix)"
    else
        log_error "OptiX SDK: НЕ найден"
    fi
    
    # DLSS/Streamline
    if [ -n "$STREAMLINE_ROOT" ] && [ -f "$STREAMLINE_ROOT/include/sl.h" ]; then
        log_success "DLSS SDK: Установлен ($STREAMLINE_ROOT)"
    else
        log_warning "DLSS SDK: НЕ найден (опционально)"
    fi
    
    # FSR
    if [ -n "$FIDELITYFX_ROOT" ] && [ -d "$FIDELITYFX_ROOT/sdk" ]; then
        log_success "FidelityFX FSR: Установлен ($FIDELITYFX_ROOT)"
    else
        log_warning "FidelityFX FSR: НЕ найден (опционально)"
    fi
    
    echo
    log_info "Переменные окружения для ~/.bashrc:"
    echo "export CUDA_PATH=/usr/local/cuda"
    echo "export PATH=\$CUDA_PATH/bin:\$PATH"
    echo "export LD_LIBRARY_PATH=\$CUDA_PATH/lib64:\$LD_LIBRARY_PATH"
    [ -n "$OptiX_ROOT_DIR" ] && echo "export OptiX_ROOT_DIR=$OptiX_ROOT_DIR"
    [ -n "$STREAMLINE_ROOT" ] && echo "export STREAMLINE_ROOT=$STREAMLINE_ROOT"
    [ -n "$FIDELITYFX_ROOT" ] && echo "export FIDELITYFX_ROOT=$FIDELITYFX_ROOT"
    echo
}

# Запуск меню
show_menu

# После установки
echo
log_success "===== Установка завершена ====="
log_info "ВАЖНО: Перезапустите терминал или выполните:"
echo "source ~/.bashrc"
echo
log_info "Для сборки SpectraForge со всеми функциями:"
echo "cd /home/tigron/Documents/GITHUB/SpectraForge"
echo "cmake -B build -G Ninja \\"
echo "  -DCMAKE_TOOLCHAIN_FILE=\$HOME/vcpkg/scripts/buildsystems/vcpkg.cmake \\"
echo "  -DCMAKE_EXPORT_COMPILE_COMMANDS=ON \\"
echo "  -DBUILD_WITH_CUDA=ON \\"
echo "  -DBUILD_WITH_OPTIX=ON \\"
echo "  -DBUILD_WITH_DLSS=ON \\"
echo "  -DBUILD_WITH_FSR=ON"
echo
log_info "Для проверки SDK запустите:"
echo "./scripts/setup_sdk.sh"

