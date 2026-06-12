#version 430 core

#define MAX_POINT_LIGHTS 5 // Максимальное количество направленных источников освещения
out vec4 FragColor;

// значения из геометрического шейдера
in vec2 vTexCoord;
in float vLineDist;
flat in int vIsPoint;
flat in int displayType; // 0 - каркасная модель, 1 - плоское затенение, 2 - мягкое затенение

layout(std430, binding = 2) buffer Material {
	vec3 base_color; // Цвет материала
	float roughness; // Шероховатость поверхности
	float metallic; // Металличность поверхности
	float refractive; // Коэффициент преломления
	float reflectivity; // Коэффициент отражения
	float alpha; // Коэффициент прозрачности
} b_MaterialStruct;

// Структура, хранящая информацию обо всех источниках освещения
layout(std430, binding = 1) buffer LightsBuffer {
	struct LightData {
		vec3 position; // Координата источника света
		float intensity; // Интенсивность
		vec3 color; // Цвет источника света
	} lights[MAX_POINT_LIGHTS + 1];
} b_LightStruct;

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

void main() {
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
		vec3 ambient = b_LightStruct.lights[0].color * b_LightStruct.lights[0].intensity;

		for(int i = 1; i < u_activePointLights; ++i) {
			vec3 Light_v = normalize(b_LightStruct.lights[i].position - Vertex_v);

			// Диффузная составляющая
			float diffuse = max(dot(Light_v, N), 0.0);

			// Накапливаем освещение: (коэффициент * цветИсточника * интенсивность)
			diffuseAccum += diff * b_LightStruct.lights[i].color * b_LightStruct.lights[i].intensity;
		}

		vec3 res = b_LightStruct.lights[0].color * (ambient + diffuseAccum);
		FragColor = vec4(res, 1.0);
	}

	else if(displayType == 2) {
		// мягкое освещение
		// Затенение методом Блинна-Фонга

		vec3 N = normalize(Normal_v); // Интерполированная нормаль

		// Базовое фоновое освещение
		vec3 ambient = b_LightStruct.lights[0].color * b_LightStruct.lights[0].intensity;
		vec3 diffuseAccum = vec3(0, 0, 0); // Общая диффузная составляющая
		vec3 specularAccum = vec3(0, 0, 0); // Общая зеркальная составляющая

		for(int i = 1; i < u_activePointLights; ++i) {
			vec3 Light_v = normalize(b_LightStruct.lights[i].position - Vertex_v);

			// Интенсивность диффузного отражения
			float diffuse = max(dot(normalize(Light_v), N), 0.0);
			diffuseAccum += diffuse * b_LightStruct.lights[i].color * b_LightStruct.lights[i].intensity;

			// Вектор половины пути
			vec3 H = normalize(normalize(Light_v) + normalize(Camera_v));

			// Зеркальная составляющая
			float specular = pow(max(dot(H, N), 0.0), 32);
			specularAccum += specular * b_LightStruct.lights[i].color + b_LightStruct.lights[i].intensity;
		}

		vec3 diffuseComponent = diffuseAccum * b_MaterialStruct.base_color * (1 - b_MaterialStruct.metallic);
		// В жизни почти все неметаллы отражают свет одинаково (~4%), поэтому для metallic == 0.0 будет возвращено vec3(0.04)
		vec3 specularComponent = specularAccum * mix(vec3(0.04), b_MaterialStruct.base_color, b_MaterialStruct.metallic) * b_MaterialStruct.reflectivity;

		vec3 res = ambient + diffuseComponent + specularComponent;
		FragColor = vec4(res, b_MaterialStruct.alpha);

	}

	else if(displayType == 3) {
		// Ray-tracing
	}
}
