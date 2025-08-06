#version 330 core

// Выводим цвета в формате RGBA
out vec4 FragColor;

in vec3 color; // Получаем цвет из вершинного шейдера
in vec2 texCoord; // Вводим координат текстуры из вершинного шейдера
in float texID;

uniform sampler2D tex0;
uniform sampler2D tex1;
uniform sampler2D tex2;

void main()
{
   if (texID == 2.0) {
      FragColor = texture(tex2, texCoord);
   } else if (texID == 1.0) {
      FragColor = texture(tex1, texCoord);
   } else {
      FragColor = texture(tex0, texCoord);
   }
}