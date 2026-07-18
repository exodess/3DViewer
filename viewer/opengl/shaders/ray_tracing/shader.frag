#version 430 core
#define MAX_POINT_LIGHTS 5 // Максимальное количество направленных источников освещения
#define MAX_COUNT_FIGURES 5 // Максимальное количество фигур на сцене
#define MAX_COUNT_VERTICES 1000000
const float PI = 3.14159265359;

// Хранение информации о материале фигуры
struct Material {
    vec4 base_color; // Цвет материала
    float roughness; // Шероховатость поверхности
    float metallic; // Металличность поверхности
    float refractive; // Коэффициент преломления
    float reflectivity; // Коэффициент отражения
    float alpha; // Коэффициент прозрачности
};

// Хранение информации о
struct LightData {
    vec4 color; // Цвет источника света
    vec4 position; // Координата источника света
    float intensity; // Интенсивность
    float padding[3];
};

struct RayFigure {
    mat4 modelMatrix; // Матрица модели (Translation * Rotation * Scale)
    mat4 normalMatrix; // Матрица нормали
    int firstIndex; // Смещение начала индексов в общем буфере
    int indexCount; // Количество индексов фигуры
    int padding[2]; // Заполнение до 80 байт
};

struct RayVertex {
    vec4 position;
    vec4 normale;
};

// Структура, хранящая информацию обо всех источниках освещения
layout(std430, binding = 1) buffer LightsBuffer {
    LightData lights[MAX_POINT_LIGHTS + 1];
} b_LightStruct;

layout(std430, binding = 2) buffer MaterialsBuffer {
    Material materials[MAX_COUNT_FIGURES];
} b_MaterialsStruct;

layout(std430, binding = 3) buffer FiguresBuffer {
    RayFigure figures[MAX_COUNT_FIGURES];
} b_Figures;

layout(std430, binding = 4) buffer AllVertices {
    RayVertex vertices[MAX_COUNT_VERTICES];
} b_Vertices;

layout(std430, binding = 5) buffer AllIndices {
    uint indices[MAX_COUNT_VERTICES];
} b_Indices;

uniform int u_activePointLights; // Реальное количество направленных источников освещения на сцене
uniform int u_totalFigures; // Общее число загруженных фигур

// значения из вершинного шейдера
in vec3 Camera_v;
in vec3 Vertex_v;
in vec3 Normal_v;

out vec4 FragColor;

// D - Нормальное распределение микрограней (Trowbridge-Reitz GGX)
float DistributionGGX(vec3 N, vec3 H, float roughness) {
    float a = roughness * roughness;
    float a2 = a * a;
    float NdotH = max(dot(N, H), 0.0);
    float NdotH2 = NdotH * NdotH;

    float num = a2;
    float denom = (NdotH2 * (a2 - 1.0) + 1.0);
    denom = PI * denom * denom;

    return num / denom;
}

// G - Геометрическое самозатенение (Schlick-GGX)
float GeometrySchlickGGX(float NdotV, float roughness) {
    // Для прямых источников света (Point lights) k вычисляется так:
    float r = (roughness + 1.0);
    float k = (r * r) / 8.0;

    float num = NdotV;
    float denom = NdotV * (1.0 - k) + k;

    return num / denom;
}

float GeometrySmith(vec3 N, vec3 V, vec3 L, float roughness) {
    float NdotV = max(dot(N, V), 0.0);
    float NdotL = max(dot(N, L), 0.0);
    float ggx2 = GeometrySchlickGGX(NdotV, roughness);
    float ggx1 = GeometrySchlickGGX(NdotL, roughness);

    return ggx1 * ggx2;
}

// F - Уравнение Френеля (Fresnel-Schlick)
vec3 fresnelSchlick(float cosTheta, vec3 F0) {
    return F0 + (1.0 - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
}

void main() {
    // Физически-корректный рендеринг

    vec3 ks; // Интенсивность зеркального отражения
    vec3 fd, fs; // Диффузное и зеркальное отражения
    vec3 H; // Вектор половины пути
    float CamDotN; // Скалярное произведение вектора на камеру и нормали
    float LDotN; // Скалярное произведение вектора на источник и нормали

    vec3 N = normalize(Normal_v);
    vec3 V = normalize(Camera_v);

    // F0 - базовая отражательная способность.
    // Для диэлектриков она равна 0.04 (4%), для металлов берется цвет самого материала.
    vec3 F0 = mix(vec3(0.04), b_MaterialStruct.base_color.xyz, b_MaterialStruct.metallic);

    // Lo - результирующая яркость (накапливаемое освещение)
    vec3 Lo = vec3(0.0);

    for(int i = 1; i < u_activePointLights; ++i) {
        // Направление света и вектор половины пути
        vec3 L = normalize(b_LightStruct.lights[i].position.xyz - Vertex_v);
        vec3 H = normalize(V + L);

        // Затухание света (Attenuation) по закону обратных квадратов
        float distance = length(b_LightStruct.lights[i].position.xyz - Vertex_v);
        float attenuation = 1.0 / (distance * distance);

        // Энергия (Radiance), приходящая от текущей лампы
        vec3 radiance = b_LightStruct.lights[i].color.xyz * b_LightStruct.lights[i].intensity * attenuation;

        // Вычисление BRDF составляющих
        float NDF = DistributionGGX(N, H, b_MaterialStruct.roughness);
        float G = GeometrySmith(N, V, L, b_MaterialStruct.roughness);
        vec3 F = fresnelSchlick(max(dot(H, V), 0.0), F0);

        // Сборка уравнения Cook-Torrance
        vec3 numerator = NDF * G * F;
        // + 0.0001 используется для предотвращения деления на ноль
        float denominator = 4.0 * max(dot(N, V), 0.0) * max(dot(N, L), 0.0) + 0.0001;
        vec3 specular = numerator / denominator;
        specular *= b_MaterialStruct.reflectivity;

        // Вычисляем соотношение отраженного света (kS) и поглощенного/рассеянного (kD)
        vec3 kS = F;
        vec3 kD = vec3(1.0) - kS;

        // Металлы поглощают преломленный свет, поэтому диффузного цвета у них нет
        kD *= 1.0 - b_MaterialStruct.metallic;

        // Финальный расчет света для текущего источника (с учетом угла падения)
        float NdotL = max(dot(N, L), 0.0);
        Lo += (kD * b_MaterialStruct.base_color.xyz / PI + specular) * radiance * NdotL;
    }

    // Добавляем фоновое освещение
    vec3 ambient = vec3(0.03) * b_LightStruct.lights[0].color.xyz * b_LightStruct.lights[0].intensity;
    ambient *= b_MaterialStruct.base_color.xyz;
    vec3 color = ambient + Lo;

    // HDR Tone Mapping (Тональная компрессия)
    // Физические расчеты могут выдать цвет ярче 1.0, возвращаем его в диапазон монитора
    color = color / (color + vec3(1.0));

    // Гамма-коррекция
    color = pow(color, vec3(1.0 / 2.2));

    FragColor = vec4(color, b_MaterialStruct.alpha);
}