#include "ShaderProgram.h"

#include <iostream>
#include <exception>
#include <fstream>
#include <sstream>

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/gtc/type_ptr.hpp>

#include "../logger/Logger.h"

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

// Загружает матрицу 4x4 в uniform-переменную шейдера
void ShaderProgram::uniformMatrix(std::string name, glm::mat4 matrix) {
    GLint transformLoc = glGetUniformLocation(id, name.c_str());
    if (transformLoc == -1) {
        LOG_ERROR("Failed to find uniform variable '{}'", name);
        return;
    }

    glUniformMatrix4fv(transformLoc, 1, GL_FALSE, glm::value_ptr(matrix));
}

// Загружает целое число в uniform-переменную шейдера
void ShaderProgram::uniform1i(std::string name, int x){
	GLuint transformLoc = glGetUniformLocation(id, name.c_str());
    if (transformLoc == -1) {
        LOG_ERROR("Failed to find uniform variable '{}'", name);
        return;
    }
	glUniform1i(transformLoc, x);
}

// Загружает вещественное число в uniform-переменную шейдера
void ShaderProgram::uniform1f(std::string name, float x){
	GLuint transformLoc = glGetUniformLocation(id, name.c_str());
    if (transformLoc == -1) {
        LOG_ERROR("Failed to find uniform variable '{}'", name);
        return;
    }
	glUniform1f(transformLoc, x);
}


// Загружает два вещественных числа в uniform-переменную шейдера
void ShaderProgram::uniform2f(std::string name, float x, float y){
	GLuint transformLoc = glGetUniformLocation(id, name.c_str());
    if (transformLoc == -1) {
        LOG_ERROR("Failed to find uniform variable '{}'", name);
        return;
    }
	glUniform2f(transformLoc, x, y);
}

// Загружает три вещественных числа в uniform-переменную шейдера
void ShaderProgram::uniform3f(std::string name, float x, float y, float z){
	GLuint transformLoc = glGetUniformLocation(id, name.c_str());
    if (transformLoc == -1) {
        LOG_ERROR("Failed to find uniform variable '{}'", name);
        return;
    }
	glUniform3f(transformLoc, x,y,z);
}
