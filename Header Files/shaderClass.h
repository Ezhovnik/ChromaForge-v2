#ifndef SHADER_CLASS_H
#define SHADER_CLASS_H

#include <../include/glad/glad.h>
#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include <cerrno>

std::string get_file_contents(const char* filename);

class Shader {
    public:
        GLuint ID; // Идентификатор шейдерной программы
        Shader(const char* vertexFile, const char* fragmentFile); // Конструктор, создающий программу для шейдеров из двух разных шейдеров

        void Activate();
        void Delete();

    private:
        void compileErrors(unsigned int shader, const char* type); // Проверяем, правильно ли скомпилированы различные шейдеры
};

#endif