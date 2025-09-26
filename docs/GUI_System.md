# GUI System - 4D Engine

## Обзор

GUI система Engine4D предоставляет полнофункциональный графический интерфейс для создания пользовательских интерфейсов в 4D приложениях. Система построена на принципах компонентной архитектуры и интегрирована с основной системой GameObject4D.

## Архитектура

### Основные компоненты

1. **UIElement** - Базовый класс для всех GUI элементов
2. **UICanvas** - Контейнер и менеджер для GUI элементов
3. **UIRenderer** - Специализированный рендерер для 2D GUI
4. **GUI Components** - Готовые компоненты интерфейса

### Иерархия классов

```
Component4D
└── UIElement
    ├── UIPanel
    ├── UIButton
    ├── UILabel
    ├── UIImage
    ├── UIScrollbar
    ├── UISlider
    └── UITextField
```

## Базовые концепции

### UIElement

Базовый класс для всех GUI элементов, предоставляющий:

- Позиционирование и размеры
- Систему якорей (anchoring)
- Иерархию родитель-ребенок
- Систему событий
- Стили и внешний вид

```cpp
auto element = std::make_shared<UIPanel>();
element->setRect(UIRect(100, 50, 200, 150));
element->setAnchor(AnchorType::TopLeft);
element->setVisible(true);
element->setInteractable(true);
```

### UICanvas

Главный контейнер для всех GUI элементов:

```cpp
auto canvasObject = GameObject4D::create("UI Canvas");
auto canvas = canvasObject->addComponent<UICanvas>();
canvas->setReferenceResolution(1920, 1080);
canvas->setScaleMode(CanvasScaleMode::ScaleWithScreenSize);
```

### Система якорей

GUI элементы поддерживают различные типы якорей:

- `TopLeft`, `TopCenter`, `TopRight`
- `MiddleLeft`, `MiddleCenter`, `MiddleRight`
- `BottomLeft`, `BottomCenter`, `BottomRight`
- `Stretch` - растягивание по всему родителю
- `Custom` - пользовательские якоря

```cpp
element->setAnchor(AnchorType::MiddleCenter);
// или
element->setAnchor(0.5f, 0.5f, 0.5f, 0.5f, 0, 0); // Custom anchor
```

## GUI Компоненты

### UIPanel

Простая панель с фоном и границами:

```cpp
auto panel = std::make_shared<UIPanel>();
panel->setBackgroundColor(Vector4(0.2f, 0.3f, 0.5f, 0.8f));
panel->setBorderWidth(2.0f);
panel->setBorderColor(Vector4(0.1f, 0.2f, 0.4f, 1.0f));
panel->setCornerRadius(10.0f);
```

### UIButton

Интерактивная кнопка с различными состояниями:

```cpp
auto button = std::make_shared<UIButton>();
button->setText("Нажми меня!");
button->setNormalColor(Vector4(0.6f, 0.8f, 1.0f, 1.0f));
button->setHoverColor(Vector4(0.7f, 0.9f, 1.0f, 1.0f));
button->setPressedColor(Vector4(0.5f, 0.7f, 0.9f, 1.0f));
button->setOnClick([]() {
    std::cout << "Кнопка нажата!" << std::endl;
});
```

### UILabel

Текстовая метка:

```cpp
auto label = std::make_shared<UILabel>();
label->setText("Привет, мир!");
label->setTextColor(Vector4(1.0f, 1.0f, 1.0f, 1.0f));
label->setAlignment(UILabel::TextAlignment::Center);
label->setAutoSize(true);
label->setWordWrap(true);
```

### UIImage

Отображение изображений:

```cpp
auto image = std::make_shared<UIImage>();
image->setTexture(textureId);
image->setColor(Vector4(1.0f, 1.0f, 1.0f, 1.0f));
image->setScaleMode(UIImage::ScaleMode::ScaleToFit);
image->setPreserveAspect(true);
```

