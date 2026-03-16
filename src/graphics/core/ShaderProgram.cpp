#include "ShaderProgram.h"

#include <exception>
#include <fstream>
#include <sstream>
#include <filesystem>

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/gtc/type_ptr.hpp>

#include "../../coders/GLSLExtension.h"
#include "../../debug/Logger.h"

GLSLExtension* ShaderProgram::preprocessor = new GLSLExtension();

int ShaderProgram::getUniformLocation(const std::string& name) {
    auto it = uniformLocationCache.find(name);
    if (it != uniformLocationCache.end()) return it->second;

    int loc = glGetUniformLocation(id, name.c_str());
    uniformLocationCache[name] = loc;

    if (loc == -1) {
        if (warnedUniforms.find(name) == warnedUniforms.end()) {
            LOG_WARN("Failed to find uniform variable '{}'", name);
            warnedUniforms.insert(name);
        }
    }
    return loc;
}

ShaderProgram::ShaderProgram(uint id) : id(id) {
}

ShaderProgram::~ShaderProgram(){
    glDeleteProgram(id);
}

void ShaderProgram::use() {
    glUseProgram(id);
}

void ShaderProgram::uniformMatrix(std::string name, glm::mat4 matrix) {
    GLint transformLoc = getUniformLocation(name);
    glUniformMatrix4fv(transformLoc, 1, GL_FALSE, glm::value_ptr(matrix));
}

void ShaderProgram::uniform1i(std::string name, int x){
	GLuint transformLoc = getUniformLocation(name);
	glUniform1i(transformLoc, x);
}

void ShaderProgram::uniform1f(std::string name, float x){
	GLuint transformLoc = getUniformLocation(name);
	glUniform1f(transformLoc, x);
}

void ShaderProgram::uniform2f(std::string name, float x, float y){
	GLuint transformLoc = getUniformLocation(name);
	glUniform2f(transformLoc, x, y);
}

void ShaderProgram::uniform2f(std::string name, glm::vec2 xy) {
    uniform2f(name, xy.x, xy.y);
}

void ShaderProgram::uniform2i(std::string name, glm::ivec2 xy){
    GLuint transformLoc = getUniformLocation(name);
    glUniform2i(transformLoc, xy.x, xy.y);
}

void ShaderProgram::uniform3f(std::string name, float x, float y, float z){
	GLuint transformLoc = getUniformLocation(name);
	glUniform3f(transformLoc, x,y,z);
}

void ShaderProgram::uniform3f(std::string name, glm::vec3 xyz) {
    uniform3f(name, xyz.x, xyz.y, xyz.z);
}

ShaderProgram* ShaderProgram::create(
    std::string vertexFile, 
    std::string fragmentFile, 
    std::string vertexSource, 
    std::string fragmentSource
) {
    // Обрабатываем исходники через препроцессор (поддержка #include и макросов)
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

    return new ShaderProgram(id);
}
