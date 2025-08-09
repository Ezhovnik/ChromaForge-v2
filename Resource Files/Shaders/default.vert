#version 330 core
layout (location = 0) in vec3 aPos; // Получаем позицию (координаты)
layout (location = 1) in vec3 aColor; // Получаем цвет
layout (location = 2) in vec2 aTex; // Получаем координаты текстур
layout (location = 3) in float aTexID; // Получаем номер текстуры
layout (location = 4) in vec3 aNormal; // Получаем нормальный вектор

out vec3 color; // Выводим цвет для фрагментного шейдера
out vec2 texCoord; // Выводим координаты текстур для фрагментного шейдера
out float texID; // Выводим номер текстуры для фрагментного шейдера
out vec3 Normal; // Выводим нормальный вектор для фрагментного шейдера
out vec3 currPos; // Выводим текущую позицию для фрагментного шейдера

uniform mat4 camMatrix;
uniform mat4 model;

void main()
{
   currPos = vec3(model * vec4(aPos, 1.0f)); 
   color = aColor;
   texCoord = aTex;
   texID = aTexID;
   Normal = aNormal;

   gl_Position = camMatrix * vec4(currPos, 1.0); // Выводит положение (координаты) всех вершин
}