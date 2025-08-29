#include "../Header Files/BlockMesh.h"

BlockMesh::BlockMesh(std::vector <Vertex>& vertices, std::vector <GLuint>& indices, CubeTexture& cubeTexture) {
    // Сохраняем переданные данные в объекте класса
    BlockMesh::vertices = vertices;
    BlockMesh::indices = indices;
    BlockMesh::cubeTexture = cubeTexture;

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
void BlockMesh::Draw(Shader& shader, Camera& camera) {
    VAO.Bind();
    cubeTexture.texUnit(shader, "cubeMap", 0);
    cubeTexture.Bind();

    glUniform3f(glGetUniformLocation(shader.ID, "camPos"), camera.Position.x, camera.Position.y, camera.Position.z);
    camera.Matrix(shader, "camMatrix");
    
    glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0);
}