### UISlider

Слайдер для выбора значений:

```cpp
auto slider = std::make_shared<UISlider>();
slider->setOrientation(UISlider::Orientation::Horizontal);
slider->setMinValue(0.0f);
slider->setMaxValue(100.0f);
slider->setValue(50.0f);
slider->setOnValueChanged([](float value) {
    std::cout << "Значение: " << value << std::endl;
});
```

### UITextField

Поле ввода текста:

```cpp
auto textField = std::make_shared<UITextField>();
textField->setPlaceholder("Введите текст...");
textField->setOnTextChanged([](const std::string& text) {
    std::cout << "Новый текст: " << text << std::endl;
});
textField->setOnEnterPressed([]() {
    std::cout << "Enter нажат!" << std::endl;
});
```

## Система событий

GUI элементы поддерживают следующие события:

- `Click` - клик мыши
- `Hover` - наведение мыши
- `Press` - нажатие кнопки мыши
- `Release` - отпускание кнопки мыши
- `Focus` - получение фокуса
- `Unfocus` - потеря фокуса
- `Enter` - вход мыши в область элемента
- `Exit` - выход мыши из области элемента

### Обработка событий

```cpp
element->setOnClick([]() {
    std::cout << "Элемент кликнут!" << std::endl;
});

element->setOnHover([](bool isHovered) {
    if (isHovered) {
        std::cout << "Мышь наведена" << std::endl;
    } else {
        std::cout << "Мышь убрана" << std::endl;
    }
});
```

## Стили и внешний вид

Каждый GUI элемент имеет стиль:

```cpp
UIStyle style;
style.backgroundColor = Vector4(1.0f, 0.0f, 0.0f, 1.0f);
style.borderColor = Vector4(0.0f, 0.0f, 0.0f, 1.0f);
style.textColor = Vector4(1.0f, 1.0f, 1.0f, 1.0f);
style.borderWidth = 2.0f;
style.cornerRadius = 5.0f;
style.opacity = 0.8f;

element->setStyle(style);
```

## Масштабирование и адаптация

GUI система поддерживает различные режимы масштабирования:

### CanvasScaleMode

- `ConstantPixelSize` - постоянный размер в пикселях
- `ScaleWithScreenSize` - масштабирование с размером экрана
- `ConstantPhysicalSize` - постоянный физический размер

```cpp
canvas->setScaleMode(CanvasScaleMode::ScaleWithScreenSize);
canvas->setReferenceResolution(1920, 1080);
```

## Рендеринг

GUI система использует специализированный UIRenderer:

```cpp
// Инициализация
GUI::InitializeGUI(screenWidth, screenHeight);

// В цикле рендеринга
canvas->render();

// Очистка
GUI::CleanupGUI();
```

### Особенности рендеринга

- 2D ортографическая проекция
- Альфа-блендинг
- Batch рендеринг для производительности
- Поддержка clipping (обрезки)
- Специализированные шейдеры

## Интеграция с GLFW

Для полной функциональности GUI нужно интегрировать с системой ввода:

```cpp
void mouse_button_callback(GLFWwindow* window, int button, int action, int mods) {
    if (button == GLFW_MOUSE_BUTTON_LEFT && canvas) {
        double xpos, ypos;
        glfwGetCursorPos(window, &xpos, &ypos);
        
        if (action == GLFW_PRESS) {
            canvas->handleMousePress(xpos, ypos);
        } else if (action == GLFW_RELEASE) {
            canvas->handleMouseRelease(xpos, ypos);
            canvas->handleMouseClick(xpos, ypos);
        }
    }
}

void cursor_position_callback(GLFWwindow* window, double xpos, double ypos) {
    if (canvas) {
        canvas->handleMouseMove(xpos, ypos);
    }
}

// Установка обратных вызовов
glfwSetMouseButtonCallback(window, mouse_button_callback);
glfwSetCursorPosCallback(window, cursor_position_callback);
```

