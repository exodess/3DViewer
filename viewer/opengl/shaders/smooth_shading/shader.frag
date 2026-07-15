#version 430 core
#define MAX_POINT_LIGHTS 5 // Максимальное количество направленных источников освещения

layout(std430, binding = 2) buffer Material {
    vec4 base_color; // Цвет материала
    float roughness; // Шероховатость поверхности
    float metallic; // Металличность поверхности
    float refractive; // Коэффициент преломления
    float reflectivity; // Коэффициент отражения
    float alpha; // Коэффициент прозрачности
} b_MaterialStruct;

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

uniform int u_activeSmoothPointLights; // Реальное количество направленных источников освещения на сцене

// значения из вершинного шейдера
in vec3 Camera_vertex;
in vec3 Vertex_view;
in vec3 Normal_vertex;

out vec4 FragColor;

void main() {
    // мягкое затенение методом Блинна-Фонга

    vec3 N = normalize(Normal_vertex); // Интерполированная нормаль

    // Базовое фоновое освещение
    vec3 ambient = b_LightStruct.lights[0].color.xyz * b_LightStruct.lights[0].intensity;
    vec3 diffuseAccum = vec3(0, 0, 0); // Общая диффузная составляющая
    vec3 specularAccum = vec3(0, 0, 0); // Общая зеркальная составляющая

    for(int i = 1; i <= u_activeSmoothPointLights; ++i) {
        vec3 Light_v = normalize(b_LightStruct.lights[i].position.xyz - Vertex_view);

        // Интенсивность диффузного отражения
        float diffuse = max(dot(normalize(Light_v), N), 0.0);
        diffuseAccum += diffuse * b_LightStruct.lights[i].color.xyz * b_LightStruct.lights[i].intensity;

        // Вектор половины пути
        vec3 H = normalize(normalize(Light_v) + normalize(Camera_vertex));

        // Зеркальная составляющая
        float specular = pow(max(dot(H, N), 0.0), 32);
        specularAccum += specular * b_LightStruct.lights[i].color.xyz + b_LightStruct.lights[i].intensity;
    }

    vec3 diffuseComponent = diffuseAccum * (1 - b_MaterialStruct.metallic);
    // В жизни почти все неметаллы отражают свет одинаково (~4%), поэтому для metallic == 0.0 будет возвращено vec3(0.04)
    vec3 specularComponent = specularAccum * mix(vec3(0.04), b_MaterialStruct.base_color.xyz, b_MaterialStruct.metallic) * b_MaterialStruct.reflectivity;

    vec3 res = (ambient + diffuseComponent + specularComponent) * b_MaterialStruct.base_color.xyz;
    FragColor = vec4(res, b_MaterialStruct.alpha);
}