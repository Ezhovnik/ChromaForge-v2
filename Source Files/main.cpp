#include <iostream>
#include <cmath>
#include <../include/glad/glad.h>
#include <GLFW/glfw3.h>
#include <filesystem>
#include "../include/stb/stb_image.h"
#include <../include/glm/glm.hpp>
#include <../include/glm/gtc/matrix_transform.hpp>
#include <../include/glm/gtc/type_ptr.hpp>

#include "../Header Files/shaderClass.h"
#include "../Header Files/VAO.h"
#include "../Header Files/VBO.h"
#include "../Header Files/EBO.h"
#include "../Header Files/Texture.h"
#include "../Header Files/Camera.h"

const GLuint WINDOW_WIDTH = 800, WINDOW_HEIGHT = 800;

// Vertices coordinates
GLfloat vertices[] = {
    // Front face (0-3)
    -0.5f, -0.5f,  0.5f,  0.83f,0.70f,0.44f, 0.0f,0.0f, 0.0f,  0.0f, 0.0f, 1.0f,  // 0
     0.5f, -0.5f,  0.5f,  0.83f,0.70f,0.44f, 1.0f,0.0f, 0.0f,  0.0f, 0.0f, 1.0f,  // 1
     0.5f,  0.5f,  0.5f,  0.83f,0.70f,0.44f, 1.0f,1.0f, 0.0f,  0.0f, 0.0f, 1.0f,  // 2
    -0.5f,  0.5f,  0.5f,  0.83f,0.70f,0.44f, 0.0f,1.0f, 0.0f,  0.0f, 0.0f, 1.0f,  // 3

    // Back face (4-7)
    -0.5f, -0.5f, -0.5f,  0.83f,0.70f,0.44f, 1.0f,0.0f, 0.0f,  0.0f, 0.0f,-1.0f,  // 4
     0.5f, -0.5f, -0.5f,  0.83f,0.70f,0.44f, 0.0f,0.0f, 0.0f,  0.0f, 0.0f,-1.0f,  // 5
     0.5f,  0.5f, -0.5f,  0.83f,0.70f,0.44f, 0.0f,1.0f, 0.0f,  0.0f, 0.0f,-1.0f,  // 6
    -0.5f,  0.5f, -0.5f,  0.83f,0.70f,0.44f, 1.0f,1.0f, 0.0f,  0.0f, 0.0f,-1.0f,  // 7

    // Top face (8-11)
    -0.5f,  0.5f,  0.5f,  0.92f,0.86f,0.76f, 0.0f,0.0f, 1.0f,  0.0f, 1.0f, 0.0f,  // 8
     0.5f,  0.5f,  0.5f,  0.92f,0.86f,0.76f, 1.0f,0.0f, 1.0f,  0.0f, 1.0f, 0.0f,  // 9
     0.5f,  0.5f, -0.5f,  0.92f,0.86f,0.76f, 1.0f,1.0f, 1.0f,  0.0f, 1.0f, 0.0f,  // 10
    -0.5f,  0.5f, -0.5f,  0.92f,0.86f,0.76f, 0.0f,1.0f, 1.0f,  0.0f, 1.0f, 0.0f,  // 11

    // Bottom face (12-15)
    -0.5f, -0.5f,  0.5f,  0.83f,0.70f,0.44f, 1.0f,1.0f, 2.0f,  0.0f,-1.0f, 0.0f,  // 12
     0.5f, -0.5f,  0.5f,  0.83f,0.70f,0.44f, 0.0f,1.0f, 2.0f,  0.0f,-1.0f, 0.0f,  // 13
     0.5f, -0.5f, -0.5f,  0.83f,0.70f,0.44f, 0.0f,0.0f, 2.0f,  0.0f,-1.0f, 0.0f,  // 14
    -0.5f, -0.5f, -0.5f,  0.83f,0.70f,0.44f, 1.0f,0.0f, 2.0f,  0.0f,-1.0f, 0.0f,  // 15

    // Right face (16-19)
     0.5f, -0.5f,  0.5f,  0.83f,0.70f,0.44f, 0.0f,0.0f, 0.0f,  1.0f, 0.0f, 0.0f,  // 16
     0.5f, -0.5f, -0.5f,  0.83f,0.70f,0.44f, 1.0f,0.0f, 0.0f,  1.0f, 0.0f, 0.0f,  // 17
     0.5f,  0.5f, -0.5f,  0.83f,0.70f,0.44f, 1.0f,1.0f, 0.0f,  1.0f, 0.0f, 0.0f,  // 18
     0.5f,  0.5f,  0.5f,  0.83f,0.70f,0.44f, 0.0f,1.0f, 0.0f,  1.0f, 0.0f, 0.0f,  // 19

    // Left face (20-23)
    -0.5f, -0.5f,  0.5f,  0.83f,0.70f,0.44f, 1.0f,0.0f, 0.0f, -1.0f, 0.0f, 0.0f,  // 20
    -0.5f, -0.5f, -0.5f,  0.83f,0.70f,0.44f, 0.0f,0.0f, 0.0f, -1.0f, 0.0f, 0.0f,  // 21
    -0.5f,  0.5f, -0.5f,  0.83f,0.70f,0.44f, 0.0f,1.0f, 0.0f, -1.0f, 0.0f, 0.0f,  // 22
    -0.5f,  0.5f,  0.5f,  0.83f,0.70f,0.44f, 1.0f,1.0f, 0.0f, -1.0f, 0.0f, 0.0f   // 23
};

