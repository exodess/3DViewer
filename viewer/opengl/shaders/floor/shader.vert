#version 430 core

layout(location = 0) in vec3 inPosition; // координата вершины

// Структура для хранения информации о камере
layout(std430, binding = 0) buffer Camera {
    mat4 projection; // Матрица проекции камеры
    mat4 view; // Матрица вида камеры
    vec3 position; // Координата камеры
} b_CameraStruct;

out vec3 Camera_vertex; // Позиция камеры относительно вершины
out vec3 Vertex_view; // Вершина в пространстве камеры

void main() {
    vec4 P = vec4(inPosition, 1.0); // трансформация вершины

    Camera_vertex = normalize(b_CameraStruct.position - P.xyz);
    Vertex_view = P.xyz;

    gl_Position = b_CameraStruct.projection * b_CameraStruct.view * P;
}
