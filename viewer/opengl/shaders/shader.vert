#version 430 core

layout(location = 0) in vec3 inPosition;

uniform mat4 u_modelMatrix; // матрица модели
uniform mat4 u_viewMatrix; // матрица камеры
uniform mat4 u_projectionMatrix; // матрица проекции

void main() {
    gl_Position = u_projectionMatrix * u_viewMatrix * u_modelMatrix * vec4(inPosition, 1.0);
}
