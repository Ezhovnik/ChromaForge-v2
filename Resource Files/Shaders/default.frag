#version 330 core

// Выводим цвета в формате RGBA
out vec4 FragColor;

in vec3 color; // Получаем цвет из вершинного шейдера
in vec2 texCoord; // Вводим координат текстуры из вершинного шейдера

uniform sampler2D tex0; // Получаем текстурный блок из основной функции

void main()
{
   FragColor = texture(tex0, texCoord);
}