## Пример использования

```cpp
#include "Engine4D/GUI/GUI.h"

int main() {
    // Инициализация GLFW, OpenGL, и Engine4D
    
    // Инициализация GUI
    GUI::InitializeGUI(1200, 800);
    
    // Создание canvas
    auto canvasObject = GameObject4D::create("UI Canvas");
    auto canvas = canvasObject->addComponent<UICanvas>();
    canvas->setReferenceResolution(1200, 800);
    
    // Создание кнопки
    auto button = std::make_shared<UIButton>();
    button->setText("Привет!");
    button->setRect(UIRect(100, 100, 200, 50));
    button->setOnClick([]() {
        std::cout << "Привет, мир!" << std::endl;
    });
    canvas->addElement(button);
    
    // Главный цикл
    while (!glfwWindowShouldClose(window)) {
        // Обработка событий
        glfwPollEvents();
        
        // Очистка экрана
        glClear(GL_COLOR_BUFFER_BIT);
        
        // Рендеринг GUI
        canvas->update(deltaTime);
        canvas->render();
        
        glfwSwapBuffers(window);
    }
    
    // Очистка
    GUI::CleanupGUI();
    
    return 0;
}
```

## Производительность

### Рекомендации по оптимизации

1. **Batch рендеринг** - группируйте элементы для одновременного рендеринга
2. **Минимизация обновлений** - обновляйте только изменившиеся элементы
3. **Текстурные атласы** - используйте атласы для уменьшения переключений текстур
4. **Culling** - не рендерите невидимые элементы

### Мониторинг производительности

```cpp
// Количество активных элементов
int activeElements = canvas->getElements().size();

// Время рендеринга GUI
auto start = std::chrono::high_resolution_clock::now();
canvas->render();
auto end = std::chrono::high_resolution_clock::now();
auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
```

## Расширение системы

### Создание пользовательских компонентов

```cpp
class CustomUIElement : public UIElement {
public:
    CustomUIElement() : UIElement() {}
    
    std::string getComponentType() const override { 
        return "CustomUIElement"; 
    }
    
    void render() override {
        // Пользовательская логика рендеринга
        UIElement::render(); // Рендерим дочерние элементы
    }
    
    void onEvent(UIEvent event) override {
        // Пользовательская обработка событий
    }
};
```

### Пользовательские стили

```cpp
class ThemeManager {
public:
    static UIStyle getDarkTheme() {
        UIStyle style;
        style.backgroundColor = Vector4(0.2f, 0.2f, 0.2f, 1.0f);
        style.textColor = Vector4(1.0f, 1.0f, 1.0f, 1.0f);
        style.borderColor = Vector4(0.4f, 0.4f, 0.4f, 1.0f);
        return style;
    }
    
    static UIStyle getLightTheme() {
        UIStyle style;
        style.backgroundColor = Vector4(0.9f, 0.9f, 0.9f, 1.0f);
        style.textColor = Vector4(0.0f, 0.0f, 0.0f, 1.0f);
        style.borderColor = Vector4(0.6f, 0.6f, 0.6f, 1.0f);
        return style;
    }
};
```

## Ограничения текущей версии

1. **Текст** - упрощенная система рендеринга текста (нужна интеграция с FreeType)
2. **Скругленные углы** - базовая реализация (нужны специальные шейдеры)
3. **Анимации** - не реализованы (планируется в будущих версиях)
4. **Сложные layout'ы** - нет системы автоматической компоновки
5. **Темы** - нет централизованной системы тем

## Планы развития

1. Интеграция с системой шрифтов (FreeType)
2. Система анимаций и переходов
3. Автоматическая компоновка (Layout системы)
4. Система тем и стилей
5. Drag & Drop функциональность
6. Поддержка мультитач ввода
7. Accessibility функции

---

*Документация обновлена для версии Engine4D 1.0.0*
