#include "../Header Files/SkyboxMesh.h"

SkyboxMesh::SkyboxMesh(std::vector <Vertex>& vertices, std::vector <GLuint>& indices, CubeTexture& skyboxTextureDay, CubeTexture& skyboxTextureNight) {
    // Сохраняем переданные данные в объекте класса
    SkyboxMesh::vertices = vertices;
    SkyboxMesh::indices = indices;
    SkyboxMesh::skyboxTextureDay = skyboxTextureDay;
    SkyboxMesh::skyboxTextureNight = skyboxTextureNight;

    VAO.Bind(); // Привязываем Vertex Array Object (VAO) для хранения настроек атрибутов вершин
    VBO VBO(vertices); // Создаём Vertex Buffer Object и связываем его с вершинами
    EBO EBO(indices); // Создаём Element Buffer Object и связываем его с индексами

    // Связываем VBO с VAO
    VAO.LinkAttrib(VBO, 0, 3, GL_FLOAT, sizeof(Vertex), (void*)0); // Координаты
    
    // Отвязываем буферы, чтобы случайно не изменить их
    VAO.Unbind();
    VBO.Unbind();
    EBO.Unbind();
}

// Метод для отрисовки меша
void SkyboxMesh::Draw(Shader& shader, Camera& camera, float timesOfDay) {
    VAO.Bind();
    skyboxTextureDay.texUnit(shader, "skyboxDay", 0);
    skyboxTextureDay.Bind();
    skyboxTextureNight.texUnit(shader, "skyboxNight", 1);
    skyboxTextureNight.Bind();

    shader.setFloat("timesOfDay", timesOfDay);

    glm::mat4 view = glm::mat4(glm::mat3(camera.view));
    glm::mat4 projection = camera.projection;
    shader.setMat4("view", view);
    shader.setMat4("projection", projection);
    
    glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0);
}