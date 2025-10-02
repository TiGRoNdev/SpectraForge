#!/bin/bash
# Скрипт для автоматического создания GitHub Issues из TODO/FIXME маркеров
# Использует GitHub CLI и MCP интеграцию

set -e

REPO_OWNER="TiGRoNdev"
REPO_NAME="SpectraForge"
SOURCE_DIR="./src"

echo "🔍 Сканирование TODO/FIXME маркеров в проекте..."

# Цвета для вывода
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

# Счетчики
TOTAL_TODOS=0
CRITICAL_COUNT=0
HIGH_COUNT=0
MEDIUM_COUNT=0
LOW_COUNT=0

# Временный файл для TODO списка
TODO_FILE=$(mktemp)

# Функция для определения приоритета на основе контекста
get_priority() {
    local todo_text="$1"
    local file_path="$2"
    
    # CRITICAL: Блокирует функциональность
    if [[ "$todo_text" =~ (CRITICAL|BLOCKER|FIXME.*URGENT) ]]; then
        echo "critical"
    elif [[ "$file_path" =~ (VulkanRenderer|FlashGS|OptiX|ResourceManager) ]]; then
        echo "high"
    # HIGH: Влияет на производительность
    elif [[ "$todo_text" =~ (PERFORMANCE|OPTIMIZE|SLOW) ]]; then
        echo "high"
    # MEDIUM: Качество кода
    elif [[ "$todo_text" =~ (REFACTOR|IMPROVE|CLEANUP) ]]; then
        echo "medium"
    # LOW: Оптимизации
    else
        echo "low"
    fi
}

# Функция для создания GitHub Issue через gh CLI
create_github_issue() {
    local title="$1"
    local body="$2"
    local priority="$3"
    local file_path="$4"
    local line_number="$5"
    
    # Определяем метки на основе приоритета и типа
    local labels="technical-debt"
    
    case "$priority" in
        critical)
            labels="$labels,priority-critical,bug"
            ;;
        high)
            labels="$labels,priority-high,enhancement"
            ;;
        medium)
            labels="$labels,priority-medium,enhancement"
            ;;
        low)
            labels="$labels,priority-low,enhancement"
            ;;
    esac
    
    # Формируем полное описание Issue
    local full_body="## 📋 Technical Debt Item

**Файл:** \`$file_path\`  
**Строка:** $line_number  
**Приоритет:** ${priority^^}

### Описание
$body

### Контекст
Этот TODO был автоматически обнаружен в кодовой базе и требует внимания согласно правилам проекта.

### Критерии выполнения
- [ ] Код реализован согласно TODO
- [ ] Добавлены unit тесты
- [ ] Обновлена документация
- [ ] Code review пройден

---
*Создано автоматически: $(date +'%Y-%m-%d %H:%M:%S')*"
    
    # Создаем Issue через GitHub CLI
    if command -v gh &> /dev/null; then
        echo -e "${GREEN}Creating issue:${NC} $title"
        gh issue create \
            --repo "$REPO_OWNER/$REPO_NAME" \
            --title "$title" \
            --body "$full_body" \
            --label "$labels" \
            2>/dev/null || echo -e "${RED}Failed to create issue${NC}"
    else
        echo -e "${YELLOW}GitHub CLI not found, skipping issue creation${NC}"
        echo "Would create: $title"
    fi
}

# Сканируем все файлы на наличие TODO/FIXME
echo "📂 Сканирование директории: $SOURCE_DIR"

while IFS= read -r -d '' file; do
    # Пропускаем бинарные файлы и сгенерированные файлы
    if [[ "$file" =~ \.(exe|dll|so|o|a|profraw|profdata)$ ]]; then
        continue
    fi
    
    # Читаем файл построчно
    line_num=0
    while IFS= read -r line; do
        ((line_num++))
        
        # Ищем TODO или FIXME
        if [[ "$line" =~ (TODO|FIXME):?(.*) ]]; then
            ((TOTAL_TODOS++))
            
            todo_type="${BASH_REMATCH[1]}"
            todo_text="${BASH_REMATCH[2]}"
            todo_text=$(echo "$todo_text" | sed 's/^[[:space:]]*//' | sed 's/[[:space:]]*$//')
            
            # Получаем относительный путь
            rel_path="${file#./}"
            
            # Определяем приоритет
            priority=$(get_priority "$todo_text" "$rel_path")
            
            # Увеличиваем счетчик приоритета
            case "$priority" in
                critical) ((CRITICAL_COUNT++)) ;;
                high) ((HIGH_COUNT++)) ;;
                medium) ((MEDIUM_COUNT++)) ;;
                low) ((LOW_COUNT++)) ;;
            esac
            
            # Формируем заголовок Issue
            issue_title="[${priority^^}] TODO: $todo_text"
            if [[ ${#issue_title} -gt 80 ]]; then
                issue_title="${issue_title:0:77}..."
            fi
            
            # Сохраняем в временный файл
            echo "$priority|$rel_path|$line_num|$issue_title|$todo_text" >> "$TODO_FILE"
            
            # Выводим на экран
            case "$priority" in
                critical)
                    echo -e "${RED}🔴 CRITICAL${NC} $rel_path:$line_num - $todo_text"
                    ;;
                high)
                    echo -e "${YELLOW}🟠 HIGH${NC} $rel_path:$line_num - $todo_text"
                    ;;
                medium)
                    echo -e "${GREEN}🟡 MEDIUM${NC} $rel_path:$line_num - $todo_text"
                    ;;
                low)
                    echo -e "🟢 LOW $rel_path:$line_num - $todo_text"
                    ;;
            esac
        fi
    done < "$file"
done < <(find "$SOURCE_DIR" -type f \( -name "*.cpp" -o -name "*.h" -o -name "*.cu" \) -print0)

echo ""
echo "📊 Статистика TODO/FIXME:"
echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
echo -e "${RED}🔴 CRITICAL:${NC} $CRITICAL_COUNT"
echo -e "${YELLOW}🟠 HIGH:${NC}     $HIGH_COUNT"
echo -e "${GREEN}🟡 MEDIUM:${NC}   $MEDIUM_COUNT"
echo "🟢 LOW:      $LOW_COUNT"
echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
echo "📝 ВСЕГО:    $TOTAL_TODOS"
echo ""

# Спрашиваем пользователя о создании Issues
read -p "Создать GitHub Issues для всех TODO? (y/N): " -n 1 -r
echo
if [[ $REPLY =~ ^[Yy]$ ]]; then
    echo "🚀 Создание GitHub Issues..."
    
    # Сортируем по приоритету и создаем Issues
    sort -t'|' -k1,1r "$TODO_FILE" | while IFS='|' read -r priority file_path line_num title body; do
        create_github_issue "$title" "$body" "$priority" "$file_path" "$line_num"
        sleep 0.5  # Rate limiting
    done
    
    echo ""
    echo -e "${GREEN}✅ Создание Issues завершено!${NC}"
    echo "🔗 Просмотреть Issues: https://github.com/$REPO_OWNER/$REPO_NAME/issues"
else
    echo "ℹ️  Создание Issues отменено"
    echo "📄 TODO список сохранен в: $TODO_FILE"
fi

# Очистка
# rm -f "$TODO_FILE"  # Закомментировано для отладки

echo ""
echo "✅ Скрипт завершен!"
