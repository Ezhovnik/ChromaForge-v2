#include "GLSLExtension.h"

#include <sstream>
#include <stdexcept>
#include <utility>

#include "../util/stringutil.h"
#include "../typedefs.h"
#include "../files/files.h"
#include "../debug/Logger.h"
#include "../files/engine_paths.h"
#include "../constants.h"

void GLSLExtension::setVersion(std::string version) {
    this->version = std::move(version);
}

void GLSLExtension::setPaths(const ResPaths* paths) {
    this->paths = paths;
}

void GLSLExtension::loadHeader(const std::string& name) {
    std::filesystem::path file = paths->find(SHADERS_FOLDER + "/lib/" + name + ".glsl");
    std::string source = files::read_string(file);
    addHeader(name, source);
}

void GLSLExtension::addHeader(const std::string& name, std::string source) {
    headers[name] = std::move(source);
}

void GLSLExtension::define(const std::string& name, std::string value) {
    defines[name] = std::move(value);
}

const std::string& GLSLExtension::getHeader(const std::string& name) const {
    auto found = headers.find(name);
    if (found == headers.end()) {
        LOG_ERROR("No header {} loaded", name);
        throw std::runtime_error("No header '" + name + "' loaded");
    }
    return found->second;
}

const std::string& GLSLExtension::getDefine(const std::string& name) const {
    auto found = defines.find(name);
    if (found == defines.end()) {
        LOG_ERROR("Name '{}' is not defined");
        throw std::runtime_error("Name '" + name + "' is not defined");
    }
    return found->second;
}

bool GLSLExtension::hasDefine(const std::string& name) const {
    return defines.find(name) != defines.end();
}

bool GLSLExtension::hasHeader(const std::string& name) const {
    return headers.find(name) != headers.end();
}

void GLSLExtension::undefine(const std::string& name) {
    if (hasDefine(name)) defines.erase(name);
}

// Вспомогательная функция: выбрасывает исключение с сообщением об ошибке парсинга
inline void parsing_error(
    const std::filesystem::path& file, 
    uint linenum, 
    const std::string& message
) {
    LOG_ERROR("File {}: {} at line {}", file.string(), message, std::to_string(linenum));
    throw std::runtime_error("File " + file.string() + ": " + message + " at line " + std::to_string(linenum));
}

// Вспомогательная функция: выводит предупреждение о проблеме при парсинге
inline void parsing_warning(
    const std::filesystem::path& file, 
    uint linenum, const 
    std::string& message
) {
    LOG_WARN("File {}: {} at line {}", file.string(), message, std::to_string(linenum));
}

// Вставляет директиву #line с указанным номером строки для сохранения корректной информации о строках в сообщениях компилятора
inline void source_line(std::stringstream& ss, uint linenum) {
    ss << "#line " << linenum << "\n";
}

// Основной метод обработки исходного кода шейдера
std::string GLSLExtension::process(const std::filesystem::path& file, const std::string& source) {
    std::stringstream ss;
    size_t pos = 0;
    uint linenum = 1; // текущая обрабатываемая строка исходного файла

    // Вставляем версию GLSL в начало
    ss << "#version " << version << '\n';
    // Вставляем все определённые макросы.
    for (auto& entry : defines) {
        ss << "#define " << entry.first << " " << entry.second << '\n';
    }
    // Устанавливаем #line на начало исходного кода (после добавленных директив)
    source_line(ss, linenum);

    while (pos < source.length()) {
        size_t endline = source.find('\n', pos);
        if (endline == std::string::npos) endline = source.length();

        // Если строка начинается с '#', проверяем директивы препроцессора.
        if (source[pos] == '#') {
            std::string line = source.substr(pos + 1, endline - pos);
            util::trim(line);

            if (line.find("include") != std::string::npos) {
                // Обработка #include
                line = line.substr(7);
                util::trim(line);
                if (line.length() < 3) parsing_error(file, linenum, "invalid 'include' syntax");

                if (line[0] != '<' || line[line.length() - 1] != '>') parsing_error(file, linenum, "expected '#include <filename>' syntax");

                // Извлекаем имя файла без угловых скобок.
                std::string name = line.substr(1, line.length() - 2);
                if (!hasHeader(name)) loadHeader(name);
                source_line(ss, 1);
                ss << getHeader(name) << '\n';
                pos = endline + 1;
                linenum++;
                source_line(ss, linenum);
                continue;
            } else if (line.find("version") != std::string::npos) {
                // Обработка #version – удаляем её с предупреждением, так как версия уже задана программно
                parsing_warning(file, linenum, "removed #version directive");
                pos = endline + 1;
                linenum++;
                source_line(ss, linenum);
                continue;
            }
        }
        // Обычная строка – копируем её в выходной поток.
        linenum++;
        ss << source.substr(pos, endline + 1 - pos);
        pos = endline + 1;
    }
    return ss.str();    
}
