#include "../Header Files/Mesh.h"

const GLuint WINDOW_WIDTH = 800, WINDOW_HEIGHT = 800;

// Координаты вершин
Vertex vertices[] = {
    // Передняя сторона
    Vertex{glm::vec3(-0.5f, -0.5f, 0.5f), glm::vec3(0.83f, 0.70f, 0.44f), glm::vec2(0.0f, 0.0f), 0.0f, glm::vec3(0.0f, 0.0f, 1.0f)}, // 0
    Vertex{glm::vec3(0.5f, -0.5f, 0.5f), glm::vec3(0.83f, 0.70f, 0.44f), glm::vec2(1.0f, 0.0f), 0.0f, glm::vec3(0.0f, 0.0f, 1.0f)}, // 1
    Vertex{glm::vec3(0.5f, 0.5f, 0.5f), glm::vec3(0.83f, 0.70f, 0.44f), glm::vec2(1.0f, 1.0f), 0.0f, glm::vec3(0.0f, 0.0f, 1.0f)}, // 2
    Vertex{glm::vec3(-0.5f, 0.5f, 0.5f), glm::vec3(0.83f, 0.70f, 0.44f), glm::vec2(0.0f, 1.0f), 0.0f, glm::vec3(0.0f, 0.0f, 1.0f)}, // 3

    // Задняя сторона
    Vertex{glm::vec3(-0.5f, -0.5f, -0.5f), glm::vec3(0.83f, 0.70f, 0.44f), glm::vec2(1.0f, 0.0f), 0.0f, glm::vec3(0.0f, 0.0f, -1.0f)}, // 4
    Vertex{glm::vec3(0.5f, -0.5f, -0.5f), glm::vec3(0.83f, 0.70f, 0.44f), glm::vec2(0.0f, 0.0f), 0.0f, glm::vec3(0.0f, 0.0f, -1.0f)}, // 5
    Vertex{glm::vec3(0.5f, 0.5f, -0.5f), glm::vec3(0.83f, 0.70f, 0.44f), glm::vec2(0.0f, 1.0f), 0.0f, glm::vec3(0.0f, 0.0f, -1.0f)}, // 6
    Vertex{glm::vec3(-0.5f, 0.5f, -0.5f), glm::vec3(0.83f, 0.70f, 0.44f), glm::vec2(1.0f, 1.0f), 0.0f, glm::vec3(0.0f, 0.0f, -1.0f)}, // 7

    // Верхняя сторона
    Vertex{glm::vec3(-0.5f, 0.5f, 0.5f), glm::vec3(0.83f, 0.70f, 0.44f), glm::vec2(0.0f, 0.0f), 0.0f, glm::vec3(0.0f, 1.0f, 0.0f)}, // 8
    Vertex{glm::vec3(0.5f, 0.5f, 0.5f), glm::vec3(0.83f, 0.70f, 0.44f), glm::vec2(1.0f, 0.0f), 0.0f, glm::vec3(0.0f, 1.0f, 0.0f)}, // 9
    Vertex{glm::vec3(0.5f, 0.5f, -0.5f), glm::vec3(0.83f, 0.70f, 0.44f), glm::vec2(1.0f, 1.0f), 0.0f, glm::vec3(0.0f, 1.0f, 0.0f)}, // 10
    Vertex{glm::vec3(-0.5f, 0.5f, -0.5f), glm::vec3(0.83f, 0.70f, 0.44f), glm::vec2(0.0f, 1.0f), 0.0f, glm::vec3(0.0f, 1.0f, 0.0f)}, // 11

    // Нижняя сторона
    Vertex{glm::vec3(-0.5f, -0.5f, 0.5f), glm::vec3(0.83f, 0.70f, 0.44f), glm::vec2(1.0f, 1.0f), 0.0f, glm::vec3(0.0f, -1.0f, 0.0f)}, // 12
    Vertex{glm::vec3(0.5f, -0.5f, 0.5f), glm::vec3(0.83f, 0.70f, 0.44f), glm::vec2(0.0f, 1.0f), 0.0f, glm::vec3(0.0f, -1.0f, 0.0f)}, // 13
    Vertex{glm::vec3(0.5f, -0.5f, -0.5f), glm::vec3(0.83f, 0.70f, 0.44f), glm::vec2(0.0f, 0.0f), 0.0f, glm::vec3(0.0f, -1.0f, 0.0f)}, // 14
    Vertex{glm::vec3(-0.5f, -0.5f, -0.5f), glm::vec3(0.83f, 0.70f, 0.44f), glm::vec2(1.0f, 0.0f), 0.0f, glm::vec3(0.0f, -1.0f, 0.0f)}, // 15

    // Правая сторона
    Vertex{glm::vec3(0.5f, -0.5f, 0.5f), glm::vec3(0.83f, 0.70f, 0.44f), glm::vec2(0.0f, 0.0f), 0.0f, glm::vec3(1.0f, 0.0f, 0.0f)}, // 16
    Vertex{glm::vec3(0.5f, -0.5f, -0.5f), glm::vec3(0.83f, 0.70f, 0.44f), glm::vec2(1.0f, 0.0f), 0.0f, glm::vec3(1.0f, 0.0f, 0.0f)}, // 17
    Vertex{glm::vec3(0.5f, 0.5f, -0.5f), glm::vec3(0.83f, 0.70f, 0.44f), glm::vec2(1.0f, 1.0f), 0.0f, glm::vec3(1.0f, 0.0f, 0.0f)}, // 18
    Vertex{glm::vec3(0.5f, 0.5f, 0.5f), glm::vec3(0.83f, 0.70f, 0.44f), glm::vec2(0.0f, 1.0f), 0.0f, glm::vec3(1.0f, 0.0f, 0.0f)}, // 19

    // Правая сторона
    Vertex{glm::vec3(-0.5f, -0.5f, 0.5f), glm::vec3(0.83f, 0.70f, 0.44f), glm::vec2(1.0f, 0.0f), 0.0f, glm::vec3(-1.0f, 0.0f, 0.0f)}, // 20
    Vertex{glm::vec3(-0.5f, -0.5f, -0.5f), glm::vec3(0.83f, 0.70f, 0.44f), glm::vec2(0.0f, 0.0f), 0.0f, glm::vec3(-1.0f, 0.0f, 0.0f)}, // 21
    Vertex{glm::vec3(-0.5f, 0.5f, -0.5f), glm::vec3(0.83f, 0.70f, 0.44f), glm::vec2(0.0f, 1.0f), 0.0f, glm::vec3(-1.0f, 0.0f, 0.0f)}, // 22
    Vertex{glm::vec3(-0.5f, 0.5f, 0.5f), glm::vec3(0.83f, 0.70f, 0.44f), glm::vec2(1.0f, 1.0f), 0.0f, glm::vec3(-1.0f, 0.0f, 0.0f)} // 23
};

