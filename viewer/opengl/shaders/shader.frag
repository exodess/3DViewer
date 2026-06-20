#version 430 core

#define MAX_POINT_LIGHTS 5 // Максимальное количество направленных источников освещения
out vec4 FragColor;

// значения из геометрического шейдера
in vec2 vTexCoord;
in float vLineDist;
flat in int vIsPoint;
flat in int displayType; // 0 - каркасная модель, 1 - плоское затенение, 2 - мягкое затенение

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

uniform int u_isFloor; // 1 - отображение пола, 0 - остальных фигур на сцене
uniform int u_activePointLights; // Реальное количество направленных источников освещения на сцене
uniform vec3 u_lightColor; // цвет источника света
uniform vec3 u_vertColor; // цвет вершин
uniform int u_isVertCircle; // 1 - круг, 0 - квадрат
uniform vec3 u_edgesColor; // цвет ребер
uniform float u_dashSize; // размер штриха (0 для сплошной)

// значения из вершинного шейдера
in vec3 Camera_v;
in vec3 Vertex_v;
in vec3 Normal_v;

const float PI = 3.14159265359;

// 1. D - Нормальное распределение микрограней (Trowbridge-Reitz GGX)
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

// 2. G - Геометрическое самозатенение (Schlick-GGX)
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

// 3. F - Уравнение Френеля (Fresnel-Schlick)
vec3 fresnelSchlick(float cosTheta, vec3 F0) {
	return F0 + (1.0 - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
}

void main() {
	if (u_isFloor == 1) {
		// Координаты сетки (зависят от абсолютных мировых координат)
		vec2 coord = Camera_v.xz / 1.0;
		vec2 derivative = fwidth(coord);

		vec2 grid = abs(fract(coord - 0.5) - 0.5) / derivative;
		float lineAlpha = min(grid.x, grid.y);

		// Вычисляем интенсивность пикселя линии (от 1.0 в центре до 0.0 на краю)
		float lineIntensity = 1.0 - min(lineAlpha, 1.0);

		// Если пиксель не принадлежит линии сетки — не отрисовываем его вообще
		if (lineIntensity <= 0.0) {
			discard;
		}

		// Плавное затухание (Fade Out) сетки к горизонту
		float distanceToCam = length(Vertex_v - Camera_v);
		float fade = clamp(1.0 - (distanceToCam / 80.0), 0.0, 1.0);

		// Отрисовываем саму линию. Прозрачность линии зависит от сглаживания и дальности
		vec3 gridLineColor = vec3(0.5, 0.5, 0.5);
		FragColor = vec4(gridLineColor, lineIntensity * fade);
		return;
	}

	if(displayType == 0) {
		if(vIsPoint == 1) {
			// если точка - круг, отсекаем лишние пиксели по окружности
			if(u_isVertCircle == 1 && length(vTexCoord - 0.5) > 0.5) {
				discard;
			}
			FragColor = vec4(u_vertColor, 1.0);
		}
		else {
			// Отрисовка ребра
			if(u_dashSize > 0.0 && mod(vLineDist, u_dashSize * 2.0) > u_dashSize) {
				discard;
			}
			FragColor = vec4(u_edgesColor, 1.0);
		}
	}

	else if(displayType == 1) {
		// Нормаль
		vec3 N = normalize(cross(dFdx(Vertex_v), dFdy(Vertex_v)));

		// Общая диффузная составляющая
		vec3 diffuseAccum = vec3(0.0);

		// Базовое фоновое освещение
		vec3 ambient = b_LightStruct.lights[0].color.xyz * b_LightStruct.lights[0].intensity;

		for(int i = 1; i < u_activePointLights; ++i) {
			vec3 Light_v = normalize(b_LightStruct.lights[i].position.xyz - Vertex_v);

			// Диффузная составляющая
			float diffuse = max(dot(Light_v, N), 0.0);

			// Накапливаем освещение: (коэффициент * цветИсточника * интенсивность)
			diffuseAccum += diffuse * b_LightStruct.lights[i].color.xyz * b_LightStruct.lights[i].intensity;
		}

		vec3 res = (ambient + diffuseAccum) * b_LightStruct.lights[0].color.xyz;
		FragColor = vec4(res, 1.0);
	}

	else if(displayType == 2) {
		// мягкое освещение
		// Затенение методом Блинна-Фонга

		vec3 N = normalize(Normal_v); // Интерполированная нормаль

		// Базовое фоновое освещение
		vec3 ambient = b_LightStruct.lights[0].color.xyz * b_LightStruct.lights[0].intensity;
		vec3 diffuseAccum = vec3(0, 0, 0); // Общая диффузная составляющая
		vec3 specularAccum = vec3(0, 0, 0); // Общая зеркальная составляющая

		for(int i = 1; i < u_activePointLights; ++i) {
			vec3 Light_v = normalize(b_LightStruct.lights[i].position.xyz - Vertex_v);

			// Интенсивность диффузного отражения
			float diffuse = max(dot(normalize(Light_v), N), 0.0);
			diffuseAccum += diffuse * b_LightStruct.lights[i].color.xyz * b_LightStruct.lights[i].intensity;

			// Вектор половины пути
			vec3 H = normalize(normalize(Light_v) + normalize(Camera_v));

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

	else if(displayType == 3) {
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
}
