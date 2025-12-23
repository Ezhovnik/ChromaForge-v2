#ifndef GRAPHICS_SHADERPROGRAM_H_
#define GRAPHICS_SHADERPROGRAM_H_

#include <string>
#include <glm/glm.hpp>

typedef unsigned int uint;

class ShaderProgram {
public:
    uint id;

    ShaderProgram(uint id);
    ~ShaderProgram();

    void use();
    void uniformMatrix(std::string name, glm::mat4 matrix);
};

extern ShaderProgram* loadShaderProgram(std::string vertexFile, std::string fragmentFile);

#endif