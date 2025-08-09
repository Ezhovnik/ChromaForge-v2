#ifndef VBO_CLASS_H
#define VBO_CLASS_H

#include <../include/glm/glm.hpp>
#include <../include/glad/glad.h>
#include <vector>

struct Vertex {
    glm::vec3 position;
    glm::vec3 color;
    glm::vec2 texUV;
    GLfloat texID;
    glm::vec3 normal;
};

class VBO {
    public:
        // Идентификатор для Vertex Buffer Object
        GLuint ID;
        // Конструктор, который создаёт Vertex Buffer Object и связывает его с вершинами
        VBO(std::vector<Vertex>& vertices);

        void Bind();
        void Unbind();
        void Delete();

};

#endif