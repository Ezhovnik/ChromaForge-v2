#include <iostream>
#include <cmath>
#include <../include/glad/glad.h>
#include <GLFW/glfw3.h>
#include <filesystem>
#include "../include/stb/stb_image.h"

#include "../Header Files/shaderClass.h"
#include "../Header Files/VAO.h"
#include "../Header Files/VBO.h"
#include "../Header Files/EBO.h"
#include "../Header Files/Texture.h"

const GLuint WINDOW_WIDTH = 800, WINDOW_HEIGHT = 800;

// Координаты вершин
GLfloat vertices[] =
{ //     КООРДИНАТЫ      /        ЦВЕТА        /      ТЕКСТУРЫ     //
    -0.5f, -0.5f, 0.0f,     1.0f, 0.0f, 0.0f,     0.0f, 0.0f, // Нижний левый угол
    -0.5f,  0.5f, 0.0f,     0.0f, 1.0f, 0.0f,     0.0f, 1.0f, // Верхний левый угол
     0.5f,  0.5f, 0.0f,     0.0f, 0.0f, 1.0f,     1.0f, 1.0f, // Верхний правый угол
     0.5f, -0.5f, 0.0f,     1.0f, 1.0f, 1.0f,     1.0f, 0.0f  // Нижний правый угол
};

// Индексы для порядка вершин
GLuint indices[] = {
    0, 2, 1,
    0, 3, 2

};

int main()
{
    // Инициализация GLFW
    if (!glfwInit())
    {
        std::cout << "Failed to initialize GLFW" << std::endl;
        return -1;
    }

    // Задаётся минимальная требуемая версия OpenGL.
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3); // Номер мажорной версии
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3); // Номер минорной версии
    
    // Сообщаем GLFW, что мы используем профиль CORE
	// Это значит, что у нас есть только современные функции
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    glfwWindowHint(GLFW_RESIZABLE, GL_FALSE); //Выключение возможности изменения размера окна
    
    // Создаём объект окна
    GLFWwindow* window = glfwCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "ChromaForge", nullptr, nullptr);
    if (window == nullptr)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window); 

    gladLoadGL();

    // Указываем OpenGL размер окна
    int width, height;
    glfwGetFramebufferSize(window, &width, &height);
    glViewport(0, 0, width, height);



    // Создаём объект шейдера с использованием шейдеров default.vert и default.frag
    Shader shaderProgram(
        "..\\Resource Files\\Shaders\\default.vert", 
        "..\\Resource Files\\Shaders\\default.frag"
    );



    // Создаём Vertex Array Object и привязывает его
    VAO VAO1;
    VAO1.Bind();

    // Создаём Vertex Buffer Object и связывает его с вершинами
    VBO VBO1(vertices, sizeof(vertices));
    // Создаём Element Buffer Object и связывает его с индексами
    EBO EBO1(indices, sizeof(indices));

    // Связываем VBO с VAO
    VAO1.LinkAttrib(VBO1, 0, 3, GL_FLOAT, 8 * sizeof(float), (void*)0); // Позиция
    VAO1.LinkAttrib(VBO1, 1, 3, GL_FLOAT, 8 * sizeof(float), (void*)(3 * sizeof(float))); // Цвет
    VAO1.LinkAttrib(VBO1, 2, 2, GL_FLOAT, 8 * sizeof(float), (void*)(6 * sizeof(float))); // Текстуры
    // Отменяем привязку всех элементов, чтобы случайно не изменить их
    VAO1.Unbind();
    VBO1.Unbind();
    EBO1.Unbind();


    // Получает идентификатор формы под названием «scale»
    GLuint uniID = glGetUniformLocation(shaderProgram.ID, "scale");

    // Текстуры
    Texture grass_block_side("../Resource Files/Textures/grass_block_side.png", GL_TEXTURE_2D, GL_TEXTURE0, GL_RGB, GL_UNSIGNED_BYTE);
	grass_block_side.texUnit(shaderProgram, "tex0", 0);

    // Главный игровой цикл
    while(!glfwWindowShouldClose(window))
    {
        glClearColor(0.2f, 0.3f, 0.3f, 1.0f); // Указываем цвет фона
        glClear(GL_COLOR_BUFFER_BIT); // Очищаем задний буфер и присваиваем ему новый цвет
        shaderProgram.Activate(); // Сообщаем OpenGL, какую программу шейдеров мы хотим использовать
        glUniform1f(uniID, 0.0f); // Присваиваем значение униформе. NOTE: Это всегда нужно делать после активации программы шейдеров
        grass_block_side.Bind();
        VAO1.Bind();
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0); // Рисуем треугольник
        glfwSwapBuffers(window); // Меняем местами задний и передний буферы, чтобы новый кадр появился на экране
        glfwPollEvents(); // Обрабатываем все события GLFW
    }



    // Удаляем все объекты, которые мы создали, перед окончанием работы
    VAO1.Delete();
    VBO1.Delete();
    EBO1.Delete();
    shaderProgram.Delete();
    grass_block_side.Delete();

    glfwDestroyWindow(window); // Удаляем окно перед окончание работы
    glfwTerminate(); // Очищаем ресурсы перед окончанием работы
    return 0;
}
