#ifndef GRAPHICS_SHADERPROGRAM_H_
#define GRAPHICS_SHADERPROGRAM_H_

#include <string>

typedef unsigned int uint;

class ShaderProgram {
public:
    uint id;

    ShaderProgram(uint id);
    ~ShaderProgram();

    void use();
};

extern ShaderProgram* loadShaderProgram(std::string vertexFile, std::string fragmentFile);

#endif