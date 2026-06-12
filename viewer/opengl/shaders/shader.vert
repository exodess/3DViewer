#version 430 core
#define MAX_POINT_LIGHTS 5 // Максимальное количество направленных источников освещения

layout(location = 0) in vec3 inPosition; // координата вершины
layout(location = 1) in vec3 inNormal; // Вектор нормали

// Структура для хранения информации о камере
layout(std430, binding = 0) buffer Camera {
    mat4 projection; // Матрица проекции камеры
    mat4 view; // Матрица вида камеры
    vec3 position; // Координата камеры
} b_CameraStruct;

// Структура, хранящая информацию обо всех источниках освещения
layout(std430, binding = 1) buffer LightsBuffer {
    struct LightData {
        vec3 position; // Координата источника света
        float intensity; // Интенсивность
        vec3 color; // Цвет источника света
    } lights[MAX_POINT_LIGHTS + 1];
} b_LightStruct;

uniform int u_activePointLights; // Реальное количество направленных источников освещения на сцене
uniform mat4 u_modelMatrix; // матрица модели
uniform mat3 u_normalMatrix; // Матрица для трансформации нормали

out vec3 Camera_vertex; // Позиция камеры относительно вершины
out vec3 Light_vertex; // Позиция источника света относительно вершины
out vec3 Vertex_view; // Вершина в пространстве камеры
out vec3 Normal_vertex; // Трансформированная нормаль

void main() {
    vec4 P = u_modelMatrix * vec4(inPosition, 1.0); // трансформация вершины

    Camera_vertex = normalize(b_CameraStruct.position - P.xyz);
    Light_vertex = normalize(b_LightStruct.position - P.xyz);
    Vertex_view = P.xyz;
    Normal_vertex = normalize(u_normalMatrix * inNormal);

    gl_Position = b_CameraStruct.projection * b_CameraStruct.view * P;
}
