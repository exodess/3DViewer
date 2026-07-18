#version 430 core
#define MAX_POINT_LIGHTS 5 // Максимальное количество направленных источников освещения
#define MAX_COUNT_FIGURES 5 // Максимальное количество фигур на сцене
#define MAX_COUNT_VERTICES 1000000
const float PI = 3.14159265359;

// UV-координаты из вершинного шейдера
in vec2 screenUV;

// Хранение информации о материале фигуры
struct Material {
    vec4 base_color; // Цвет материала
    float roughness; // Шероховатость поверхности
    float metallic; // Металличность поверхности
    float refractive; // Коэффициент преломления
    float reflectivity; // Коэффициент отражения
    float alpha; // Коэффициент прозрачности
    float padding[2]; // Выравнивание до 48 байт
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

// Структура для хранения информации о камере
layout(std430, binding = 0) buffer Camera {
    mat4 projection; // Матрица проекции камеры
    mat4 view; // Матрица вида камеры
    vec3 position; // Координата камеры
} b_CameraStruct;

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

uniform int u_rayTracingPointLights; // Реальное количество направленных источников освещения на сцене
uniform int u_totalFigures; // Общее число загруженных фигур

out vec4 FragColor;

// Функция пересечения Меллера-Трумбора
float intersectTriangle(vec3 ro, vec3 rd, vec3 v0, vec3 v1, vec3 v2, out vec2 barycentric) {
    vec3 edge1 = v1 - v0;
    vec3 edge2 = v2 - v0;
    vec3 h = cross(rd, edge2);
    float a = dot(edge1, h);

    if (abs(a) < 0.0000001) return -1.0; // Луч параллелен треугольнику

    float f = 1.0 / a;
    vec3 s = ro - v0;
    float u = f * dot(s, h);

    if (u < 0.0 || u > 1.0) return -1.0;

    vec3 q = cross(s, edge1);
    float v = f * dot(rd, q);

    if (v < 0.0 || u + v > 1.0) return -1.0;

    float t = f * dot(edge2, q);
    if (t > 0.0000001) {
        barycentric = vec2(u, v);
        return t;
    }
    return -1.0;
}

void main() {
    // Генерация луча
    vec2 ndc = screenUV * 2.0 - 1.0;

    // Вычисляем обратные матрицы прямо из буфера камеры
    mat4 invProj = inverse(b_CameraStruct.projection);
    mat4 invView = inverse(b_CameraStruct.view);

    vec4 target = invProj * vec4(ndc.x, ndc.y, 1.0, 1.0);
    vec3 rayDirCamera = normalize(target.xyz / target.w);
    vec3 rayDirWorld = normalize((invView * vec4(rayDirCamera, 0.0)).xyz);

    vec3 rayOrigin = b_CameraStruct.position;

    // Поиск пересечений
    float closestT = 999999.0;
    vec3 hitNormal = vec3(0.0);
    vec3 hitPoint = vec3(0.0);
    int hitMaterialId = -1;

    for (int i = 0; i < u_totalFigures; ++i) {
        RayFigure fig = b_Figures.figures[i];

        mat4 modelMat = fig.modelMatrix;
        mat3 normalMat = transpose(inverse(mat3(modelMat)));

        for (int j = 0; j < fig.indexCount; j += 3) {
            uint i0 = b_Indices.indices[fig.firstIndex + j + 0];
            uint i1 = b_Indices.indices[fig.firstIndex + j + 1];
            uint i2 = b_Indices.indices[fig.firstIndex + j + 2];

            // Теперь вершины читаются элегантно напрямую через структуру RayVertex
            vec3 v0 = (modelMat * b_Vertices.vertices[i0].position).xyz;
            vec3 v1 = (modelMat * b_Vertices.vertices[i1].position).xyz;
            vec3 v2 = (modelMat * b_Vertices.vertices[i2].position).xyz;

            vec2 bary;
            float t = intersectTriangle(rayOrigin, rayDirWorld, v0, v1, v2, bary);

            if (t > 0.0 && t < closestT) {
                closestT = t;
                hitMaterialId = i;
                hitPoint = rayOrigin + rayDirWorld * t;

                // Читаем локальные нормали
                vec3 n0 = b_Vertices.vertices[i0].normale.xyz;
                vec3 n1 = b_Vertices.vertices[i1].normale.xyz;
                vec3 n2 = b_Vertices.vertices[i2].normale.xyz;

                float w = 1.0 - bary.x - bary.y;
                vec3 localNormal = normalize(n0 * w + n1 * bary.x + n2 * bary.y);

                hitNormal = normalize(normalMat * localNormal);
            }
        }
    }

    // Отрисовка и освещение
    if (hitMaterialId != -1) {
        // Достаем материал из буфера материалов по ID
        Material mat = b_MaterialsStruct.materials[hitMaterialId];

        // Логика освещения аналогичная вашему растеризатору
        // lights[0] - это Ambient (фоновое освещение)
        vec3 ambient = b_LightStruct.lights[0].color.xyz * b_LightStruct.lights[0].intensity;
        vec3 diffuseAccum = vec3(0.0);

        // Считаем направленные источники освещения (начиная с i = 1)
        for(int i = 1; i <= u_rayTracingPointLights; ++i) {
            vec3 lightDir = normalize(b_LightStruct.lights[i].position.xyz - hitPoint);
            float diff = max(dot(hitNormal, lightDir), 0.0);

            diffuseAccum += diff * b_LightStruct.lights[i].color.xyz * b_LightStruct.lights[i].intensity;
        }

        // Итоговый цвет объекта
        vec3 finalColor = (ambient + diffuseAccum) * mat.base_color.xyz;
        FragColor = vec4(finalColor, mat.alpha);

    } else {
        // Цвет фона (Skybox или однородная заливка), если луч не столкнулся с фигурами
        FragColor = vec4(0.1, 0.1, 0.1, 1.0);
    }
}