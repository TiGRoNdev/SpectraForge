#version 330 core

in vec4 vTexCoord;
in vec4 vColor;

uniform sampler2D uFontTexture;
uniform vec4 uTextColor;
uniform float uOpacity;

out vec4 FragColor;

void main() {
    // Получаем alpha канал из текстуры шрифта
    float alpha = texture(uFontTexture, vTexCoord.xy).r;
    
    // Применяем цвет текста
    vec4 finalColor = uTextColor * vColor;
    finalColor.a *= alpha * uOpacity;
    
    // Отбрасываем прозрачные пиксели
    if (finalColor.a < 0.01) {
        discard;
    }
    
    FragColor = finalColor;
}
