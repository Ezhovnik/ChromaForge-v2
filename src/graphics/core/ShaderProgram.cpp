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

uint ShaderProgram::getUniformLocation(const std::string& name) {
    auto it = uniformLocationCache.find(name);
    if (it != uniformLocationCache.end()) return it->second;

    uint loc = glGetUniformLocation(id, name.c_str());
    uniformLocationCache.emplace(name, loc);

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

void ShaderProgram::uniformMatrix(const std::string& name, glm::mat4 matrix) {
    glUniformMatrix4fv(getUniformLocation(name), 1, GL_FALSE, glm::value_ptr(matrix));
}

void ShaderProgram::uniform1i(const std::string& name, int x){

	glUniform1i(getUniformLocation(name), x);
}

void ShaderProgram::uniform1f(const std::string& name, float x){
	glUniform1f(getUniformLocation(name), x);
}

void ShaderProgram::uniform2f(const std::string& name, float x, float y){
	glUniform2f(getUniformLocation(name), x, y);
}

void ShaderProgram::uniform2f(const std::string& name, glm::vec2 xy) {
    uniform2f(name, xy.x, xy.y);
}

void ShaderProgram::uniform2i(const std::string& name, glm::ivec2 xy){
    glUniform2i(getUniformLocation(name), xy.x, xy.y);
}

void ShaderProgram::uniform3f(const std::string& name, float x, float y, float z){
	glUniform3f(getUniformLocation(name), x,y,z);
}

void ShaderProgram::uniform3f(const std::string& name, glm::vec3 xyz) {
    uniform3f(name, xyz.x, xyz.y, xyz.z);
}

inline auto shader_deleter = [](GLuint* shader) {
    glDeleteShader(*shader);
};
inline constexpr uint GL_LOG_LEN = 512;
using glshader = std::unique_ptr<GLuint, decltype(shader_deleter)>;

glshader compile_shader(GLenum type, const GLchar* source, const std::string& file) {
    GLint success;
    GLuint shader = glCreateShader(type);
    glShaderSource(shader, 1, &source, nullptr);
    glCompileShader(shader);
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success) {
        GLchar infoLog[GL_LOG_LEN];
        glGetShaderInfoLog(shader, GL_LOG_LEN, nullptr, infoLog);
        glDeleteShader(shader);
        LOG_CRITICAL("Shader '{}' compilation failed: {}", file, std::string(infoLog));
        throw std::runtime_error("Shader '"+ file +"' compilation failed: " + std::string(infoLog));
    }

    return glshader(new GLuint(shader), shader_deleter);
}

ShaderProgram* ShaderProgram::create(
    const std::string& vertexFile, 
    const std::string& fragmentFile, 
    const std::string& vertexSource, 
    const std::string& fragmentSource
) {
    // Преобразование строк в C-строки для OpenGL
    const GLchar* vShaderCode = vertexSource.c_str();
    const GLchar* fShaderCode = fragmentSource.c_str();

    glshader vertex = compile_shader(GL_VERTEX_SHADER, vShaderCode, vertexFile);
    glshader fragment = compile_shader(GL_FRAGMENT_SHADER, fShaderCode, fragmentFile);

    // Создание и линковка шейдерной программы
    GLint success;
    GLuint id = glCreateProgram();
    glAttachShader(id, *vertex);
    glAttachShader(id, *fragment);
    glLinkProgram(id);

    // Проверяем успешность линковки
    glGetProgramiv(id, GL_LINK_STATUS, &success);
    if (!success) {
        GLchar infoLog[GL_LOG_LEN];
        glGetProgramInfoLog(id, 512, nullptr, infoLog);
        LOG_CRITICAL("Shader Program linking failed. Info: {}", infoLog);

        glDeleteShader(*vertex);
        glDeleteShader(*fragment);
        glDeleteProgram(id);

        return nullptr;
    }

    // После успешной линковки шейдеры можно удалить
    glDeleteShader(*vertex);
    glDeleteShader(*fragment);

    return new ShaderProgram(id);
}