GLuint indices[] =
{
    // Передняя сторона
    0, 1, 2,
    2, 3, 0,
    
    // Задняя сторона
    5, 4, 7,
    7, 6, 5,
    
    // Верхняя сторона
    8, 9, 10,
    10, 11, 8,
    
    // Нижняя сторона
    14, 13, 12,
    12, 15, 14,
    
    // Правая сторона
    16, 17, 18,
    18, 19, 16,
    
    // Левая сторона
    21, 20, 23,
    23, 22, 21
};

// Координаты вершин светящегося куба
Vertex lightVertices[] =
{
	Vertex{glm::vec3(-0.1f, -0.1f,  0.1f)},
	Vertex{glm::vec3(-0.1f, -0.1f, -0.1f)},
	Vertex{glm::vec3(0.1f, -0.1f, -0.1f)},
	Vertex{glm::vec3(0.1f, -0.1f,  0.1f)},
	Vertex{glm::vec3(-0.1f,  0.1f,  0.1f)},
	Vertex{glm::vec3(-0.1f,  0.1f, -0.1f)},
	Vertex{glm::vec3(0.1f,  0.1f, -0.1f)},
	Vertex{glm::vec3(0.1f,  0.1f,  0.1f)}
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



    Texture textures[] {
        Texture ("../Resource Files/Textures/oak_planks.png", "diffuse", 0, GL_RGB, GL_UNSIGNED_BYTE),
        Texture ("../Resource Files/Textures/oak_planks_specular.png", "specular", 1, GL_RED, GL_UNSIGNED_BYTE)
    };

    // Создаём объект шейдера с использованием шейдеров default.vert и default.frag
    Shader shaderProgram(
        "..\\Resource Files\\Shaders\\default.vert", 
        "..\\Resource Files\\Shaders\\default.frag"
    );
    std::vector <Vertex> verts(vertices, vertices + sizeof(vertices) / sizeof(Vertex));
    std::vector <GLuint> ind(indices, indices + sizeof(indices) / sizeof(GLuint));
    std::vector <Texture> tex(textures, textures + sizeof(textures) / sizeof(Texture));
    Mesh planks(verts, ind, tex);



    // Создаём шейдерную программу для источников света
    Shader lightShader(
        "..\\Resource Files\\Shaders\\light.vert", 
        "..\\Resource Files\\Shaders\\light.frag"
    );
    std::vector <Vertex> lightVerts(lightVertices, lightVertices + sizeof(lightVertices) / sizeof(Vertex));
    std::vector <GLuint> lightInd(lightIndices, lightIndices + sizeof(lightIndices) / sizeof(GLuint));
    Mesh light(lightVerts, lightInd, tex);



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



    glEnable(GL_DEPTH_TEST); // Включаем Depth Buffer

    Camera camera(WINDOW_WIDTH, WINDOW_HEIGHT, glm::vec3(0.0f, 0.0f, 2.0f)); // Создаём объект камеры



    // Главный игровой цикл
    while(!glfwWindowShouldClose(window))
    {
        glClearColor(0.2f, 0.3f, 0.3f, 1.0f); // Указываем цвет фона
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); // Очищаем задний буфер и присваиваем ему новый цвет

        camera.Inputs(window); // Управляет камерой
        camera.updateMatrix(45.0f, 0.1f, 100.0f, shaderProgram); // Обновляем и экспортируем матрицу камеры в вершинный шейдер

        planks.Draw(shaderProgram, camera);
        light.Draw(lightShader, camera);
        
        glfwSwapBuffers(window); // Меняем местами задний и передний буферы, чтобы новый кадр появился на экране
        glfwPollEvents(); // Обрабатываем все события GLFW
    }



    // Удаляем все объекты, которые мы создали, перед окончанием работы
    shaderProgram.Delete();
    lightShader.Delete();

    glfwDestroyWindow(window); // Удаляем окно перед окончание работы
    glfwTerminate(); // Очищаем ресурсы перед окончанием работы
    return 0;
}
