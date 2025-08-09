#ifndef EBO_CLASS_H
#define EBO_CLASS_H

#include <../include/glad/glad.h>
#include <vector>

class EBO {
    public:
        // Идентификатор Elements Buffer Object
        GLuint ID;
        // Конструктор, который создаёт Elements Buffer Object и связывает его с индексами
        EBO(std::vector<GLuint>& indices);

        void Bind();
        void Unbind();
        void Delete();

};

#endif