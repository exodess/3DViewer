#version 430 core

uniform vec3 u_backgroundColor;

// значения из вершинного шейдера
in vec3 Camera_vertex;
in vec3 Vertex_view;

out vec4 FragColor;

void main() {
    // Координаты сетки (зависят от абсолютных мировых координат)
    vec2 coord = Camera_vertex.xz * 85.0;
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
    float distanceToCam = length(Vertex_view - Camera_vertex);

    // Плавное затухание
    float fade = clamp(1.0 - (distanceToCam / 80.0), 0.0, 1.0);

    vec3 oppositeColor = vec3(1.0) - u_backgroundColor;
    FragColor = vec4(oppositeColor, lineIntensity * fade);
    return;
}