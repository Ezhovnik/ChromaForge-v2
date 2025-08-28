#include "../Header Files/Skybox.h"
#include "../Header Files/Block.h"

#define GL_DEBUG_OUTPUT_SYNCHRONOUS_ARB 0x8242
#define GL_DEBUG_NEXT_LOGGED_MESSAGE_LENGTH_ARB 0x8243
#define GL_DEBUG_CALLBACK_FUNCTION_ARB 0x8244
#define GL_DEBUG_CALLBACK_USER_PARAM_ARB 0x8245
#define GL_DEBUG_SOURCE_API_ARB 0x8246
#define GL_DEBUG_SOURCE_WINDOW_SYSTEM_ARB 0x8247
#define GL_DEBUG_SOURCE_SHADER_COMPILER_ARB 0x8248
#define GL_DEBUG_SOURCE_THIRD_PARTY_ARB 0x8249
#define GL_DEBUG_SOURCE_APPLICATION_ARB 0x824A
#define GL_DEBUG_SOURCE_OTHER_ARB 0x824B
#define GL_DEBUG_TYPE_ERROR_ARB 0x824C
#define GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR_ARB 0x824D
#define GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR_ARB 0x824E
#define GL_DEBUG_TYPE_PORTABILITY_ARB 0x824F
#define GL_DEBUG_TYPE_PERFORMANCE_ARB 0x8250
#define GL_DEBUG_TYPE_OTHER_ARB 0x8251
#define GL_MAX_DEBUG_MESSAGE_LENGTH_ARB 0x9143
#define GL_MAX_DEBUG_LOGGED_MESSAGES_ARB 0x9144
#define GL_DEBUG_LOGGED_MESSAGES_ARB 0x9145
#define GL_DEBUG_SEVERITY_HIGH_ARB 0x9146
#define GL_DEBUG_SEVERITY_MEDIUM_ARB 0x9147
#define GL_DEBUG_SEVERITY_LOW_ARB 0x9148
typedef void (APIENTRY *GLDEBUGPROCARB)(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar* message, const void* userParam);
typedef void (APIENTRY *PFNGLDEBUGMESSAGECALLBACKARBPROC)(GLDEBUGPROCARB callback, void* userParam);

const GLuint WINDOW_WIDTH = 800, WINDOW_HEIGHT = 800;

// Координаты вершин светящегося куба
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

