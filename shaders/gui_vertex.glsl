#version 330 core

layout (location = 0) in vec4 aPosition;
layout (location = 1) in vec4 aTexCoord;
layout (location = 2) in vec4 aColor;

uniform mat4 uProjection;
uniform mat4 uModel;

out vec4 vTexCoord;
out vec4 vColor;

void main() {
    gl_Position = uProjection * uModel * aPosition;
    vTexCoord = aTexCoord;
    vColor = aColor;
}
