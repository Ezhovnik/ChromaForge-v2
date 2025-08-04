#include <iostream>
#include <cmath>
#include <GL/glew.h>
#include <GLFW/glfw3.h>

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mode);

const GLuint WIDTH = 800, HEIGHT = 600;

// Исходный код вершинного шейдера
const char* vertexShaderSource = "#version 330 core\n"
"layout (location = 0) in vec3 aPos;\n"
"void main()\n"
"{\n"
"   gl_Position = vec4(aPos.x, aPos.y, aPos.z, 1.0);\n"
"}\0";

// Исходный код фрагментного шейдера
const char* fragmentShaderSource = "#version 330 core\n"
"out vec4 FragColor;\n"
"void main()\n"
"{\n"
"   FragColor = vec4(0.8f, 0.3f, 0.02f, 1.0f);\n"
"}\0";

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

    // Координаты вершин
    GLfloat vertices[] =
    {
        -0.5f, -0.5f * float(sqrt(3)) / 3, 0.0f, // Нижний левый угол
        0.5f, -0.5f * float(sqrt(3)) / 3, 0.0f, // Нижний правый угол
        0.0f, 0.5f * float(sqrt(3)) * 2 / 3, 0.0f // Верхний угол
    };
    
    // Создаём объект окна
    GLFWwindow* window = glfwCreateWindow(WIDTH, HEIGHT, "ChromaForge", nullptr, nullptr);
    if (window == nullptr)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);

    glfwSetKeyCallback(window, key_callback); 

    // Инициализируем GLEW
    glewExperimental = GL_TRUE;
    if (glewInit() != GLEW_OK)
    {
        std::cout << "Failed to initialize GLEW" << std::endl;
        return -1;
    }

    // Указываем OpenGL размер окна
    int width, height;
    glfwGetFramebufferSize(window, &width, &height);
    glViewport(0, 0, width, height);



    // Создаём объект вершинного шейдера
    GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
    // Прикрепляем исходный код вершинного шейдера к объекту
    glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
    // Компилируем вершинный шейдер в машинный код
    glCompileShader(vertexShader);

    // Создаём объект фрагментного шейдера
    GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    // Прикрепляем исходный код фрагментного шейдера к объекту
    glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
    // Компилируем фрагментный шейдер в машинный код
    glCompileShader(fragmentShader);

    // Создаём объект программы шейдеров
    GLuint shaderProgram = glCreateProgram();
    // Прикрепляем вершинный и фрагментный шейдеры к программе шейдеров
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    // Объединяем все шейдеры в программу шейдеров
    glLinkProgram(shaderProgram);

    // Удаляем уже ненужные объекты вершинного и фрагментного шейдеров 
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);



    // Создаём переменные для Vertex Array Object и Vertex Buffer Object
    GLuint VAO, VBO;

    // Создаём VAO и VBO
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);

    // Делаем VAO и VBO текущими Vertex Array Object и Vertex Buffer Object
    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    // Настраиваем Vertex Attribute так, чтобы OpenGL знал, как читать VBO
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    // Включаем Vertex Attribute, чтобы OpenGL знал, как использовать их
    glEnableVertexAttribArray(0);

    // Связываем VBO и VAO к 0, чтобы их случайно не изменить
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);



    // Главный игровой цикл
    while(!glfwWindowShouldClose(window))
    {
        glfwPollEvents();

        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        // Говорим OpenGl какие программы шейдеров мы хотим использовать
        glUseProgram(shaderProgram);
        // Привязываем VAO, чтобы OpenGL использовал VAO
        glBindVertexArray(VAO);
        // Рисуем треугольник
        glDrawArrays(GL_TRIANGLES, 0, 3);

        glfwSwapBuffers(window);
    }



    // Удаляем все объекты, которые мы создали, перед окончанием работы
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteProgram(shaderProgram);

    glfwDestroyWindow(window); // Удаляем окно перед окончание работы
    glfwTerminate(); // Очищаем ресурсы перед окончанием работы
    return 0;
}



void key_callback(GLFWwindow* window, int key, int scancode, int action, int mode)
{
    // Когда пользователь нажимает ESC, приложение закроется
    if(key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
        glfwSetWindowShouldClose(window, GL_TRUE);
    }
}