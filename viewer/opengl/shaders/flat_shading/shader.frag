#version 430
#define MAX_POINT_LIGHTS 5 // Максимальное количество направленных источников освещения

struct LightData {
    vec4 color; // Цвет источника света
    vec4 position; // Координата источника света
    float intensity; // Интенсивность
    float padding[3];
};

// Структура, хранящая информацию обо всех источниках освещения
layout(std430, binding = 1) buffer LightsBuffer {
    LightData lights[MAX_POINT_LIGHTS + 1];
} b_LightStruct;

uniform int u_activeFlatPointLights; // Реальное количество направленных источников освещения на сцене

// значения из вершинного шейдера
in vec3 Vertex_view;

out vec4 FragColor;

void main() {
    // Нормаль
    vec3 N = normalize(cross(dFdx(Vertex_view), dFdy(Vertex_view)));

    // Общая диффузная составляющая
    vec3 diffuseAccum = vec3(0.0);

    // Базовое фоновое освещение
    vec3 ambient = b_LightStruct.lights[0].color.xyz * b_LightStruct.lights[0].intensity;

    for(int i = 1; i <= u_activeFlatPointLights; ++i) {
        vec3 Light_v = normalize(b_LightStruct.lights[i].position.xyz - Vertex_view);

        // Диффузная составляющая
        float diffuse = max(dot(Light_v, N), 0.0);

        // Накапливаем освещение: (коэффициент * цветИсточника * интенсивность)
        diffuseAccum += diffuse * b_LightStruct.lights[i].color.xyz * b_LightStruct.lights[i].intensity;
    }

    vec3 res = (ambient + diffuseAccum) * b_LightStruct.lights[0].color.xyz;
    FragColor = vec4(res, 1.0);
}