#ifndef GRAPHICS_SHADERPROGRAM_H_
#define GRAPHICS_SHADERPROGRAM_H_

#include <string>
#include <glm/glm.hpp>

#include "../typedefs.h"

class GLSLExtension;

// Класс-обертка для шейдероной программы OpenGL
class ShaderProgram {
private:
    static std::string loadShaderFile(std::string filename);
public:
    static GLSLExtension* preprocessor;
    uint id; // Идентификатор шейдерной программы OpenGL

    ShaderProgram(uint id); // Конструктор
    ~ShaderProgram(); // Деструктор

    void use(); // Активирует шейдерную программу для использования в текущем контексте отрисовки

    void uniformMatrix(std::string name, glm::mat4 matrix); // Загружает матрицу 4x4 в uniform-переменную шейдера
    void uniform1i(std::string name, int x); // Загружает целое число в uniform-переменную шейдера
    void uniform1f(std::string name, float x); // Загружает вещественное число в uniform-переменную шейдера
    void uniform2f(std::string name, float x, float y); // Загружает два вещественных числа в uniform-переменную шейдера
    void uniform2f(std::string name, glm::vec2 xy);
    void uniform3f(std::string name, float x, float y, float z); // Загружает три вещественных числа в uniform-переменную шейдера
    void uniform3f(std::string name, glm::vec3 xyz);

    static ShaderProgram* loadShaderProgram(std::string vertexFile, std::string fragmentFile);
};

#endif // GRAPHICS_SHADERPROGRAM_H_
