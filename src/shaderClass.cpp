#include "../Header Files/shaderClass.h"

// Читает текстовый файл и выводит строку со всем содержимым текстового файла
std::string get_file_contents(const char* filename) {
    std::ifstream in(filename, std::ios::binary);
    if (!in) {
        std::cerr << "Ошибка: не удалось открыть файл " << filename << ". Код ошибки: " << errno << std::endl;
        throw std::runtime_error("Failed to open file"); // Бросаем понятное исключение
    } else {
        std::string contents;
        in.seekg(0, std::ios::end);
        contents.resize(in.tellg());
        in.seekg(0, std::ios::beg);
        in.read(&contents[0], contents.size());
        in.close();
        return(contents);
    }
}

// Конструктор, создающий программу для шейдеров из двух разных шейдеров
Shader::Shader(const char* vertexFile, const char* fragmentFile) {
    // Считываем vertexFile и fragmentFile и сохраняем строки
    std::string vertexCode = get_file_contents(vertexFile);
    std::string fragmentCode = get_file_contents(fragmentFile);

    // Преобразуем строки исходного кода шейдера в массивы символов
    const char* vertexSource = vertexCode.c_str();
    const char* fragmentSource = fragmentCode.c_str();

    // Создаём объект вершинного шейдера
    GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
    // Прикрепляем исходный код вершинного шейдера к объекту
    glShaderSource(vertexShader, 1, &vertexSource, NULL);
    // Компилируем вершинный шейдер в машинный код
    glCompileShader(vertexShader);
    // Проверяем, успешно ли скомпилирован шейдер
    compileErrors(vertexShader, "VERTEX");

    // Создаём объект фрагментного шейдера
    GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    // Прикрепляем исходный код фрагментного шейдера к объекту
    glShaderSource(fragmentShader, 1, &fragmentSource, NULL);
    // Компилируем фрагментный шейдер в машинный код
    glCompileShader(fragmentShader);
    // Проверяем, успешно ли скомпилирован шейдер
    compileErrors(fragmentShader, "FRAGMENT");

    // Создаём объект программы шейдеров
    ID = glCreateProgram();
    // Прикрепляем вершинный и фрагментный шейдеры к программе шейдеров
    glAttachShader(ID, vertexShader);
    glAttachShader(ID, fragmentShader);
    // Объединяем все шейдеры в программу шейдеров
    glLinkProgram(ID);
    // Проверяем, успешно ли связаны шейдеры
    compileErrors(ID, "PROGRAM");

    // Удаляем уже ненужные объекты вершинного и фрагментного шейдеров 
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);
}

void Shader::Activate() {
    glUseProgram(ID);
}

void Shader::Delete() {
    glDeleteProgram(ID);
}

// Проверяет, правильно ли скомпилированы различные шейдеры
void Shader::compileErrors(unsigned int shader, const char* type) {
    GLint hasCompiled; // Переменная для сохранения статуса компиляции
    char infoLog[1024]; // Массив символов для хранения сообщения об ошибке
    if (type != "PROGRAM") {
        glGetShaderiv(shader, GL_COMPILE_STATUS, &hasCompiled);
        if (hasCompiled == GL_FALSE) {
            glGetShaderInfoLog(shader, 1024, NULL, infoLog);
            std::cout << "SHADER_COMPILATION_ERROR for: " << type << std::endl;
            std::cout << infoLog << "\n" << std::endl;
        }
    } else {
        glGetProgramiv(shader, GL_COMPILE_STATUS, &hasCompiled);
        if (hasCompiled == GL_FALSE) {
            glGetProgramInfoLog(shader, 1024, NULL, infoLog);
            std::cout << "SHADER_LINKING_ERROR for: " << type << std::endl;
            std::cout << infoLog << "\n" << std::endl;
        }
    }
}