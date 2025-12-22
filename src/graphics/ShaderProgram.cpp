#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include "ShaderProgram.h"

#include <exception>
#include <fstream>
#include <iostream>
#include <sstream>

ShaderProgram::ShaderProgram(uint id) : id(id) {
}

ShaderProgram::~ShaderProgram(){
    glDeleteProgram(id);
}

void ShaderProgram::use() {
    glUseProgram(id);
}