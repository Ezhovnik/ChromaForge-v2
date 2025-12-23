#include "ShaderProgram.h"

#include <iostream>
#include <exception>
#include <fstream>
#include <sstream>

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/gtc/type_ptr.hpp>

ShaderProgram::ShaderProgram(uint id) : id(id) {
}

ShaderProgram::~ShaderProgram(){
    glDeleteProgram(id);
}

void ShaderProgram::use() {
    glUseProgram(id);
}

void ShaderProgram::uniformMatrix(std::string name, glm::mat4 matrix) {
    GLuint transformLoc = glGetUniformLocation(id, name.c_str());
    glUniformMatrix4fv(transformLoc, 1, GL_FALSE, glm::value_ptr(matrix));
}