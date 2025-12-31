#ifndef GRAPHICS_SHADERPROGRAM_H_
#define GRAPHICS_SHADERPROGRAM_H_

#include <string>
#include <glm/glm.hpp>

typedef unsigned int uint;

// Класс-обертка для шейдероной программы OpenGL
class ShaderProgram {
public:
    uint id; // Идентификатор шейдерной программы OpenGL

    ShaderProgram(uint id); // Конструктор
    ~ShaderProgram(); // Деструктор

    void use(); // Активирует шейдерную программу для использования в текущем контексте отрисовки

    void uniformMatrix(std::string name, glm::mat4 matrix); // Загружает матрицу 4x4 в uniform-переменную шейдера
    void uniform1i(std::string name, int x); // Загружает целое число в uniform-переменную шейдера
    void uniform1f(std::string name, float x); // Загружает вещественное число в uniform-переменную шейдера
    void uniform2f(std::string name, float x, float y); // Загружает два вещественных числа в uniform-переменную шейдера
    void uniform3f(std::string name, float x, float y, float z); // Загружает три вещественных числа в uniform-переменную шейдера

};

extern ShaderProgram* loadShaderProgram(std::string vertexFile, std::string fragmentFile); // Внешняя функция для загрузки и компиляции шейдерной программы

#endif // GRAPHICS_SHADERPROGRAM_H_
