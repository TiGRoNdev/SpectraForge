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
    
    // Общие параметры для проекции
    float fov = 45.0; // поле зрения в градусах
    float aspect = 1.5; // соотношение сторон экрана
    float near = 0.1;
    float far = 100.0;
    
    // Создаем стандартную OpenGL перспективную матрицу
    float f = 1.0 / tan(radians(fov) * 0.5);
    mat4 perspectiveMatrix = mat4(
        f / aspect, 0.0, 0.0, 0.0,
        0.0, f, 0.0, 0.0,
        0.0, 0.0, (far + near) / (near - far), (2.0 * far * near) / (near - far),
        0.0, 0.0, -1.0, 0.0
    );
    
    vec3 final3D;
    
    // Обрабатываем сечение
    if (useCrossSection) {
        // Если вершина находится далеко от плоскости сечения, отбрасываем её
        if (abs(worldPos.w - crossSectionW) > 0.5) {
            gl_Position = vec4(0.0, 0.0, 0.0, -1.0); // Отбрасываем вершину
            return;
        }
        // Для сечения используем xyz координаты напрямую
        final3D = viewPos.xyz;
    } else {
        // Перспективная проекция из 4D в 3D
        float perspectiveDistance = 10.0; // Расстояние для перспективной проекции
        
        if (abs(perspectiveDistance - viewPos.w) > 0.001) {
            float factor = perspectiveDistance / (perspectiveDistance - viewPos.w);
            final3D = viewPos.xyz * factor;
        } else {
            final3D = viewPos.xyz;
        }
    }
    
    // Применяем OpenGL проекцию
    gl_Position = perspectiveMatrix * vec4(final3D, 1.0);
    
    // Передаем данные в фрагментный шейдер
    FragPos = worldPos;
    Normal = normalize(model * normal);
    TexCoord = texCoord;
    Color = color;
    WCoord = worldPos.w;
}
