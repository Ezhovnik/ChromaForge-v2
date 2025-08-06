#version 330 core
layout (location = 0) in vec3 aPos; // Получаем позицию (координаты)
layout (location = 1) in vec3 aColor; // Получаем цвет
layout (location = 2) in vec2 aTex; // Получаем координаты текстур
layout (location = 3) in float aTexID;

out vec3 color; // Выводим цвет для фрагментного шейдера
out vec2 texCoord; // Выводим координаты текстур для фрагментного шейдера
out float texID;

uniform mat4 camMatrix;

void main()
{
   gl_Position = camMatrix * vec4(aPos, 1.0); // Выводит положение (координаты) всех вершин
   color = aColor;
   texCoord = aTex;
   texID = aTexID;
}