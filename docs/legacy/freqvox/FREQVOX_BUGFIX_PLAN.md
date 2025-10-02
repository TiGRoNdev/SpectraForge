# 🔧 FreqVox Demo - План Исправления Критических Багов

## 🎯 Проблема
Демо `freqvox_sponza_demo.cpp` не работает:
- Окно показывает "дубликат экрана" вместо рендера
- Управление не работает (только ESC)
- Нет реального отображения графики

## 🔍 Основная Причина
```cpp
// Строка 447: Окно БЕЗ графического API
glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);  // ❌ Нет OpenGL/Vulkan!

// Строка 946-950: Пустая функция отображения
void displayFrame(const std::vector<float>& buffer) {
    (void)buffer;  // ❌ Ничего не делает!
}
```

## ✅ Решение (3 варианта)

### Вариант 1: OpenGL (Простой) ⭐ РЕКОМЕНДУЕТСЯ ДЛЯ ПРОТОТИПА

#### Шаг 1: Изменить инициализацию окна
```cpp
// В FreqVoxDemo::initialize(), ЗАМЕНИТЬ строки 446-448:

// УДАЛИТЬ:
// glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

// ДОБАВИТЬ:
glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
```

#### Шаг 2: Инициализировать OpenGL
```cpp
// В FreqVoxDemo::initialize(), ПОСЛЕ glfwCreateWindow (строка 455):

glfwMakeContextCurrent(window_);

// Инициализация GLEW (добавить в CMakeLists.txt: find_package(GLEW))
if (glewInit() != GLEW_OK) {
    SAFE_ERROR("Ошибка инициализации GLEW");
    return false;
}

SAFE_PRINT_LINE("[OpenGL] Версия: " + 
                std::string(reinterpret_cast<const char*>(glGetString(GL_VERSION))));
```

#### Шаг 3: Создать OpenGL текстуру
```cpp
// В FreqVoxDemo private members (добавить после строки 603):
GLuint displayTexture_ = 0;
GLuint quadVAO_ = 0;
GLuint quadVBO_ = 0;
GLuint shaderProgram_ = 0;

// В initializeBuffers() (добавить в конец функции):
void initializeBuffers() {
    // ... существующий код ...
    
    // Создать OpenGL текстуру
    glGenTextures(1, &displayTexture_);
    glBindTexture(GL_TEXTURE_2D, displayTexture_);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    
    // Создать fullscreen quad
    createFullscreenQuad();
}

void createFullscreenQuad() {
    float quadVertices[] = {
        // positions   // texCoords
        -1.0f,  1.0f,  0.0f, 1.0f,
        -1.0f, -1.0f,  0.0f, 0.0f,
         1.0f, -1.0f,  1.0f, 0.0f,
         1.0f,  1.0f,  1.0f, 1.0f
    };
    
    glGenVertexArrays(1, &quadVAO_);
    glGenBuffers(1, &quadVBO_);
    glBindVertexArray(quadVAO_);
    glBindBuffer(GL_ARRAY_BUFFER, quadVBO_);
    glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), quadVertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
    
    // Создать шейдер
    createShaderProgram();
}

void createShaderProgram() {
    const char* vertexShaderSource = R"(
        #version 330 core
        layout (location = 0) in vec2 aPos;
        layout (location = 1) in vec2 aTexCoord;
        out vec2 TexCoord;
        void main() {
            gl_Position = vec4(aPos, 0.0, 1.0);
            TexCoord = aTexCoord;
        }
    )";
    
    const char* fragmentShaderSource = R"(
        #version 330 core
        in vec2 TexCoord;
        out vec4 FragColor;
        uniform sampler2D screenTexture;
        void main() {
            FragColor = texture(screenTexture, TexCoord);
        }
    )";
    
    // Компиляция шейдеров (упрощенно)
    GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vertexShaderSource, nullptr);
    glCompileShader(vertexShader);
    
    GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fragmentShaderSource, nullptr);
    glCompileShader(fragmentShader);
    
    shaderProgram_ = glCreateProgram();
    glAttachShader(shaderProgram_, vertexShader);
    glAttachShader(shaderProgram_, fragmentShader);
    glLinkProgram(shaderProgram_);
    
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);
}
```

