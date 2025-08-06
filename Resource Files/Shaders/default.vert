#version 330 core
layout (location = 0) in vec3 aPos; // Получаем позицию (координаты)
layout (location = 1) in vec3 aColor; // Получаем цвет
layout (location = 2) in vec2 aTex; // Получаем координаты текстур

out vec3 color; // Выводим цвет для фрагментного шейдера
out vec2 texCoord; // Выводим координаты текстур для фрагментного шейдера

uniform float scale; // Управляем масштабом вершин

// Вводим матрицы, необходимые для 3D-просмотра с перспективой
uniform mat4 model;
uniform mat4 view;
uniform mat4 proj;

void main()
{
   gl_Position = proj * view * model * vec4(aPos * (scale + 1.0), 1.0); // Выводит положение (координаты) всех вершин
   color = aColor;
   texCoord = aTex;
}