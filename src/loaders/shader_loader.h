#ifndef LOADERS_SHADER_LOADER_H_
#define LOADERS_SHADER_LOADER_H_

#include <string>

class ShaderProgram; // Предварительное объвление класса ShaderProgram

extern ShaderProgram* loadShaderProgram(std::string vertexFile, std::string fragmentFile); // Функция для загрузки и компиляции шейдерной программы из файлов

#endif // LOADERS_SHADER_LOADER_H_