#include "shader_loader.h"

#include <iostream>
#include <fstream>
#include <sstream>

#include <GL/glew.h>

#include "../graphics/ShaderProgram.h"

std::string loadFile(std::string filename) {
    std::string code;
    std::ifstream file;

    file.exceptions(std::ifstream::badbit);
    try {
        file.open(filename);
        std::stringstream stream;

        stream << file.rdbuf();

        file.close();

        code = stream.str();
    } catch (std::ifstream::failure& e) {
        std::cerr << "ERROR::SHADER::FILE_NOT_SUCCESFULLY_READ" << std::endl;
        return nullptr;
    }

    return code;
}

ShaderProgram* loadShaderProgram(std::string vertexFile, std::string fragmentFile) {
    std::string vShaderString = loadFile(vertexFile);
    std::string fShaderString = loadFile(fragmentFile);
    
    if (vShaderString.empty() || fShaderString.empty()) {
        std::cerr << "Failed to load shader files" << std::endl;
        return nullptr;
    }
    
    const GLchar* vShaderCode = vShaderString.c_str();
    const GLchar* fShaderCode = fShaderString.c_str();

    if (vShaderCode == nullptr || fShaderCode == nullptr) {
        std::cerr << "Failed to load shader's file" << std::endl;
        return nullptr;
    }

    GLuint vertex, fragment;
    GLint success;
    GLchar infoLog[512];

    vertex = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertex, 1, &vShaderCode, nullptr);
    glCompileShader(vertex);
    glGetShaderiv(vertex, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(vertex, 512, nullptr, infoLog);
        std::cerr << "SHADER::VERTEX: compililation failed" << std::endl;
        std::cerr << infoLog << std::endl;
        return nullptr;
    }

    fragment = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragment, 1, &fShaderCode, nullptr);
    glCompileShader(fragment);
    glGetShaderiv(fragment, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(fragment, 512, nullptr, infoLog);
        std::cerr << "SHADER::FRAGMENT: compililation failed" << std::endl;
        std::cerr << infoLog << std::endl;
        return nullptr;
    }

    GLuint id = glCreateProgram();
    glAttachShader(id, vertex);
    glAttachShader(id, fragment);
    glLinkProgram(id);

    glGetProgramiv(id, GL_LINK_STATUS, &success);
    if (!success) {
        glGetProgramInfoLog(id, 512, nullptr, infoLog);
        std::cerr << "SHADER::PROGRAM: linking failed" << std::endl;
        std::cerr << infoLog << std::endl;

        glDeleteShader(vertex);
        glDeleteShader(fragment);
        glDeleteProgram(id);

        return nullptr;
    }

    glDeleteShader(vertex);
    glDeleteShader(fragment);

    return new ShaderProgram(id);
}