#version 430 core

// Принимаем линии (по 2 вершины)
layout (triangles) in;

// Выдаем 4 вершины на ребро + 2 раза по 4 вершины на точки = 12 вершин
layout (triangle_strip, max_vertices = 27) out;

uniform float u_edgesSize;
uniform float u_vertSize;
uniform float u_aspectRatio;

// Данные, передаваемые в фрагментный шейдер
out vec2 vTexCoord;
out float vLineDist;
flat out int vIsPoint;

// Генерация квадрата (точки) вокруг позиции
void emitPoint(vec4 center, float size) {
    if (center.w <= 0.0) return; // Защита от точек за камерой
    vIsPoint = 1;
    float s = size / 2.0;
    vec2 point = center.xy / center.w;
    vec2 offset = vec2(s / u_aspectRatio, s);

    // Левый низ
    vTexCoord = vec2(0.0, 0.0);
    gl_Position = vec4((point + vec2(-offset.x, -offset.y)) * center.w, center.z, center.w);
    EmitVertex();

    // Правый низ
    vTexCoord = vec2(1.0, 0.0);
    gl_Position = vec4((point + vec2(offset.x, -offset.y)) * center.w, center.z, center.w);
    EmitVertex();

    // Левый верх
    vTexCoord = vec2(0.0, 1.0);
    gl_Position = vec4((point + vec2(-offset.x, offset.y)) * center.w, center.z, center.w);
    EmitVertex();

    // Правый верх
    vTexCoord = vec2(1.0, 1.0);
    gl_Position = vec4((point + vec2(offset.x, offset.y)) * center.w, center.z, center.w);
    EmitVertex();

    EndPrimitive();
}

// Генерация утолщенной линии (ребра)
void emitEdge(vec4 begin, vec4 end) {
    // Смягченная проверка w, чтобы не обрезать линии, подходящие близко к камере
    if (begin.w <= 0.001 || end.w <= 0.001) return;
    
    // Переходим в NDC
    vec2 point_begin = begin.xy / begin.w;
    vec2 point_end = end.xy / end.w;

    // Вычисляем длину и направление с учетом соотношения сторон
    float len = length((point_end - point_begin) * vec2(u_aspectRatio, 1.0));
    vec2 dir = normalize((point_end - point_begin) * vec2(u_aspectRatio, 1.0));
    
    // Нормаль к линии (для создания толщины)
    vec2 norm = vec2(-dir.y, dir.x) * u_edgesSize;
    vec2 offset = vec2(norm.x / u_aspectRatio, norm.y);

    vIsPoint = 0;
    
    // Возвращаем в Clip Space, умножая обратно на w
    vLineDist = 0.0;
    gl_Position = vec4((point_begin + offset) * begin.w, begin.z, begin.w); EmitVertex();
    gl_Position = vec4((point_begin - offset) * begin.w, begin.z, begin.w); EmitVertex();
    
    vLineDist = len;
    gl_Position = vec4((point_end + offset) * end.w, end.z, end.w); EmitVertex();
    gl_Position = vec4((point_end - offset) * end.w, end.z, end.w); EmitVertex();
    
    EndPrimitive();
}

void main() {
    // Три вершины образуют треугольник
    vec4 p0 = gl_in[0].gl_Position;
    vec4 p1 = gl_in[1].gl_Position;
    vec4 p2 = gl_in[2].gl_Position;

    // Отрисовываем вершины треугольника
    emitPoint(p0, u_vertSize);
    emitPoint(p1, u_vertSize);
    emitPoint(p2, u_vertSize);

    // Отрисовываем ребра между вершинами
    emitEdge(p0, p1);
    emitEdge(p1, p2);
    emitEdge(p0, p2);
}
