#version 430 core

layout(location = 0) in vec3 inPosition; // координата вершины
layout(location = 1) in vec3 inNormal; // Вектор нормали

uniform mat4 u_modelMatrix; // матрица модели
uniform mat3 u_normalMatrix; // Матрица для трансформации нормали
uniform mat4 u_viewMatrix; // матрица камеры
uniform mat4 u_projectionMatrix; // матрица проекции вида
uniform vec3 u_cameraPosition; // координата камеры в пространстве
uniform vec3 u_lightPosition; // координата источника света

out vec3 Camera_vertex; // Позиция камеры относительно вершины
out vec3 Light_vertex; // Позиция источника света относительно вершины
out vec3 Vertex_view; // Вершина в пространстве камеры
out vec3 Normal_vertex; // Трансформированная нормаль

void main() {
    vec4 P = u_modelMatrix * vec4(inPosition, 1.0); // трансформация вершины

    Camera_vertex = normalize(u_cameraPosition - P.xyz);
    Light_vertex = normalize(u_lightPosition - P.xyz);
    Vertex_view = P.xyz;
    Normal_vertex = normalize(u_normalMatrix * inNormal);

    gl_Position = u_projectionMatrix * u_viewMatrix * P;
}
