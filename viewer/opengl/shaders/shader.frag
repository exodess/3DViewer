#version 430 core

out vec4 FragColor;

// значения из геометрического шейдера
in vec2 vTexCoord;
in float vLineDist;
flat in int vIsPoint;
flat in int displayType; // 0 - каркасная модель, 1 - плоское затенение, 2 - мягкое затенение

uniform vec3 u_lightColor; // цвет источника света
uniform vec3 u_vertColor; // цвет вершин
uniform int u_isVertCircle; // 1 - круг, 0 - квадрат
uniform vec3 u_edgesColor; // цвет ребер
uniform float u_dashSize; // размер штриха (0 для сплошной)


// значения из вершинного шейдера
in vec3 Camera_v;
in vec3 Light_v;
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

	else if(displayType == 1){
		// Нормаль
		vec3 N = Normal_v;
//		if(N == vec3(0, 0, 0)) {
			N = normalize(cross(dFdx(Vertex_v), dFdy(Vertex_v)));
//		}

		// Диффузная составляющая
		float diffuse = max(dot(Light_v, N), 0.0);

		vec3 res = (0.1 + diffuse) * u_lightColor;
		FragColor = vec4(res, 1.0);
	}

	else if(displayType == 2) {
		// мягкое освещение
		// Затенение методом Блинна-Фонга

		vec3 N = normalize(Normal_v);
//		if(N == vec3(0, 0, 0)) {
//			N = normalize(cross(dFdx(Vertex_v), dFdy(Vertex_v)));
//		}

		// Интенсивность диффузного отражения
		float diffuse = max(dot(normalize(Light_v), N), 0.0);

		// Вектор половины пути
		vec3 H = normalize(normalize(Light_v) + normalize(Camera_v));

		// Зеркальная составляющая
		float specular = pow(max(dot(H, N), 0.0), 32);

		vec3 res = (0.1 + diffuse + 0.5 * specular) * u_lightColor;
		FragColor = vec4(res, 1.0);

	}
}
