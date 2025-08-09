#include "../Header Files/Mesh.h"

// Конструктор меша (сетки)
Mesh::Mesh(std::vector <Vertex>& vertices, std::vector <GLuint>& indices, std::vector <Texture>& textures) {
    // Сохраняем переданные данные в объекте класса
    Mesh::vertices = vertices;
    Mesh::indices = indices;
    Mesh::textures = textures;

    VAO.Bind(); // Привязываем Vertex Array Object (VAO) для хранения настроек атрибутов вершин
    VBO VBO(vertices); // Создаём Vertex Buffer Object и связываем его с вершинами
    EBO EBO(indices); // Создаём Element Buffer Object и связываем его с индексами

    // Связываем VBO с VAO
    VAO.LinkAttrib(VBO, 0, 3, GL_FLOAT, sizeof(Vertex), (void*)0); // Координаты
    VAO.LinkAttrib(VBO, 1, 3, GL_FLOAT, sizeof(Vertex), (void*)(3 * sizeof(float))); // Цвет
    VAO.LinkAttrib(VBO, 2, 2, GL_FLOAT, sizeof(Vertex), (void*)(6 * sizeof(float))); // Текстурные координаты (s, t)
    VAO.LinkAttrib(VBO, 4, 3, GL_FLOAT, sizeof(Vertex), (void*)(8 * sizeof(float))); // Нормали
    
    // Отвязываем буферы, чтобы случайно не изменить их
    VAO.Unbind();
    VBO.Unbind();
    EBO.Unbind();
}

// Метод для отрисовки меша
void Mesh::Draw(Shader& shader, Camera& camera) {
    shader.Activate(); // Активируем шейдерную программу
    VAO.Bind(); // Привязываем VAO этого меша

    unsigned int numDiffuse = 0; // Счётчик для диффузных текстур
    unsigned int numSpecular = 0; // Счётчик для зеркальных текстур

    for (unsigned int i = 0; i < textures.size(); i++) {
        std::string num;
        std::string type = textures[i].type;
        
        if (type == "diffuse") {
            num = std::to_string(numDiffuse++); // Диффузная текстура (основной цвет)
        } else if (type == "specular") {
            num = std::to_string(numSpecular++); // Зеркальная текстура (карта бликов)
        }
        textures[i].texUnit(shader, (type + num).c_str(), i);
        textures[i].Bind();
    }

    glUniform3f(glGetUniformLocation(shader.ID, "camPos"), camera.Position.x, camera.Position.y, camera.Position.z);
    camera.Matrix(shader, "camMatrix");

    glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0);
}