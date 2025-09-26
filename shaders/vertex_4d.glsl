#version 330 core

// Входные атрибуты вершин
layout (location = 0) in vec4 position;    // 4D позиция [x, y, z, w]
layout (location = 1) in vec4 normal;      // 4D нормаль
layout (location = 2) in vec4 texCoord;    // 4D текстурные координаты
layout (location = 3) in vec4 color;       // RGBA цвет

// Униформы
uniform mat4 model;        // Матрица модели (4x4)
uniform mat4 view;         // Матрица вида (4x4)
uniform mat4 projection;   // Матрица проекции (4x4)
uniform float crossSectionW; // W-координата сечения
uniform bool useCrossSection; // Использовать ли сечение

// Выходные переменные
out vec4 FragPos;          // Позиция фрагмента
out vec4 Normal;           // Нормаль
out vec4 TexCoord;         // Текстурные координаты
out vec4 Color;            // Цвет
out float WCoord;          // W-координата для эффектов

void main() {
    // Применяем трансформации
    vec4 worldPos = model * position;
    vec4 viewPos = view * worldPos;
    vec4 clipPos = projection * viewPos;
    
    // Обрабатываем сечение
    if (useCrossSection) {
        // Если вершина находится на другой стороне сечения, отбрасываем её
        if (abs(worldPos.w - crossSectionW) > 0.01) {
            gl_Position = vec4(0.0, 0.0, 0.0, -1.0); // Отбрасываем вершину
            return;
        }
        
        // Проецируем на плоскость сечения
        clipPos = vec4(clipPos.xyz, 0.0);
    }
    
    // Перспективная проекция из 4D в 3D
    float perspectiveDistance = 10.0; // Расстояние для перспективной проекции
    if (clipPos.w != 0.0) {
        clipPos.xyz = clipPos.xyz * perspectiveDistance / clipPos.w;
        clipPos.w = 1.0;
    }
    
    // Дополнительная проекция из 3D в 2D (стандартная OpenGL проекция)
    gl_Position = clipPos;
    
    // Передаем данные в фрагментный шейдер
    FragPos = worldPos;
    Normal = normalize(model * normal);
    TexCoord = texCoord;
    Color = color;
    WCoord = worldPos.w;
}
