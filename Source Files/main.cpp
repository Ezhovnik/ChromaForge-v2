#include <iostream>
#include <cmath>
#include <../include/glad/glad.h>
#include <GLFW/glfw3.h>
#include <filesystem>

#include "../Header Files/shaderClass.h"
#include "../Header Files/VAO.h"
#include "../Header Files/VBO.h"
#include "../Header Files/EBO.h"

const GLuint WIDTH = 800, HEIGHT = 600;

// Координаты вершин
GLfloat vertices[] =
{
    -0.5f, -0.5f * float(sqrt(3)) / 3, 0.0f, // Нижний левый угол
    0.5f, -0.5f * float(sqrt(3)) / 3, 0.0f, // Нижний правый угол
    0.0f, 0.5f * float(sqrt(3)) * 2 / 3, 0.0f, // Верхний угол
    -0.5f / 2, 0.5f * float(sqrt(3)) / 6, 0.0f, // Левая средняя точка
    0.5f / 2, 0.5f * float(sqrt(3)) / 6, 0.0f, // Правая средняя точка
    0.0f, -0.5f * float(sqrt(3)) / 3, 0.0f // Нижняя средняя точка
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
    
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE); //Установка профайла для которого создается контекст
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



    Shader shaderProgram(
        "..\\Resource Files\\Shaders\\default.vert", 
        "..\\Resource Files\\Shaders\\default.frag"
    );



    VAO VAO1;
    VAO1.Bind();

    VBO VBO1(vertices, sizeof(vertices));
    EBO EBO1(indices, sizeof(indices));

    VAO1.LinkVBO(VBO1, 0);

    VAO1.Unbind();
    VBO1.Unbind();
    EBO1.Unbind();



    // Главный игровой цикл
    while(!glfwWindowShouldClose(window))
    {
        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        shaderProgram.Activate();

        VAO1.Bind();
        // Рисуем треугольник
        glDrawElements(GL_TRIANGLES, 9, GL_UNSIGNED_INT, 0);
        glfwSwapBuffers(window);
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
