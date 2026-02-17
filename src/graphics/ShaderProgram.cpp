#include "ShaderProgram.h"

#include <exception>
#include <fstream>
#include <sstream>

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/gtc/type_ptr.hpp>

#include "../coders/GLSLExtension.h"
#include "../logger/Logger.h"

GLSLExtension* ShaderProgram::preprocessor = new GLSLExtension();

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

void ShaderProgram::uniform2f(std::string name, glm::vec2 xy) {
    uniform2f(name, xy.x, xy.y);
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

void ShaderProgram::uniform3f(std::string name, glm::vec3 xyz) {
    uniform3f(name, xyz.x, xyz.y, xyz.z);
}

// Основная функция для загрузки и компиляции шейдерной программы
ShaderProgram* ShaderProgram::loadShaderProgram(std::string vertexFile, std::string fragmentFile, std::string vertexSource, std::string fragmentSource) {
    vertexSource = preprocessor->process(std::filesystem::path(vertexFile), vertexSource);
    fragmentSource = preprocessor->process(std::filesystem::path(fragmentFile), fragmentSource);
    
    // Преобразование строк в C-строки для OpenGL
    const GLchar* vShaderCode = vertexSource.c_str();
    const GLchar* fShaderCode = fragmentSource.c_str();

    // Переменные для работы с OpenGL
    GLuint vertex, fragment; // Идентификаторы шейдеров
    GLint success; // Статус успешности операции
    GLchar infoLog[512]; // Буфер для сообщений об ошибке

    // Компиляция вершинного шейдера
    vertex = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertex, 1, &vShaderCode, nullptr);
    glCompileShader(vertex);

    // Проверяем успешность компиляции
    glGetShaderiv(vertex, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(vertex, 512, nullptr, infoLog);
        LOG_CRITICAL("Vertex shader '{}' compililation failed\n{}", vertexFile, infoLog);
        return nullptr;
    }

    // Компиляция фрагментного шейдера
    fragment = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragment, 1, &fShaderCode, nullptr);
    glCompileShader(fragment);

    // Проверяем успешность компиляции
    glGetShaderiv(fragment, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(fragment, 512, nullptr, infoLog);
        LOG_CRITICAL("Fragment shader '{}' compililation failed. Info: {}", fragmentFile, infoLog);
        return nullptr;
    }

    // Создание и линковка шейдерной программы
    GLuint id = glCreateProgram();
    glAttachShader(id, vertex);
    glAttachShader(id, fragment);
    glLinkProgram(id);

    // Проверяем успешность линковки
    glGetProgramiv(id, GL_LINK_STATUS, &success);
    if (!success) {
        glGetProgramInfoLog(id, 512, nullptr, infoLog);
        LOG_CRITICAL("Shader Program linking failed. Info: {}", infoLog);

        glDeleteShader(vertex);
        glDeleteShader(fragment);
        glDeleteProgram(id);

        return nullptr;
    }

    // После успешной линковки шейдеры можно удалить
    glDeleteShader(vertex);
    glDeleteShader(fragment);

    return new ShaderProgram(id); // Создаем и возвращаем объект шейдерной программы
}
