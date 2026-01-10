#include "shader_loader.h"

#include <iostream>
#include <fstream>
#include <sstream>

#include <GL/glew.h>

#include "../graphics/ShaderProgram.h"
#include "../logger/Logger.h"

// Функция для загрузки текстового файла
std::string loadFile(std::string filename) {
    std::string code;
    std::ifstream file;

    file.exceptions(std::ifstream::badbit); // Устанавливаем исключения для обработки ошибок чтения файла

    try {
        file.open(filename); // Открываем файл

        // Проверяем, успешно ли открылся файл
        if (!file.is_open()) {
            LOG_ERROR("File not found: '{}'", filename);
            return "";
        }

        std::stringstream stream; // Создаем строковый поток
        stream << file.rdbuf(); // Читаем весь файл в поток

        file.close(); // Закрываем файл

        code = stream.str(); // Получаем строку из потока
    } catch (std::ifstream::failure& e) {
        // Обработка ошибок чтения файла
        LOG_ERROR("File not succesfully read\n{}", e.what());
        return "";
    }

    return code;
}

// Основная функция для загрузки и компиляции шейдерной программы
ShaderProgram* loadShaderProgram(std::string vertexFile, std::string fragmentFile) {
    // Загрузка исходного кода шейдеров из файлов
    std::string vShaderString = loadFile(vertexFile);
    std::string fShaderString = loadFile(fragmentFile);
    
    // Проверка успешности загрузки файлов
    if (vShaderString.empty() || fShaderString.empty()) {
        LOG_CRITICAL("Failed to load shader files");
        return nullptr;
    }
    
    // Преобразование строк в C-строки для OpenGL
    const GLchar* vShaderCode = vShaderString.c_str();
    const GLchar* fShaderCode = fShaderString.c_str();

    // Переменные для работы с OpenGL
    GLuint vertex, fragment; // Идентификаторы шейдеров
    GLint success; // Статус успешности операции
    GLchar infoLog[512]; // Буфер для сообщений об ошибке

    // Компиляция вершинного шейдера
    vertex = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertex, 1, &vShaderCode, nullptr);
    glCompileShader(vertex);

    // Проверяем успешность компиляции
    glGetShaderiv(vertex, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(vertex, 512, nullptr, infoLog);
        LOG_CRITICAL("Vertex shader compililation failed\n{}", infoLog);
        return nullptr;
    }

    // Компиляция фрагментного шейдера
    fragment = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragment, 1, &fShaderCode, nullptr);
    glCompileShader(fragment);

    // Проверяем успешность компиляции
    glGetShaderiv(fragment, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(fragment, 512, nullptr, infoLog);
        LOG_CRITICAL("Fragment shader compililation failed\n{}", infoLog);
        return nullptr;
    }

    // Создание и линковка шейдерной программы
    GLuint id = glCreateProgram();
    glAttachShader(id, vertex);
    glAttachShader(id, fragment);
    glLinkProgram(id);

    // Проверяем успешность линковки
    glGetProgramiv(id, GL_LINK_STATUS, &success);
    if (!success) {
        glGetProgramInfoLog(id, 512, nullptr, infoLog);
        LOG_CRITICAL("Shader Program linking failed\n{}", infoLog);

        glDeleteShader(vertex);
        glDeleteShader(fragment);
        glDeleteProgram(id);

        return nullptr;
    }

    // После успешной линковки шейдеры можно удалить
    glDeleteShader(vertex);
    glDeleteShader(fragment);

    return new ShaderProgram(id); // Создаем и возвращаем объект шейдерной программы
}
