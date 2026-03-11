#ifndef GRAPHICS_SHADERPROGRAM_H_
#define GRAPHICS_SHADERPROGRAM_H_

#include <string>
#include <glm/glm.hpp>

#include "../typedefs.h"

class GLSLExtension;

/**
 * @brief Класс-обёртка для шейдерной программы OpenGL.
 *
 * Управляет созданием, компиляцией, линковкой и использованием шейдеров.
 * Предоставляет методы для загрузки uniform-переменных различных типов.
 */
class ShaderProgram {
public:
    /// Глобальный препроцессор для шейдеров (обрабатывает #include и директивы).
    static GLSLExtension* preprocessor;

    /// Идентификатор шейдерной програмы OpenGL
    uint id;

    /**
     * @brief Конструктор, создающий объект из существующего OpenGL-идентификатора.
     * @param id Идентификатор скомпилированной и слинкованной программы.
     */
    ShaderProgram(uint id);

    ~ShaderProgram();

    /**
     * @brief Активирует шейдерную программу для использования в текущем контексте.
     *
     * Вызывает glUseProgram(id).
     */
    void use();

    /**
     * @brief Загружает матрицу 4x4 в uniform-переменную.
     * @param name Имя uniform-переменной в шейдере.
     * @param matrix Значение матрицы.
     */
    void uniformMatrix(std::string name, glm::mat4 matrix);

    /**
     * @brief Загружает целое число в uniform-переменную.
     * @param name Имя переменной.
     * @param x Значение.
     */
    void uniform1i(std::string name, int x);

    /**
     * @brief Загружает вещественное число в uniform-переменную.
     * @param name Имя переменной.
     * @param x Значение.
     */
    void uniform1f(std::string name, float x);

    /**
     * @brief Загружает два вещественных числа в uniform-переменную.
     * @param name Имя переменной.
     * @param x,y Компоненты.
     */
    void uniform2f(std::string name, float x, float y);

    /**
     * @brief Загружает два вещественных числа в uniform-переменную (vec2).
     * @param name Имя переменной.
     * @param xy Два вещественных числа в виде вектора.
     */
    void uniform2f(std::string name, glm::vec2 xy);

    /**
     * @brief Загружает три вещественных числа в uniform-переменную (vec3).
     * @param name Имя переменной.
     * @param x,y,z Компоненты.
     */
    void uniform3f(std::string name, float x, float y, float z);

    /**
     * @brief Загружает три вещественных числа в uniform-переменную (vec3).
     * @param name Имя переменной.
     * @param xyz Три вещественных числа в виде вектора.
     */
    void uniform3f(std::string name, glm::vec3 xyz);

    /**
     * @brief Создаёт шейдерную программу из исходных кодов вершинного и фрагментного шейдеров.
     * @param vertexFile Имя файла вершинного шейдера.
     * @param fragmentFile Имя файла фрагментного шейдера.
     * @param vertexSource Исходный код вершинного шейдера.
     * @param fragmentSource Исходный код фрагментного шейдера.
     * @return Указатель на новый объект ShaderProgram или nullptr в случае ошибки.
     *
     * Код предварительно обрабатывается препроцессором (GLSLExtension), затем компилируется и линкуется.
     */
    static ShaderProgram* create(
        std::string vertexFile, 
        std::string fragmentFile, 
        std::string vertexSource, 
        std::string fragmentSource
    );
};

#endif // GRAPHICS_SHADERPROGRAM_H_
