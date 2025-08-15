#version 330 core
layout (location = 0) in vec3 aPos; // Получаем позицию (координаты)
layout (location = 1) in vec2 aTex; // Получаем координаты текстур
layout (location = 2) in vec3 aNormal; // Получаем нормальный вектор

out vec2 texCoord; // Выводим координаты текстур для фрагментного шейдера
out vec3 Normal; // Выводим нормальный вектор для фрагментного шейдера
out vec3 currPos; // Выводим текущую позицию для фрагментного шейдера

uniform mat4 camMatrix;
uniform mat4 model;

void main()
{
   currPos = vec3(model * vec4(aPos, 1.0f)); 
   texCoord = aTex;
   Normal = aNormal;

   gl_Position = camMatrix * vec4(currPos, 1.0); // Выводит положение (координаты) всех вершин
}