void APIENTRY glDebugOutputARB(GLenum source, 
                            GLenum type, 
                            GLuint id, 
                            GLenum severity, 
                            GLsizei length, 
                            const GLchar* message, 
                            const void* userParam)
{
    std::cerr << "\n----- OpenGL Debug -----\n";
    std::cerr << "Message: " << message << "\n";
    std::cerr << "Source: " << source << ", Type: " << type << ", ID: " << id << ", Severity: " << severity << "\n";
}


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

    glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GL_TRUE);
    
    // Создаём объект окна
    GLFWwindow* window = glfwCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "ChromaForge", nullptr, nullptr);
    if (window == nullptr)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }

    // Инициализируем иконку приложения
    GLFWimage icons[1];
    icons[0].pixels = stbi_load("../Resource Files/icon/icon.png", &icons[0].width, &icons[0].height, 0, 4);
    glfwSetWindowIcon(window, 1, icons);
    stbi_image_free(icons[0].pixels);

    glfwMakeContextCurrent(window); 

    gladLoadGL();

    // Указываем OpenGL размер окна
    int width, height;
    glfwGetFramebufferSize(window, &width, &height);
    glViewport(0, 0, width, height);

    if (glfwExtensionSupported("GL_ARB_debug_output")) {
        PFNGLDEBUGMESSAGECALLBACKARBPROC glDebugMessageCallbackARB = 
            (PFNGLDEBUGMESSAGECALLBACKARBPROC)glfwGetProcAddress("glDebugMessageCallbackARB");
        
        if (glDebugMessageCallbackARB) {
            glDebugMessageCallbackARB(glDebugOutputARB, nullptr);
            glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS_ARB);
        }
    }


    // Texture textures[] {
    //     Texture ("../Resource Files/Textures/Blocks/oak_planks.png", "diffuse", 0),
    //     Texture ("../Resource Files/Textures/Blocks/oak_planks_specular.png", "specular", 1)
    // };

    Shader cubeShaderProgram(
        "../Resource Files/Shaders/cube.vert", 
        "../Resource Files/Shaders/cube.frag"
    );

    Shader skyboxShaderProgram(
        "../Resource Files/Shaders/skybox.vert", 
        "../Resource Files/Shaders/skybox.frag"
    );

    // std::vector <Vertex> verts(vertices, vertices + sizeof(vertices) / sizeof(Vertex));
    // std::vector <GLuint> ind(indices, indices + sizeof(indices) / sizeof(GLuint));
    // std::vector <Texture> tex(textures, textures + sizeof(textures) / sizeof(Texture));

    // Создаём шейдерную программу для источников света
    // Shader lightShader(
    //     "..\\Resource Files\\Shaders\\light.vert", 
    //     "..\\Resource Files\\Shaders\\light.frag"
    // );
    // std::vector <Vertex> lightVerts(lightVertices, lightVertices + sizeof(lightVertices) / sizeof(Vertex));
    // std::vector <GLuint> lightInd(lightIndices, lightIndices + sizeof(lightIndices) / sizeof(GLuint));
    // Mesh light(lightVerts, lightInd, tex);

    // Настройка параметров освещения
    // glm::vec4 lightColor = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);

    // glm::vec3 lightPos = glm::vec3(0.0f, 1.1f, 0.0f);
    // glm::mat4 lightModel = glm::mat4(1.0f);
    // lightModel = glm::translate(lightModel, lightPos);

    // Создаём объекты блоков
    Block turf("turf");
    Block dirt("dirt");
    Block oakLog("oak_log");

    Skybox skybox("../Resource Files/Textures/Skybox/");

    glEnable(GL_DEPTH_TEST); // Включаем Depth Buffer

    // Включаем отсечение
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK); 

    Camera camera(WINDOW_WIDTH, WINDOW_HEIGHT, glm::vec3(0.0f, 0.0f, 2.0f)); // Создаём объект камеры



    // "Искры" (или "спарки") - "системная" единица измерения времени
    const int sparksInSecond = 100; // Количество спарков в секунде
    const int dayDurationInSparks = 24000; // Количество спарков в игровых сутках

    skybox.setDayDuration(dayDurationInSparks);

    float timesOfDayInSparks = 0.5f * dayDurationInSparks; // Момент суток в "спарках". Начинаем с полудня

    float lastFrame = glfwGetTime();
    float currFrame, deltaTime;


    // Главный игровой цикл
    while(!glfwWindowShouldClose(window))
    {
        currFrame = glfwGetTime();
        deltaTime = currFrame - lastFrame;
        lastFrame = currFrame;

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); // Очищаем задний буфер и присваиваем ему новый цвет

        camera.Inputs(window); // Управляет камерой
        camera.updateMatrix(45.0f, 0.1f, 100.0f, cubeShaderProgram); // Обновляем и экспортируем матрицу камеры в вершинный шейдер

        cubeShaderProgram.Activate();
        // glUniform3f(glGetUniformLocation(cubeShaderProgram.ID, "viewPos"), camera.Position.x, camera.Position.y, camera.Position.z);

        // Устанавливаем позицию света (это можно вынести из цикла, если свет не движется)
        // glUniform3f(glGetUniformLocation(cubeShaderProgram.ID, "lightPos"), lightPos.x, lightPos.y, lightPos.z);
        // glUniform4f(glGetUniformLocation(cubeShaderProgram.ID, "lightColor"), lightColor.x, lightColor.y, lightColor.z, lightColor.w);

        // Отрисовываем блоки
        turf.Draw(cubeShaderProgram, camera, glm::vec3(0.0f, 0.0f, 0.0f));
        oakLog.Draw(cubeShaderProgram, camera, glm::vec3(-1.0f, 0.0f, 0.0f));
        dirt.Draw(cubeShaderProgram, camera, glm::vec3(1.0f, 1.0f, 1.0f));

        // Рисуем источник света
        // lightShader.Activate();
        // glUniformMatrix4fv(glGetUniformLocation(lightShader.ID, "model"), 1, GL_FALSE, glm::value_ptr(lightModel));
        // glUniform4f(glGetUniformLocation(lightShader.ID, "lightColor"), lightColor.x, lightColor.y, lightColor.z, lightColor.w);
        // light.Draw(lightShader, camera);

        timesOfDayInSparks += deltaTime * sparksInSecond;
        if(timesOfDayInSparks > dayDurationInSparks) {
            timesOfDayInSparks -= dayDurationInSparks;
        }

        skyboxShaderProgram.Activate();
        skybox.Draw(skyboxShaderProgram, camera, timesOfDayInSparks);

        glfwSwapBuffers(window); // Меняем местами задний и передний буферы, чтобы новый кадр появился на экране
        glfwPollEvents(); // Обрабатываем все события GLFW
    }



    // Удаляем все объекты, которые мы создали, перед окончанием работы
    cubeShaderProgram.Delete();
    skyboxShaderProgram.Delete();
    // lightShader.Delete();

    glfwDestroyWindow(window); // Удаляем окно перед окончание работы
    glfwTerminate(); // Очищаем ресурсы перед окончанием работы
    return 0;
}
