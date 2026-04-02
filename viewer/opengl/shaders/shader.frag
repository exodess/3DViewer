#version 430 core

out vec4 FragColor;

// значения из геометрического шейдера
in vec2 vTexCoord;
in float vLineDist;
flat in int vIsPoint;

uniform vec3 u_vertColor; // цвет вершин
uniform int u_isVertCircle; // 1 - круг, 0 - квадрат
uniform vec3 u_edgesColor; // цвет ребер
uniform float u_dashSize; // размер штриха (0 для сплошной)

void main() {

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
