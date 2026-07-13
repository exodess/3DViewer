#version 430 core

layout(location = 0) in vec3 inPosition; // координата вершины

// Структура для хранения информации о камере
layout(std430, binding = 0) buffer Camera {
    mat4 projection; // Матрица проекции камеры
    mat4 view; // Матрица вида камеры
    vec3 position; // Координата камеры
} b_CameraStruct;

uniform mat4 u_modelMatrix; // матрица модели

void main() {
    vec4 P = u_modelMatrix * vec4(inPosition, 1.0); // трансформация вершины

    gl_Position = b_CameraStruct.projection * b_CameraStruct.view * P;
}
