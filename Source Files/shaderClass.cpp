#include "../Header Files/shaderClass.h"

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

Shader::Shader(const char* vertexFile, const char* fragmentFile) {
    std::string vertexCode = get_file_contents(vertexFile);
    std::string fragmentCode = get_file_contents(fragmentFile);

    const char* vertexSource = vertexCode.c_str();
    const char* fragmentSource = fragmentCode.c_str();

    // Создаём объект вершинного шейдера
    GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
    // Прикрепляем исходный код вершинного шейдера к объекту
    glShaderSource(vertexShader, 1, &vertexSource, NULL);
    // Компилируем вершинный шейдер в машинный код
    glCompileShader(vertexShader);

    // Создаём объект фрагментного шейдера
    GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    // Прикрепляем исходный код фрагментного шейдера к объекту
    glShaderSource(fragmentShader, 1, &fragmentSource, NULL);
    // Компилируем фрагментный шейдер в машинный код
    glCompileShader(fragmentShader);

    // Создаём объект программы шейдеров
    ID = glCreateProgram();
    // Прикрепляем вершинный и фрагментный шейдеры к программе шейдеров
    glAttachShader(ID, vertexShader);
    glAttachShader(ID, fragmentShader);
    // Объединяем все шейдеры в программу шейдеров
    glLinkProgram(ID);

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