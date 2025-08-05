#ifndef VAO_CLASS_H
#define VAO_CLASS_H

#include <../include/glad/glad.h>
#include "VBO.h"

class VAO {
    public:
        // Идентификатор для Vertex Array Object
        GLuint ID;
        // Конструктор, генерирующий идентификатор VAO
        VAO();

        void LinkAttrib(VBO VBO, GLuint layout, GLuint numComponents, GLenum type, GLsizeiptr stride, void* offset);
        void Bind();
        void Unbind();
        void Delete();
};

#endif