// Indices for vertices order (6 faces with 2 triangles each = 12 triangles)
GLuint indices[] =
{
    // Front face
    0, 1, 2,
    2, 3, 0,
    
    // Back face
    5, 4, 7,
    7, 6, 5,
    
    // Top face
    8, 9, 10,
    10, 11, 8,
    
    // Bottom face
    14, 13, 12,
    12, 15, 14,
    
    // Right face
    16, 17, 18,
    18, 19, 16,
    
    // Left face
    21, 20, 23,
    23, 22, 21
};

GLfloat lightVertices[] =
{ //     COORDINATES     //
	-0.1f, -0.1f,  0.1f,
	-0.1f, -0.1f, -0.1f,
	 0.1f, -0.1f, -0.1f,
	 0.1f, -0.1f,  0.1f,
	-0.1f,  0.1f,  0.1f,
	-0.1f,  0.1f, -0.1f,
	 0.1f,  0.1f, -0.1f,
	 0.1f,  0.1f,  0.1f
};

GLuint lightIndices[] =
{
	0, 1, 2,
	0, 2, 3,
	0, 4, 7,
	0, 7, 3,
	3, 7, 6,
	3, 6, 2,
	2, 6, 5,
	2, 5, 1,
	1, 5, 4,
	1, 4, 0,
	4, 5, 6,
	4, 6, 7
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



    // Создаём Vertex Array Object и привязываем его
    VAO VAO1;
    VAO1.Bind();

    // Создаём Vertex Buffer Object и связываем его с вершинами
    VBO VBO1(vertices, sizeof(vertices));
    // Создаём Element Buffer Object и связываем его с индексами
    EBO EBO1(indices, sizeof(indices));

    // Связываем VBO с VAO
    VAO1.LinkAttrib(VBO1, 0, 3, GL_FLOAT, 12 * sizeof(float), (void*)0); // Координаты
    VAO1.LinkAttrib(VBO1, 1, 3, GL_FLOAT, 12 * sizeof(float), (void*)(3 * sizeof(float))); // Цвет
    VAO1.LinkAttrib(VBO1, 2, 2, GL_FLOAT, 12 * sizeof(float), (void*)(6 * sizeof(float))); // Текстурные координаты (s, t)
    VAO1.LinkAttrib(VBO1, 3, 1, GL_FLOAT, 12 * sizeof(float), (void*)(8 * sizeof(float))); // texID
    VAO1.LinkAttrib(VBO1, 4, 3, GL_FLOAT, 12 * sizeof(float), (void*)(9 * sizeof(float))); // Нормали
    // Отменяем привязку всех элементов, чтобы случайно не изменить их
    VAO1.Unbind();
    VBO1.Unbind();
    EBO1.Unbind();



    // Создаём шейдерную программу для источников света
    Shader lightShader(
        "..\\Resource Files\\Shaders\\light.vert", 
        "..\\Resource Files\\Shaders\\light.frag"
    );

    VAO lightVAO;
    lightVAO.Bind();

    VBO lightVBO(lightVertices, sizeof(lightVertices));
    EBO lightEBO(lightIndices, sizeof(lightIndices));

    lightVAO.LinkAttrib(lightVBO, 0, 3, GL_FLOAT, 3 * sizeof(float), (void*)0);

    lightVAO.Unbind();
    lightVBO.Unbind();
    lightEBO.Unbind();


    // Настройка параметров освещения
    glm::vec4 lightColor = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);

    glm::vec3 lightPos = glm::vec3(0.0f, 1.1f, 0.0f);
    glm::mat4 lightModel = glm::mat4(1.0f);
    lightModel = glm::translate(lightModel, lightPos);

    glm::vec3 cubePos = glm::vec3(0.0f, 0.0f, 0.0f);
    glm::mat4 cubeModel = glm::mat4(1.0f);
    cubeModel = glm::translate(cubeModel, cubePos);

    // Активация шейдеров и передача uniform-переменных
    lightShader.Activate();
    glUniformMatrix4fv(glGetUniformLocation(lightShader.ID, "model"), 1, GL_FALSE, glm::value_ptr(lightModel));
    glUniform4f(glGetUniformLocation(lightShader.ID, "lightColor"), lightColor.x, lightColor.y, lightColor.z, lightColor.w);
    shaderProgram.Activate();
    glUniformMatrix4fv(glGetUniformLocation(shaderProgram.ID, "model"), 1, GL_FALSE, glm::value_ptr(cubeModel));
    glUniform4f(glGetUniformLocation(shaderProgram.ID, "lightColor"), lightColor.x, lightColor.y, lightColor.z, lightColor.w);
	glUniform3f(glGetUniformLocation(shaderProgram.ID, "lightPos"), lightPos.x, lightPos.y, lightPos.z);



    // Текстуры
    Texture grass_block_side("../Resource Files/Textures/grass_block_side.png", GL_TEXTURE_2D, GL_TEXTURE0, GL_RGB, GL_UNSIGNED_BYTE);
	Texture grass_block_top("../Resource Files/Textures/grass_block_top.png", GL_TEXTURE_2D, GL_TEXTURE1, GL_RGB, GL_UNSIGNED_BYTE);
    Texture grass_block_bottom("../Resource Files/Textures/dirt.png", GL_TEXTURE_2D, GL_TEXTURE2, GL_RGB, GL_UNSIGNED_BYTE);
    grass_block_side.texUnit(shaderProgram, "tex0", 0);
    grass_block_top.texUnit(shaderProgram, "tex1", 1);
    grass_block_bottom.texUnit(shaderProgram, "tex2", 2);
    
    glEnable(GL_DEPTH_TEST); // Включаем Depth Buffer

    Camera camera(WINDOW_WIDTH, WINDOW_HEIGHT, glm::vec3(0.0f, 0.0f, 2.0f)); // Создаём объект камеры

    // Главный игровой цикл
    while(!glfwWindowShouldClose(window))
    {
        glClearColor(0.2f, 0.3f, 0.3f, 1.0f); // Указываем цвет фона
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); // Очищаем задний буфер и присваиваем ему новый цвет

        camera.Inputs(window); // Управляет камерой
        camera.updateMatrix(45.0f, 0.1f, 100.0f, shaderProgram); // Обновляем и экспортируем матрицу камеры в вершинный шейдер
        
        shaderProgram.Activate(); // Сообщаем OpenGL, какую программу шейдеров мы хотим использовать
        glUniform3f(glGetUniformLocation(shaderProgram.ID, "camPos"), camera.Position.x, camera.Position.y, camera.Position.z);
        camera.Matrix(shaderProgram, "camMatrix");

        glActiveTexture(GL_TEXTURE0);
        grass_block_side.Bind();
        glActiveTexture(GL_TEXTURE1);
        grass_block_top.Bind(); 
        glActiveTexture(GL_TEXTURE2);
        grass_block_bottom.Bind(); 

        VAO1.Bind();
        glDrawElements(GL_TRIANGLES, sizeof(indices)/sizeof(int), GL_UNSIGNED_INT, 0); // Рисуем треугольник
        
        lightShader.Activate();
        camera.Matrix(lightShader, "camMatrix");
        lightVAO.Bind();
        glDrawElements(GL_TRIANGLES, sizeof(lightIndices)/sizeof(int), GL_UNSIGNED_INT, 0);
        
        glfwSwapBuffers(window); // Меняем местами задний и передний буферы, чтобы новый кадр появился на экране
        glfwPollEvents(); // Обрабатываем все события GLFW
    }



    // Удаляем все объекты, которые мы создали, перед окончанием работы
    VAO1.Delete();
    VBO1.Delete();
    EBO1.Delete();
    lightVAO.Delete();
    lightVBO.Delete();
    lightEBO.Delete();
    shaderProgram.Delete();
    lightShader.Delete();
    grass_block_side.Delete();

    glfwDestroyWindow(window); // Удаляем окно перед окончание работы
    glfwTerminate(); // Очищаем ресурсы перед окончанием работы
    return 0;
}
