#define CHROMA_ENABLE_REFLECTION
#include <coders/GLSLExtension.h>

#include <sstream>
#include <stdexcept>
#include <utility>

#include <util/stringutil.h>
#include <typedefs.h>
#include <debug/Logger.h>
#include <io/engine_paths.h>
#include <constants.h>
#include <coders/BasicParser.h>
#include <coders/json.h>
#include <data/dv_util.h>

void GLSLExtension::setPaths(const ResPaths* paths) {
    this->paths = paths;
}

void GLSLExtension::loadHeader(const std::string& name) {
    if (paths == nullptr) return;

    io::path file = paths->find(SHADERS_FOLDER + "/lib/" + name + ".glsl");
    std::string source = io::read_string(file);
    addHeader(name, {});
    addHeader(name, process(file, source, true));
}

void GLSLExtension::addHeader(const std::string& name, ProcessingResult header) {
    headers[name] = std::move(header);
}

void GLSLExtension::define(const std::string& name, std::string value) {
    defines[name] = std::move(value);
}

const GLSLExtension::ProcessingResult& GLSLExtension::getHeader(
    const std::string& name
) const {
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

const std::unordered_map<std::string, std::string>& GLSLExtension::getDefines() const {
    return defines;
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
    const io::path& file, 
    uint linenum, 
    const std::string& message
) {
    LOG_ERROR("File {}: {} at line {}", file.string(), message, std::to_string(linenum));
    throw std::runtime_error("File " + file.string() + ": " + message + " at line " + std::to_string(linenum));
}

// Вспомогательная функция: выводит предупреждение о проблеме при парсинге
inline void parsing_warning(
    std::string_view file, uint linenum, const std::string& message
) {
    LOG_WARN("File {}: {} at line {}", std::string(file), message, std::to_string(linenum));
}

// Вставляет директиву #line с указанным номером строки для сохранения корректной информации о строках в сообщениях компилятора
inline void source_line(std::stringstream& ss, uint linenum) {
    ss << "#line " << linenum << "\n";
}

static PostEffect::Param::Value default_value_for(PostEffect::Param::Type type) {
    switch (type) {
        case PostEffect::Param::Type::Int:
            return 0;
        case PostEffect::Param::Type::Float:
            return 0.0f;
        case PostEffect::Param::Type::Vec2:
            return glm::vec2 {0.0f, 0.0f};
        case PostEffect::Param::Type::Vec3:
            return glm::vec3 {0.0f, 0.0f, 0.0f};
        case PostEffect::Param::Type::Vec4:
            return glm::vec4 {0.0f, 0.0f, 0.0f, 0.0f};
        default:
            LOG_ERROR("Unsupported type");
            throw std::runtime_error("Unsupported type");
    }
}

class GLSLParser : public BasicParser<char> {
public:
    GLSLParser(GLSLExtension& glsl, std::string_view file, std::string_view source, bool header) : BasicParser(file, source), glsl(glsl) {
        if (!header) {
            ss << "#version " << GLSLExtension::VERSION << '\n';
        }
        for (auto& entry : glsl.getDefines()) {
            ss << "#define " << entry.first << " " << entry.second << '\n';
        }
        uint linenum = 1;
        source_line(ss, linenum);

        clikeComment = true;
    }

    bool processIncludeDirective() {
        skipWhitespace(false);
        if (peekNoJump() != '<') {
            LOG_ERROR("'<' expected");
            throw error("'<' expected");
        }
        skip(1);
        skipWhitespace(false);
        auto headerName = parseName();
        skipWhitespace(false);
        if (peekNoJump() != '>') {
            LOG_ERROR("'>' expected");
            throw error("'>' expected");
        }
        skip(1);
        skipWhitespace(false);
        skipLine();

        if (!glsl.hasHeader(headerName)) {
            glsl.loadHeader(headerName);
        }
        const auto& header = glsl.getHeader(headerName);
        for (const auto& [name, param] : header.params) {
            params[name] = param;
        }
        ss << header.code << '\n';
        source_line(ss, line);
        return false;
    }

    bool processVersionDirective() {
        parsing_warning(filename, line, "Removed #version directive");
        source_line(ss, line);
        skipLine();
        return false;
    }

    template<int n>
    PostEffect::Param::Value parseVectorValue() {
        if (peekNoJump() != '[') {
            LOG_ERROR("'[' expected");
            throw error("'[' expected");
        }
        auto value = json::parse(
            filename,
            std::string_view(source.data() + pos, source.size() - pos)
        );
        glm::vec<n, float> vec {};
        try {
            dv::get_vec<n>(value, vec);
            return vec;
        } catch (const std::exception& err) {
            throw error(err.what());
        }
    }

    PostEffect::Param::Value parseDefaultValue(PostEffect::Param::Type type, const std::string& name) {
        switch (type) {
            case PostEffect::Param::Type::Int:
                return static_cast<int>(parseNumber(1).asInteger());
            case PostEffect::Param::Type::Float:
                return static_cast<float>(parseNumber(1).asNumber());
            case PostEffect::Param::Type::Vec2:
                return parseVectorValue<2>();
            case PostEffect::Param::Type::Vec3:
                return parseVectorValue<3>();
            case PostEffect::Param::Type::Vec4:
                return parseVectorValue<4>();
            default:
                LOG_ERROR("Unsupported default value for type {}", name);
                throw error("Unsupported default value for type " + name);
        }
    }

    bool processParamDirective() {
        skipWhitespace(false);
        auto typeName = parseName();
        PostEffect::Param::Type type {};
        if (!PostEffect::Param::TypeMeta.getItem(typeName, type)) {
            LOG_ERROR("Unsupported param type {}", util::quote(typeName));
            throw error("Unsupported param type " + util::quote(typeName));
        }
        skipWhitespace(false);
        auto paramName = parseName();
        if (params.find(paramName) != params.end()) {
            LOG_ERROR("Duplicating param {}", util::quote(typeName));
            throw error("duplicating param " + util::quote(paramName));
        }
        skipWhitespace(false);
        int start = pos;

        ss << "uniform " << typeName << " " << paramName;

        bool array = false;
        if (peekNoJump() == '[') {
            skip(1);
            array = true;
            readUntil(']');
            skip(1);
            ss << source.substr(start, pos - start + 1);
        }

        ss << ";\n";
        auto defValue = default_value_for(type);
        if (peekNoJump() == '=') {
            skip(1);
            skipWhitespace(false);
            defValue = parseDefaultValue(type, typeName);
        }
        skipLine();

        params[paramName] = PostEffect::Param(type, std::move(defValue), array);
        return false;
    }

    bool processPreprocessorDirective() {
        skip(1);

        auto name = parseName();

        if (name == "version") {
            return processVersionDirective();
        } else if (name == "include") {
            return processIncludeDirective();
        } else if (name == "param") {
            return processParamDirective();
        }
        return true;
    }

    GLSLExtension::ProcessingResult process() {
        while (hasNext()) {
            skipWhitespace(false);
            if (!hasNext()) break;
            if (source[pos] != '#' || processPreprocessorDirective()) {
                pos = linestart;
                ss << readUntilEOL() << '\n';
                skip(1);
            }
        }
        return {ss.str(), std::move(params)};
    }
private:
    GLSLExtension& glsl;
    std::unordered_map<std::string, PostEffect::Param> params;
    std::stringstream ss;
};

GLSLExtension::ProcessingResult GLSLExtension::process(
    const io::path& file, const std::string& source, bool header
) {
    std::string filename = file.string();
    GLSLParser parser(*this, filename, source, header);
    return parser.process();
}

