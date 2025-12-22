#ifndef LOADERS_SHADER_LOADER_H_
#define LOADERS_SHADER_LOADER_H_

#include <string>

class ShaderProgram;

extern ShaderProgram* loadShaderProgram(std::string vertexFile, std::string fragmentFile);

#endif