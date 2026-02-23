#ifndef CODERS_GLSL_EXTESION_H_
#define CODERS_GLSL_EXTESION_H_

#include <string>
#include <unordered_map>
#include <filesystem>
#include <vector>

class ResPaths;

// Предобработчик GLSL-шейдеров
class GLSLExtension {
    std::unordered_map<std::string, std::string> headers; // Кэш заголовков (имя -> исходный код)
    std::unordered_map<std::string, std::string> defines; // Кэш макросов (имя -> значение)

    // Версия GLSL, используемая для подстановки в шейдер
    std::string version = "330 core";

    // Указатель на объект путей ресурсов для поиска заголовочных файлов
    const ResPaths* paths = nullptr;

    // Загружает заголовок name из файловой системы (через paths) и сохраняет в кэш
    void loadHeader(std::string name);
public:
    void setPaths(const ResPaths* paths);
    void setVersion(std::string version);

    // Макросы
    void define(std::string name, std::string value); // Добавляем определение макроса
    void undefine(std::string name); // Удаляем определение макроса
    const std::string getDefine(const std::string name) const; // Получаем значение макроса
    bool hasDefine(const std::string name) const; // Проверяем наличие определения макроса

    // Заголовки
    void addHeader(std::string name, std::string source); // Добавляем заголовок вручную (минуя файловую систему)
    const std::string& getHeader(const std::string name) const; // Получаем код заголовка по имени
    bool hasHeader(const std::string name) const; // Проверяем наличие заголовка

    // Основная функций обработки
    const std::string process(const std::filesystem::path file, const std::string& source);
};

#endif // CODERS_GLSL_EXTESION_H_
