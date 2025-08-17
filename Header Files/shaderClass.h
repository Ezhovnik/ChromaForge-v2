#ifndef SHADER_CLASS_H
#define SHADER_CLASS_H

#define GLM_ENABLE_EXPERIMENTAL

#include <../include/glad/glad.h>
#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include <cerrno>
#include <../include/glm/glm.hpp>
#include <../include/glm/gtc/matrix_transform.hpp>
#include <../include/glm/gtc/type_ptr.hpp>
#include <../include/glm/gtx/rotate_vector.hpp>
#include <../include/glm/gtx/vector_angle.hpp>

std::string get_file_contents(const char* filename);

class Shader {
    public:
        GLuint ID; // Идентификатор шейдерной программы
        Shader(const char* vertexFile, const char* fragmentFile); // Конструктор, создающий программу для шейдеров из двух разных шейдеров

        void setMat4(const std::string &name, const glm::mat4 &mat);
        void setFloat(const std::string &name, const float &num);

        void Activate();
        void Delete();
    private:
        void compileErrors(unsigned int shader, const char* type); // Проверяем, правильно ли скомпилированы различные шейдеры
};

#endif