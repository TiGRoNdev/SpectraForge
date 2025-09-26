#version 330 core

in vec4 vTexCoord;
in vec4 vColor;

uniform bool uUseTexture;
uniform sampler2D uTexture;
uniform vec4 uTintColor;
uniform float uOpacity;

out vec4 FragColor;

void main() {
    vec4 finalColor;
    
    if (uUseTexture) {
        // Используем текстуру
        vec4 textureColor = texture(uTexture, vTexCoord.xy);
        finalColor = textureColor * vColor * uTintColor;
    } else {
        // Используем только цвет вершины
        finalColor = vColor * uTintColor;
    }
    
    // Применяем прозрачность
    finalColor.a *= uOpacity;
    
    FragColor = finalColor;
}
