#include <iostream>
#include <cmath>
#include <../include/glad/glad.h>
#include <GLFW/glfw3.h>
#include <filesystem>

#include "../Header Files/shaderClass.h"
#include "../Header Files/VAO.h"
#include "../Header Files/VBO.h"
#include "../Header Files/EBO.h"

const GLuint WIDTH = 800, HEIGHT = 800;

// Координаты вершин
GLfloat vertices[] =
{ //               КООРДИНАТЫ                  /        ЦВЕТА           //
	-0.5f, -0.5f * float(sqrt(3)) * 1 / 3, 0.0f,     0.8f, 0.3f,  0.02f, // Нижний левый угол
	 0.5f, -0.5f * float(sqrt(3)) * 1 / 3, 0.0f,     0.8f, 0.3f,  0.02f, // Нижний правый угол
	 0.0f,  0.5f * float(sqrt(3)) * 2 / 3, 0.0f,     1.0f, 0.6f,  0.32f, // Верхний угол
	-0.25f, 0.5f * float(sqrt(3)) * 1 / 6, 0.0f,     0.9f, 0.45f, 0.17f, // Левая средняя точка
	 0.25f, 0.5f * float(sqrt(3)) * 1 / 6, 0.0f,     0.9f, 0.45f, 0.17f, // Правая средняя точка
	 0.0f, -0.5f * float(sqrt(3)) * 1 / 3, 0.0f,     0.8f, 0.3f,  0.02f  // Нижняя средняя точка
};

// Индексы для порядка вершин
GLuint indices[] = {
    0, 3, 5, // Маленький левый треугольник
    3, 2, 4, // Маленький правый треугольник
    5, 4, 1 // Маленький верхний треугольник
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
    GLFWwindow* window = glfwCreateWindow(WIDTH, HEIGHT, "ChromaForge", nullptr, nullptr);
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
    VAO1.LinkAttrib(VBO1, 0, 3, GL_FLOAT, 6 * sizeof(float), (void*)0);
    VAO1.LinkAttrib(VBO1, 1, 3, GL_FLOAT, 6 * sizeof(float), (void*)(3 * sizeof(float)));
    // Отменяем привязку всех элементов, чтобы случайно не изменить их
    VAO1.Unbind();
    VBO1.Unbind();
    EBO1.Unbind();



    GLuint uniID = glGetUniformLocation(shaderProgram.ID, "scale");

    // Главный игровой цикл
    while(!glfwWindowShouldClose(window))
    {
        // Указываем цвет фона
        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        // Очищаем задний буфер и присваиваем ему новый цвет
        glClear(GL_COLOR_BUFFER_BIT);

        // Сообщаем OpenGL, какую программу шейдеров мы хотим использовать
        shaderProgram.Activate();
        // Присваиваем значение униформе. 
        // * NOTE: Это всегда нужно делать после активации программы шейдеров
        glUniform1f(uniID, 0.5f);
        VAO1.Bind();

        // Рисуем треугольник
        glDrawElements(GL_TRIANGLES, 9, GL_UNSIGNED_INT, 0);

        // Меняем местами задний и передний буферы, чтобы новый кадр появился на экране
        glfwSwapBuffers(window);
        // Обрабатываем все события GLFW
        glfwPollEvents();
    }



    // Удаляем все объекты, которые мы создали, перед окончанием работы
    VAO1.Delete();
    VBO1.Delete();
    EBO1.Delete();
    shaderProgram.Delete();

    glfwDestroyWindow(window); // Удаляем окно перед окончание работы
    glfwTerminate(); // Очищаем ресурсы перед окончанием работы
    return 0;
}
