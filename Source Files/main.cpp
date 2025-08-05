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

    int widthImg, heightImg, numColCh;
    stbi_set_flip_vertically_on_load(true);
    unsigned char* bytes = stbi_load("../Resource Files/Textures/grass_block_side.png", &widthImg, &heightImg, &numColCh, 0);

    GLuint texture;
    glGenTextures(1, &texture);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texture);

    // Установка параметров текстуры
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

    // Проверяем формат текстуры
    GLenum format = GL_RGB;
    if (numColCh == 4) format = GL_RGBA;

    glTexImage2D(GL_TEXTURE_2D, 0, format, widthImg, heightImg, 0, format, GL_UNSIGNED_BYTE, bytes);
    glGenerateMipmap(GL_TEXTURE_2D);

    stbi_image_free(bytes);
    glBindTexture(GL_TEXTURE_2D, 0);

    GLuint tex0Uni = glGetUniformLocation(shaderProgram.ID, "tex0");
    shaderProgram.Activate();
    glUniform1i(tex0Uni, 0);

    // Главный игровой цикл
    while(!glfwWindowShouldClose(window))
    {
        glClearColor(0.2f, 0.3f, 0.3f, 1.0f); // Указываем цвет фона
        glClear(GL_COLOR_BUFFER_BIT); // Очищаем задний буфер и присваиваем ему новый цвет
        shaderProgram.Activate(); // Сообщаем OpenGL, какую программу шейдеров мы хотим использовать
        glUniform1f(uniID, 0.0f); // Присваиваем значение униформе. NOTE: Это всегда нужно делать после активации программы шейдеров
        glBindTexture(GL_TEXTURE_2D, texture);
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
    glDeleteTextures(1, &texture);

    glfwDestroyWindow(window); // Удаляем окно перед окончание работы
    glfwTerminate(); // Очищаем ресурсы перед окончанием работы
    return 0;
}
