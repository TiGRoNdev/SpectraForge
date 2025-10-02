#!/bin/bash
# Установка и настройка инструментов качества кода для SpectraForge

echo "🔧 Установка инструментов качества кода SpectraForge..."

# Определить операционную систему
OS="unknown"
if [[ "$OSTYPE" == "linux-gnu"* ]]; then
    OS="linux"
elif [[ "$OSTYPE" == "darwin"* ]]; then
    OS="macos"
elif [[ "$OSTYPE" == "msys" || "$OSTYPE" == "cygwin" ]]; then
    OS="windows"
fi

echo "🖥️ Обнаружена ОС: $OS"

# Функции для логирования
log_success() {
    echo "✅ $1"
}

log_error() {
    echo "❌ $1"
}

log_info() {
    echo "ℹ️ $1"
}

log_warning() {
    echo "⚠️ $1"
}

# 1. Установка базовых инструментов
echo ""
echo "📦 Установка базовых инструментов..."

if [ "$OS" = "linux" ]; then
    # Ubuntu/Debian
    if command -v apt-get &> /dev/null; then
        log_info "Обновление пакетов..."
        sudo apt-get update
        
        log_info "Установка инструментов разработки..."
        sudo apt-get install -y \
            build-essential \
            cmake \
            ninja-build \
            clang \
            clang-tidy \
            clang-format \
            cppcheck \
            valgrind \
            lcov \
            gcovr \
            doxygen \
            graphviz \
            flawfinder \
            python3-pip
        
        log_success "Базовые инструменты установлены"
    
    # CentOS/RHEL/Fedora
    elif command -v yum &> /dev/null || command -v dnf &> /dev/null; then
        PKG_MANAGER="yum"
        if command -v dnf &> /dev/null; then
            PKG_MANAGER="dnf"
        fi
        
        log_info "Установка инструментов разработки..."
        sudo $PKG_MANAGER install -y \
            gcc-c++ \
            cmake \
            ninja-build \
            clang \
            clang-tools-extra \
            cppcheck \
            valgrind \
            lcov \
            doxygen \
            graphviz \
            python3-pip
        
        log_success "Базовые инструменты установлены"
    fi

elif [ "$OS" = "macos" ]; then
    # macOS с Homebrew
    if command -v brew &> /dev/null; then
        log_info "Установка через Homebrew..."
        brew install \
            cmake \
            ninja \
            llvm \
            cppcheck \
            lcov \
            doxygen \
            graphviz \
            flawfinder
        
        # Добавить LLVM в PATH
        echo 'export PATH="/opt/homebrew/opt/llvm/bin:$PATH"' >> ~/.zshrc
        export PATH="/opt/homebrew/opt/llvm/bin:$PATH"
        
        log_success "Инструменты macOS установлены"
    else
        log_error "Homebrew не найден. Установите Homebrew: https://brew.sh/"
        exit 1
    fi

elif [ "$OS" = "windows" ]; then
    # Windows с Chocolatey
    if command -v choco &> /dev/null; then
        log_info "Установка через Chocolatey..."
        choco install -y \
            cmake \
            ninja \
            llvm \
            doxygen.install \
            graphviz
        
        log_success "Инструменты Windows установлены"
    else
        log_warning "Chocolatey не найден. Установите инструменты вручную или установите Chocolatey"
    fi
fi

# 2. Установка Python инструментов
echo ""
echo "🐍 Установка Python инструментов..."

if command -v pip3 &> /dev/null; then
    pip3 install --user \
        pre-commit \
        bandit \
        semgrep \
        cpplint \
        lizard \
        cmake-format
    
    log_success "Python инструменты установлены"
else
    log_error "pip3 не найден. Установите Python 3 и pip"
fi

# 3. Установка дополнительных инструментов безопасности
echo ""
echo "🔒 Установка инструментов безопасности..."

# Semgrep
if ! command -v semgrep &> /dev/null; then
    if command -v pip3 &> /dev/null; then
        pip3 install --user semgrep
        log_success "Semgrep установлен"
    fi
fi

# Flawfinder (если не установлен через пакетный менеджер)
if ! command -v flawfinder &> /dev/null; then
    if command -v pip3 &> /dev/null; then
        pip3 install --user flawfinder
        log_success "Flawfinder установлен"
    fi
fi

# 4. Настройка pre-commit
echo ""
echo "🪝 Настройка pre-commit hooks..."

if command -v pre-commit &> /dev/null; then
    # Установить pre-commit hooks
    pre-commit install
    
    # Запустить на всех файлах для проверки
    log_info "Запуск pre-commit на всех файлах..."
    pre-commit run --all-files || log_warning "Некоторые pre-commit проверки не прошли"
    
    log_success "Pre-commit hooks настроены"
else
    log_error "pre-commit не найден. Установите: pip install pre-commit"
fi

# 5. Создание конфигурационных файлов (если не существуют)
echo ""
echo "⚙️ Проверка конфигурационных файлов..."

# .clang-format
if [ ! -f ".clang-format" ]; then
    log_warning ".clang-format не найден. Создайте его для форматирования кода"
fi

# .clang-tidy
if [ ! -f ".clang-tidy" ]; then
    log_warning ".clang-tidy не найден. Создайте его для статического анализа"
fi

# cppcheck.cfg
if [ ! -f "cppcheck.cfg" ]; then
    log_info "Создание базовой конфигурации cppcheck..."
    cat > cppcheck.cfg << 'EOF'
