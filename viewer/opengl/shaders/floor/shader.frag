#version 430 core

uniform vec3 u_backgroundColor;

// значения из вершинного шейдера
in vec3 Camera_v;
in vec3 Vertex_v;

out vec4 FragColor;

void main() {
    // Координаты сетки (абсолютные мировые)
    vec2 coord = Vertex_v.xz / 2.0;
    vec2 derivative = fwidth(coord);

    vec2 grid = abs(fract(coord - 0.5) - 0.5) / derivative;
    float lineAlpha = min(grid.x, grid.y);
    float lineIntensity = 1.0 - min(lineAlpha, 1.0);

    // Прозрачный пол
    if (lineIntensity <= 0.01) {
        discard;
    }

    float distanceToCam = length(Vertex_v - Camera_v);

    // Плавное затухание
    float fade = clamp(1.0 - (distanceToCam / 80.0), 0.0, 1.0);

    vec3 oppositeColor = vec3(1.0) - u_backgroundColor;
    FragColor = vec4(oppositeColor, lineIntensity * fade);
    return;
}