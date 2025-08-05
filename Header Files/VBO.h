#ifndef VBO_CLASS_H
#define VBO_CLASS_H

#include <../include/glad/glad.h>

class VBO {
    public:
        // Идентификатор для Vertex Buffer Object
        GLuint ID;
        // Конструктор, который создаёт Vertex Buffer Object и связывает его с вершинами
        VBO(GLfloat* vertices, GLsizeiptr size);

        void Bind();
        void Unbind();
        void Delete();

};

#endif