# Конфигурация cppcheck для SpectraForge
# Подавление ложных срабатываний
missingIncludeSystem
unusedFunction
EOF
    log_success "cppcheck.cfg создан"
fi

# 6. Создание скриптов-обёрток
echo ""
echo "📜 Создание скриптов-обёрток..."

# Скрипт для быстрой проверки перед коммитом
cat > scripts/pre_commit_check.sh << 'EOF'
#!/bin/bash
# Быстрая проверка перед коммитом

echo "🚀 Быстрая проверка качества кода..."

# Форматирование
echo "🎨 Проверка форматирования..."
if ! git diff --cached --name-only | grep -E '\.(cpp|h|hpp)$' | xargs clang-format --dry-run --Werror; then
    echo "❌ Ошибки форматирования. Запустите: git diff --cached --name-only | grep -E '\.(cpp|h|hpp)$' | xargs clang-format -i"
    exit 1
fi

# Базовые проверки
echo "🔍 Базовые проверки..."
git diff --cached --name-only | grep -E '\.(cpp|h|hpp)$' | while read file; do
    # Проверка на небезопасные функции
    if grep -n "strcpy\|strcat\|sprintf\|gets" "$file"; then
        echo "❌ Найдены небезопасные функции в $file"
        exit 1
    fi
    
    # Проверка namespace
    if [ "${file##*.}" = "h" ] || [ "${file##*.}" = "hpp" ]; then
        if ! grep -q "namespace SpectraForge" "$file"; then
            echo "⚠️ Файл $file может не использовать namespace SpectraForge"
        fi
    fi
done

echo "✅ Быстрая проверка завершена успешно"
EOF

chmod +x scripts/pre_commit_check.sh
log_success "Скрипт pre_commit_check.sh создан"

# 7. Проверка установки
echo ""
echo "🧪 Проверка установленных инструментов..."

TOOLS=("cmake" "clang-format" "clang-tidy" "cppcheck" "doxygen")
MISSING_TOOLS=()

for tool in "${TOOLS[@]}"; do
    if command -v "$tool" &> /dev/null; then
        VERSION=$($tool --version 2>/dev/null | head -1 || echo "unknown")
        log_success "$tool: $VERSION"
    else
        log_error "$tool не найден"
        MISSING_TOOLS+=("$tool")
    fi
done

# Python инструменты
PYTHON_TOOLS=("pre-commit" "bandit" "semgrep")
for tool in "${PYTHON_TOOLS[@]}"; do
    if command -v "$tool" &> /dev/null; then
        VERSION=$($tool --version 2>/dev/null | head -1 || echo "unknown")
        log_success "$tool: $VERSION"
    else
        log_warning "$tool не найден (опционально)"
    fi
done

# 8. Создание документации по использованию
echo ""
echo "📚 Создание документации..."

cat > docs/guides/QUALITY_TOOLS.md << 'EOF'
# Инструменты качества кода SpectraForge

## Установленные инструменты

### Статический анализ
- **clang-tidy**: Статический анализ C++ кода
- **cppcheck**: Дополнительный статический анализ
- **flawfinder**: Анализ безопасности

### Форматирование
- **clang-format**: Автоматическое форматирование кода

### Динамический анализ
- **AddressSanitizer**: Обнаружение ошибок памяти
- **UBSanitizer**: Обнаружение неопределенного поведения
- **ThreadSanitizer**: Обнаружение гонок потоков
- **Valgrind**: Детальный анализ памяти (Linux)

### Покрытие кода
- **lcov/gcov**: Анализ покрытия кода

### Документация
- **Doxygen**: Генерация API документации

## Использование

### Ежедневная разработка
```bash
# Быстрая проверка перед коммитом
./scripts/pre_commit_check.sh

# Полная проверка качества
./scripts/quality_check.sh

# Проверка безопасности
./scripts/security_check.sh
```

### Глубокий анализ
```bash
# Статический анализ
./scripts/static_analysis.sh

# Динамический анализ
./scripts/dynamic_analysis.sh
```

### Автоматизация
- Pre-commit hooks автоматически запускаются при коммите
- CI/CD пайплайн выполняет полную проверку при push
- Отчеты сохраняются как артефакты

## Конфигурация

- `.clang-format`: Настройки форматирования
- `.clang-tidy`: Настройки статического анализа
- `.pre-commit-config.yaml`: Конфигурация pre-commit hooks
- `cppcheck.cfg`: Настройки cppcheck

## Интеграция с IDE

### Visual Studio Code
Установите расширения:
- C/C++ (Microsoft)
- clangd
- Clang-Format

### CLion
Встроенная поддержка clang-tidy и clang-format

### Visual Studio
Встроенная поддержка статического анализа
EOF

log_success "Документация создана: docs/guides/QUALITY_TOOLS.md"

# Финальный отчет
echo ""
echo "🎉 УСТАНОВКА ЗАВЕРШЕНА"
echo "====================="

if [ ${#MISSING_TOOLS[@]} -eq 0 ]; then
    echo "✅ Все основные инструменты установлены успешно!"
else
    echo "⚠️ Отсутствуют инструменты: ${MISSING_TOOLS[*]}"
    echo "Установите их вручную для полной функциональности"
fi

echo ""
echo "📋 Следующие шаги:"
echo "1. Запустите ./scripts/quality_check.sh для проверки"
echo "2. Настройте IDE для использования clang-format и clang-tidy"
echo "3. Изучите документацию: docs/guides/QUALITY_TOOLS.md"
echo ""
echo "🚀 Инструменты качества кода готовы к использованию!"
