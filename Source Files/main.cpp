#include "../Header Files/Mesh.h"
#include "../Header Files/Block.h"

const GLuint WINDOW_WIDTH = 800, WINDOW_HEIGHT = 800;

// // Координаты вершин светящегося куба
// Vertex lightVertices[] =
// {
// 	Vertex{glm::vec3(-0.1f, -0.1f,  0.1f)},
// 	Vertex{glm::vec3(-0.1f, -0.1f, -0.1f)},
// 	Vertex{glm::vec3(0.1f, -0.1f, -0.1f)},
// 	Vertex{glm::vec3(0.1f, -0.1f,  0.1f)},
// 	Vertex{glm::vec3(-0.1f,  0.1f,  0.1f)},
// 	Vertex{glm::vec3(-0.1f,  0.1f, -0.1f)},
// 	Vertex{glm::vec3(0.1f,  0.1f, -0.1f)},
// 	Vertex{glm::vec3(0.1f,  0.1f,  0.1f)}
// };

// GLuint lightIndices[] =
// {
// 	0, 1, 2,
// 	0, 2, 3,
// 	0, 4, 7,
// 	0, 7, 3,
// 	3, 7, 6,
// 	3, 6, 2,
// 	2, 6, 5,
// 	2, 5, 1,
// 	1, 5, 4,
// 	1, 4, 0,
// 	4, 5, 6,
// 	4, 6, 7
// };

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



    // Texture textures[] {
    //     Texture ("../Resource Files/Textures/Blocks/oak_planks.png", "diffuse", 0),
    //     Texture ("../Resource Files/Textures/Blocks/oak_planks_specular.png", "specular", 1)
    // };

    Shader cubeShaderProgram(
        "..\\Resource Files\\Shaders\\cube.vert", 
        "..\\Resource Files\\Shaders\\cube.frag"
    );
    // std::vector <Vertex> verts(vertices, vertices + sizeof(vertices) / sizeof(Vertex));
    // std::vector <GLuint> ind(indices, indices + sizeof(indices) / sizeof(GLuint));
    // std::vector <Texture> tex(textures, textures + sizeof(textures) / sizeof(Texture));

    // // Создаём шейдерную программу для источников света
    // Shader lightShader(
    //     "..\\Resource Files\\Shaders\\light.vert", 
    //     "..\\Resource Files\\Shaders\\light.frag"
    // );
    // std::vector <Vertex> lightVerts(lightVertices, lightVertices + sizeof(lightVertices) / sizeof(Vertex));
    // std::vector <GLuint> lightInd(lightIndices, lightIndices + sizeof(lightIndices) / sizeof(GLuint));
    // Mesh light(lightVerts, lightInd, tex);

    // // Настройка параметров освещения
    // glm::vec4 lightColor = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);

    // glm::vec3 lightPos = glm::vec3(0.0f, 1.1f, 0.0f);
    // glm::mat4 lightModel = glm::mat4(1.0f);
    // lightModel = glm::translate(lightModel, lightPos);

    Block turf("turf");
    Block dirt("dirt");

    glEnable(GL_DEPTH_TEST); // Включаем Depth Buffer

    Camera camera(WINDOW_WIDTH, WINDOW_HEIGHT, glm::vec3(0.0f, 0.0f, 2.0f)); // Создаём объект камеры



    // Главный игровой цикл
    while(!glfwWindowShouldClose(window))
    {
        glClearColor(0.2f, 0.3f, 0.3f, 1.0f); // Указываем цвет фона
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); // Очищаем задний буфер и присваиваем ему новый цвет

        camera.Inputs(window); // Управляет камерой
        camera.updateMatrix(45.0f, 0.1f, 100.0f, cubeShaderProgram); // Обновляем и экспортируем матрицу камеры в вершинный шейдер

        cubeShaderProgram.Activate();

        glUniform3f(glGetUniformLocation(cubeShaderProgram.ID, "viewPos"), camera.Position.x, camera.Position.y, camera.Position.z);

        // Устанавливаем позицию света (это можно вынести из цикла, если свет не движется)
        // glUniform3f(glGetUniformLocation(cubeShaderProgram.ID, "lightPos"), lightPos.x, lightPos.y, lightPos.z);
        // glUniform4f(glGetUniformLocation(cubeShaderProgram.ID, "lightColor"), lightColor.x, lightColor.y, lightColor.z, lightColor.w);

        turf.Draw(cubeShaderProgram, camera, glm::vec3(0.0f, 0.0f, 0.0f));
        turf.Draw(cubeShaderProgram, camera, glm::vec3(-1.0f, -1.0f, -1.0f));
        dirt.Draw(cubeShaderProgram, camera, glm::vec3(1.0f, 1.0f, 1.0f));

        // Рисуем источник света
        // lightShader.Activate();
        // glUniformMatrix4fv(glGetUniformLocation(lightShader.ID, "model"), 1, GL_FALSE, glm::value_ptr(lightModel));
        // glUniform4f(glGetUniformLocation(lightShader.ID, "lightColor"), lightColor.x, lightColor.y, lightColor.z, lightColor.w);
        // light.Draw(lightShader, camera);
        
        glfwSwapBuffers(window); // Меняем местами задний и передний буферы, чтобы новый кадр появился на экране
        glfwPollEvents(); // Обрабатываем все события GLFW
    }



    // Удаляем все объекты, которые мы создали, перед окончанием работы
    cubeShaderProgram.Delete();
    // lightShader.Delete();

    glfwDestroyWindow(window); // Удаляем окно перед окончание работы
    glfwTerminate(); // Очищаем ресурсы перед окончанием работы
    return 0;
}
