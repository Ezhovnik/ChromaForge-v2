#pragma once

#include <string>
#include <unordered_map>
#include <vector>

#include <io/io.h>
#include <graphics/core/PostEffect.h>

class ResPaths;

// Предобработчик GLSL-шейдеров
class GLSLExtension {
public:
    using ParamsMap = std::unordered_map<std::string, PostEffect::Param>;

    struct ProcessingResult {
        std::string code;
        ParamsMap params;
    };

    void setPaths(const ResPaths* paths);

    // Макросы
    void define(const std::string& name, std::string value); // Добавляем определение макроса
    void undefine(const std::string& name); // Удаляем определение макроса
    const std::string& getDefine(const std::string& name) const; // Получаем значение макроса
    const std::unordered_map<std::string, std::string>& getDefines() const;
    bool hasDefine(const std::string& name) const; // Проверяем наличие определения макроса

    // Заголовки
    void addHeader(const std::string& name, ProcessingResult header);
    const ProcessingResult& getHeader(const std::string& name) const;
    bool hasHeader(const std::string& name) const; // Проверяем наличие заголовка
    void loadHeader(const std::string& name);

    ProcessingResult process(
        const io::path& file,
        const std::string& source,
        bool header=false
    );

    static inline std::string VERSION = "330 core";
private:
    std::unordered_map<std::string, ProcessingResult> headers;
    std::unordered_map<std::string, std::string> defines;

    const ResPaths* paths = nullptr;
};
