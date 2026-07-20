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
    float padding[3]; // Выравнивание до 48 байт
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
    int firstVertex; // Смещение начала вершин в общем буфере
    int firstIndex; // Смещение начала индексов вершин в общем буфере
    int indexCount; // Количество индексов фигуры
    int padding; // Выравнивание 16 байт
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
uniform vec3 u_backgroundColor; // Цвет фона

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
    ndc.y = -ndc.y;

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
    int hitObjectId = -1;

    // Проверяем, что луч не параллелен полу (защита от деления на ноль)
    if (abs(rayDirWorld.y) > 1e-6) {
        float tFloor = -rayOrigin.y / rayDirWorld.y;

        // Пересечение валидно, если оно перед камерой (t > 0)
        if (tFloor > 0.0 && tFloor < closestT) {
            closestT = tFloor;
            hitObjectId = -2; // Назначаем уникальный ID для пола
            hitPoint = rayOrigin + rayDirWorld * tFloor;

            // Нормаль пола всегда смотрит строго вверх (или вниз, если камера под полом)
            hitNormal = rayOrigin.y > 0.0 ? vec3(0.0, 1.0, 0.0) : vec3(0.0, -1.0, 0.0);
        }
    }

    for (int i = 0; i < u_totalFigures; ++i) {
        RayFigure fig = b_Figures.figures[i];

        mat4 modelMat = fig.modelMatrix;

        // Приведение mat4 к mat3 нужно для безопасного умножения нормалей
        // без учета смещения (translation)
        mat3 normalMat = mat3(fig.normalMatrix);

        for (int j = 0; j < fig.indexCount; j += 3) {
            uint i0 = b_Indices.indices[fig.firstIndex + j + 0];
            uint i1 = b_Indices.indices[fig.firstIndex + j + 1];
            uint i2 = b_Indices.indices[fig.firstIndex + j + 2];

            // Заменяем W-компонент на 1.0, чтобы исключить мусор из памяти C++
            vec4 pos0 = vec4(b_Vertices.vertices[fig.firstVertex + i0].position.xyz, 1.0);
            vec4 pos1 = vec4(b_Vertices.vertices[fig.firstVertex + i1].position.xyz, 1.0);
            vec4 pos2 = vec4(b_Vertices.vertices[fig.firstVertex + i2].position.xyz, 1.0);

            vec3 v0 = (modelMat * pos0).xyz;
            vec3 v1 = (modelMat * pos1).xyz;
            vec3 v2 = (modelMat * pos2).xyz;

            vec2 bary;
            float t = intersectTriangle(rayOrigin, rayDirWorld, v0, v1, v2, bary);

            // Если нашли пересечение и оно ближе, чем предыдущее найденное
            if (t > 0.0 && t < closestT) {
                closestT = t;
                hitObjectId = i;
                hitPoint = rayOrigin + rayDirWorld * t;

                // Читаем локальные нормали
                vec3 n0 = b_Vertices.vertices[i0].normale.xyz;
                vec3 n1 = b_Vertices.vertices[i1].normale.xyz;
                vec3 n2 = b_Vertices.vertices[i2].normale.xyz;

                // Интерполяция локальной нормали по барицентрическим координатам
                float w = 1.0 - bary.x - bary.y;
                vec3 localNormal = normalize(n0 * w + n1 * bary.x + n2 * bary.y);

                // Перевод нормали в мировые координаты
                hitNormal = normalize(normalMat * localNormal);
            }
        }
    }

    if (hitObjectId != -1) {
        vec3 albedo;
        float metallic;
        float roughness;
        float alpha = 1.0;

        if (hitObjectId == -2) {
            // === ПРОЦЕДУРНЫЙ МАТЕРИАЛ ПОЛА ===
            float scale = 0.5; // Размер клеток

            // Математика шахматной доски (чередование 0 и 1 в зависимости от координат X и Z)
            float pattern = mod(floor(hitPoint.x * scale) + floor(hitPoint.z * scale), 2.0);

            albedo = pattern < 1.0 ? vec3(0.8) : vec3(0.2); // Белые и темно-серые клетки
            metallic = 0.05;  // Легкий отблеск
            roughness = 0.15; // Делаем пол почти гладким, чтобы видеть красивые блики от ламп
        } else {
            // === МАТЕРИАЛ ОБЫЧНЫХ ФИГУР ===
            Material mat = b_MaterialsStruct.materials[hitObjectId];
            albedo = mat.base_color.xyz;
            metallic = mat.metallic;
            roughness = clamp(mat.roughness, 0.05, 1.0);
            alpha = mat.alpha;
        }

        vec3 diffuseColor = albedo * (1.0 - metallic);
        vec3 F0 = mix(vec3(0.04), albedo, metallic);

        vec3 viewDir = normalize(b_CameraStruct.position - hitPoint);
        vec3 totalLight = vec3(0.0);

        // Глобальное освещение, чтобы тени не были абсолютно черными
        vec3 ambient = vec3(0.03) * albedo;

        // 2. Расчет вклада всех источников освещения
        for (int i = 0; i < u_rayTracingPointLights; ++i) {
            vec3 lightPos = b_LightStruct.lights[i].position.xyz;
            vec3 lightColor = b_LightStruct.lights[i].color.xyz;
            float lightIntensity = b_LightStruct.lights[i].intensity;

            vec3 lightVec = lightPos - hitPoint;
            float distanceToLight = length(lightVec);
            vec3 lightDir = normalize(lightVec);

            // Диффузная составляющая (зависит от угла наклона света)
            float diff = max(dot(hitNormal, lightDir), 0.0);

            // Бликовая составляющая (зависит от позиции наблюдателя и шероховатости)
            vec3 halfwayDir = normalize(lightDir + viewDir);
            float NdotH = max(dot(hitNormal, halfwayDir), 0.0);

            float shininess = pow(2.0, (1.0 - roughness) * 10.0 + 1.0);
            float spec = pow(NdotH, shininess);
            vec3 specularColor = F0 * spec;

            // Затухание света с расстоянием (квадратичный закон)
            float attenuation = 1.0 / (1.0 + 0.09 * distanceToLight + 0.032 * (distanceToLight * distanceToLight));

            // Итоговый свет от конкретной лампы
            totalLight += (diffuseColor * diff + specularColor) * lightColor * lightIntensity * attenuation;
        }

        vec3 finalColor = ambient + totalLight;

        // 3. Тон-маппинг (HDR -> LDR) и гамма-коррекция
        finalColor = finalColor / (finalColor + vec3(1.0));
        finalColor = pow(finalColor, vec3(1.0 / 2.2));

        FragColor = vec4(finalColor, alpha);
    }

    else {
        FragColor = vec4(u_backgroundColor, 1.0);
    }
}