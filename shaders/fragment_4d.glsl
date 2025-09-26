#version 330 core

// Входные переменные от вершинного шейдера
in vec4 FragPos;           // Позиция фрагмента
in vec4 Normal;            // Нормаль
in vec4 TexCoord;          // Текстурные координаты
in vec4 Color;             // Цвет
in float WCoord;           // W-координата

// Униформы
uniform vec4 lightPos;     // Позиция источника света (4D)
uniform vec4 lightColor;   // Цвет света
uniform vec4 viewPos;      // Позиция камеры (4D)
uniform float ambientStrength; // Сила окружающего освещения
uniform float specularStrength; // Сила зеркального отражения
uniform float shininess;   // Блеск материала
uniform bool useWColorGradient; // Использовать градиент по W-координате

// Выходной цвет
out vec4 FragColor;

void main() {
    // Базовый цвет
    vec4 baseColor = Color;
    
    // Эффект градиента по W-координате
    if (useWColorGradient) {
        // Создаем градиент от синего к красному в зависимости от W
        float wFactor = (WCoord + 1.0) * 0.5; // Нормализуем W от -1..1 к 0..1
        baseColor = mix(vec4(0.0, 0.0, 1.0, 1.0), vec4(1.0, 0.0, 0.0, 1.0), wFactor);
    }
    
    // Окружающее освещение
    vec4 ambient = ambientStrength * lightColor;
    
    // Диффузное освещение (используем только xyz компоненты для 3D расчетов)
    vec3 lightDir3D = normalize((lightPos - FragPos).xyz);
    vec3 normal3D = normalize(Normal.xyz);
    float diff = max(dot(normal3D, lightDir3D), 0.0);
    vec4 diffuse = diff * lightColor;
    
    // Зеркальное отражение
    vec3 viewDir3D = normalize((viewPos - FragPos).xyz);
    vec3 reflectDir3D = reflect(-lightDir3D, normal3D);
    float spec = pow(max(dot(viewDir3D, reflectDir3D), 0.0), shininess);
    vec4 specular = specularStrength * spec * lightColor;
    
    // Финальный цвет
    vec4 result = (ambient + diffuse + specular) * baseColor;
    
    // Эффект прозрачности для объектов далеко в W-измерении
    float wDistance = abs(WCoord);
    float alpha = 1.0 - smoothstep(0.0, 2.0, wDistance);
    result.a *= alpha;
    
    // Эффект "призрака" для объектов в других W-слоях
    if (abs(WCoord) > 0.1) {
        result.rgb *= 0.7; // Приглушаем цвет
    }
    
    FragColor = result;
}
