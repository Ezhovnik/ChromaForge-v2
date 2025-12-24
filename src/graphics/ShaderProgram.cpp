#include "ShaderProgram.h"

#include <iostream>
#include <exception>
#include <fstream>
#include <sstream>

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/gtc/type_ptr.hpp>

// Конструктор шейдерной программы
ShaderProgram::ShaderProgram(uint id) : id(id) {
}

// Деструктор шейдерной программы
ShaderProgram::~ShaderProgram(){
    glDeleteProgram(id);
}

// Активация шейдерной программы для использования в текущем контексте отрисовки
void ShaderProgram::use() {
    glUseProgram(id);
}

void ShaderProgram::uniformMatrix(std::string name, glm::mat4 matrix) {
    GLint transformLoc = glGetUniformLocation(id, name.c_str());
    if (transformLoc == -1) {
        std::cerr << "Failed to find uniform variable " << name << std::endl;
        return;
    }

    glUniformMatrix4fv(transformLoc, 1, GL_FALSE, glm::value_ptr(matrix));
}