#### Шаг 4: Реализовать displayFrame()
```cpp
// ЗАМЕНИТЬ функцию displayFrame (строка 946):

void displayFrame(const std::vector<float>& buffer) {
    // Очистить экран
    glClear(GL_COLOR_BUFFER_BIT);
    
    // Загрузить данные в текстуру
    glBindTexture(GL_TEXTURE_2D, displayTexture_);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 
                 renderWidth_, renderHeight_,
                 0, GL_RGB, GL_FLOAT, buffer.data());
    
    // Отрисовать fullscreen quad
    glUseProgram(shaderProgram_);
    glBindVertexArray(quadVAO_);
    glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
}
```

#### Шаг 5: Обновить CMakeLists.txt
```cmake
# Добавить в CMakeLists.txt:
find_package(OpenGL REQUIRED)
find_package(GLEW REQUIRED)

target_link_libraries(FreqVox_Sponza_Demo 
    PRIVATE 
    ${OPENGL_LIBRARIES}
    GLEW::GLEW
    glfw
    # ... остальные зависимости
)
```

---

### Вариант 2: Vulkan (Правильный, но сложнее)

Требует создания:
- VkSurfaceKHR через `glfwCreateWindowSurface`
- VkSwapchainKHR для double/triple buffering
- VkImage и staging buffer для копирования данных
- VkCommandBuffer для записи команд презентации

**Оценка времени:** 4-6 часов работы

---

### Вариант 3: Headless + Save to File (для тестирования)

```cpp
void displayFrame(const std::vector<float>& buffer) {
    static int frameNum = 0;
    if (frameNum % 60 == 0) { // Каждую секунду
        std::string filename = "frame_" + std::to_string(frameNum) + ".ppm";
        saveToPPM(filename, buffer, renderWidth_, renderHeight_);
        std::cout << "Сохранен: " << filename << std::endl;
    }
    frameNum++;
}

void saveToPPM(const std::string& filename, const std::vector<float>& buffer, 
               int width, int height) {
    std::ofstream file(filename, std::ios::binary);
    file << "P6\n" << width << " " << height << "\n255\n";
    
    for (size_t i = 0; i < buffer.size(); i += 3) {
        unsigned char r = static_cast<unsigned char>(std::clamp(buffer[i + 0] * 255.0f, 0.0f, 255.0f));
        unsigned char g = static_cast<unsigned char>(std::clamp(buffer[i + 1] * 255.0f, 0.0f, 255.0f));
        unsigned char b = static_cast<unsigned char>(std::clamp(buffer[i + 2] * 255.0f, 0.0f, 255.0f));
        file.write(reinterpret_cast<char*>(&r), 1);
        file.write(reinterpret_cast<char*>(&g), 1);
        file.write(reinterpret_cast<char*>(&b), 1);
    }
}
```

---

## 📋 Чек-лист Исправлений

### Минимальный (для работающего демо):
- [ ] Удалить `GLFW_NO_API`
- [ ] Добавить OpenGL контекст (3.3 Core)
- [ ] Инициализировать GLEW
- [ ] Создать текстуру и quad
- [ ] Реализовать `displayFrame()` с `glTexImage2D`
- [ ] Добавить зависимости в CMake
- [ ] Пересобрать проект

### Дополнительно (для правильной математики):
- [ ] Реализовать настоящий DCT-II в CpuDctBackend
- [ ] Добавить frequency-domain convolution
- [ ] Интегрировать VkFFT с DCT режимом

---

## 🚀 Быстрый Старт (5 минут)

```bash
# 1. Установить зависимости
sudo apt-get install libglew-dev

# 2. Изменить код (см. Шаги 1-4 выше)

# 3. Пересобрать
cd /home/tigron/Documents/GITHUB/SpectraForge/build
cmake .. -DCMAKE_BUILD_TYPE=Release
make FreqVox_Sponza_Demo

# 4. Запустить
./examples/FreqVox_Sponza_Demo
```

---

## 🎯 Ожидаемый Результат

После исправлений:
- ✅ Окно показывает реальный рендер (воксели сцены)
- ✅ Камера управляется WASD + мышь
- ✅ FPS отображается в заголовке окна
- ✅ Статистика выводится в консоль

---

## 📞 Поддержка

Если возникнут проблемы:
1. Проверить, что GLEW установлен: `pkg-config --libs glew`
2. Проверить версию OpenGL: `glxinfo | grep "OpenGL version"`
3. Включить дебаг вывод OpenGL ошибок

---

**Приоритет:** 🔴 КРИТИЧЕСКИЙ  
**Время на исправление:** 30-60 минут  
**Сложность:** ⭐⭐ (Средняя)

