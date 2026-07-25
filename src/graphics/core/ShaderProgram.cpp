#include <graphics/core/ShaderProgram.h>

#include <exception>
#include <fstream>
#include <sstream>
#include <filesystem>

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/gtc/type_ptr.hpp>

#include <coders/GLSLExtension.h>
#include <debug/Logger.h>

GLSLExtension* ShaderProgram::preprocessor = new GLSLExtension();
ShaderProgram* ShaderProgram::used = nullptr;

uint ShaderProgram::getUniformLocation(const std::string& name) {
    auto it = uniformLocationCache.find(name);
    if (it != uniformLocationCache.end()) return it->second;

    uint loc = glGetUniformLocation(id, name.c_str());
    uniformLocationCache.try_emplace(name, loc);

    if (loc == -1) {
        if (warnedUniforms.find(name) == warnedUniforms.end()) {
            LOG_WARN("Failed to find uniform variable '{}'", name);
            warnedUniforms.insert(name);
        }
    }
    return loc;
}

ShaderProgram::ShaderProgram(
    uint id,
    Source&& vertexSource,
    Source&& fragmentSource
) : id(id),
    vertexSource(std::move(vertexSource)),
    fragmentSource(std::move(fragmentSource)) {
}

ShaderProgram::~ShaderProgram() {
    glDeleteProgram(id);
}

void ShaderProgram::use() {
    used = this;
    glUseProgram(id);
}

void ShaderProgram::uniformMatrix(const std::string& name, const glm::mat4& matrix) {
    glUniformMatrix4fv(
        getUniformLocation(name), 1, GL_FALSE, glm::value_ptr(matrix)
    );
}

void ShaderProgram::uniformMatrix(const std::string& name, const glm::mat3& matrix) {
    glUniformMatrix3fv(
        getUniformLocation(name), 1, GL_FALSE, glm::value_ptr(matrix)
    );
}

void ShaderProgram::uniform1i(const std::string& name, int x) {
	glUniform1i(getUniformLocation(name), x);
}

void ShaderProgram::uniform1f(const std::string& name, float x) {
	glUniform1f(getUniformLocation(name), x);
}

void ShaderProgram::uniform2f(const std::string& name, float x, float y) {
	glUniform2f(getUniformLocation(name), x, y);
}

void ShaderProgram::uniform2f(const std::string& name, const glm::vec2& xy) {
    glUniform2f(getUniformLocation(name), xy.x, xy.y);
}

void ShaderProgram::uniform2i(const std::string& name, const glm::ivec2& xy) {
    glUniform2i(getUniformLocation(name), xy.x, xy.y);
}

void ShaderProgram::uniform3f(const std::string& name, float x, float y, float z){
	glUniform3f(getUniformLocation(name), x, y, z);
}

void ShaderProgram::uniform3f(const std::string& name, const glm::vec3& xyz) {
    glUniform3f(getUniformLocation(name), xyz.x, xyz.y, xyz.z);
}

void ShaderProgram::uniform4f(const std::string& name, const glm::vec4& xyzw) {
    glUniform4f(getUniformLocation(name), xyzw.x, xyzw.y, xyzw.z, xyzw.w);
}

void ShaderProgram::uniform1v(const std::string& name, int length, const int* v) {
    glUniform1iv(getUniformLocation(name), length, v);
}

void ShaderProgram::uniform1v(const std::string& name, int length, const float* v) {
    glUniform1fv(getUniformLocation(name), length, v);
}

void ShaderProgram::uniform2v(const std::string& name, int length, const float* v) {
    glUniform2fv(getUniformLocation(name), length, v);
}

void ShaderProgram::uniform3v(const std::string& name, int length, const float* v) {
    glUniform3fv(getUniformLocation(name), length, v);
}

void ShaderProgram::uniform4v(const std::string& name, int length, const float* v) {
    glUniform4fv(getUniformLocation(name), length, v);
}

static inline auto shader_deleter = [](GLuint* shader) {
    glDeleteShader(*shader);
    delete shader;
};

inline constexpr uint GL_LOG_LEN = 512;
using glshader = std::unique_ptr<GLuint, decltype(shader_deleter)>;

glshader compile_shader(
    GLenum type, const GLchar* source, const std::string& file
) {
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

static GLuint compile_program(
    const ShaderProgram::Source& vertexSource,
    const ShaderProgram::Source& fragmentSource,
    const std::vector<std::string>& defines
) {
    auto& preprocessor = *ShaderProgram::preprocessor;

    auto vertexCode = std::move(
        preprocessor.process(vertexSource.file, vertexSource.code, false, defines).code
    );
    auto fragmentCode = std::move(
        preprocessor.process(fragmentSource.file, fragmentSource.code, false, defines).code
    );

    const GLchar* vCode = vertexCode.c_str();
    const GLchar* fCode = fragmentCode.c_str();

    glshader vertex = compile_shader(
        GL_VERTEX_SHADER, vCode, vertexSource.file
    );
    glshader fragment = compile_shader(
        GL_FRAGMENT_SHADER, fCode, fragmentSource.file
    );

    GLint success;
    GLuint program = glCreateProgram();
    glAttachShader(program, *vertex);
    glAttachShader(program, *fragment);
    glLinkProgram(program);

    glGetProgramiv(program, GL_LINK_STATUS, &success);

    if (!success) {
        GLchar infoLog[GL_LOG_LEN];
        glGetProgramInfoLog(program, GL_LOG_LEN, nullptr, infoLog);
        THROW_ERR(
            "Shader program linking failed: {}", std::string(infoLog)
        );
    }
    return program;
}

void ShaderProgram::recompile(const std::vector<std::string>& defines) {
    GLuint newProgram = compile_program(vertexSource, fragmentSource, defines);
    glDeleteProgram(id);
    id = newProgram;
    uniformLocationCache.clear();
    LOG_INFO("Shader {} has been recompiled", id);
}

std::unique_ptr<ShaderProgram> ShaderProgram::create(
    Source&& vertexSource, Source&& fragmentSource
) {
    return std::make_unique<ShaderProgram>(
        compile_program(vertexSource, fragmentSource, {}),
        std::move(vertexSource),
        std::move(fragmentSource)
    );
}

ShaderProgram& ShaderProgram::getUsed() {
    return *used;
}
