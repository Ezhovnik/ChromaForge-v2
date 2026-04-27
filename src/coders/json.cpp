#include <coders/json.h>

#include <math.h>
#include <sstream>
#include <iomanip>
#include <memory>

#include <coders/commons.h>
#include <debug/Logger.h>
#include <data/dynamic.h>
#include <util/stringutil.h>

using namespace json;

class Parser : BasicParser {
private:
    std::unique_ptr<dynamic::List> parseList();
    std::unique_ptr<dynamic::Map> parseObject();
    dynamic::Value parseValue();
public:
    Parser(std::string_view filename, std::string_view source);

    std::unique_ptr<dynamic::Map> parse();
};

class ParserDV : BasicParser {
private:
    dv::value parseList();
    dv::value parseObject();
    dv::value parseValue();
public:
    ParserDV(std::string_view filename, std::string_view source);

    dv::value parse();
};

inline void newline(std::stringstream& ss, bool nice, uint indent, const std::string& indentstr) {
    if (nice) {
        ss << "\n";
        for (uint i = 0; i < indent; ++i) {
            ss << indentstr;
        }
    } else {
        ss << ' ';
    }
}

void stringifyObj(
    const dynamic::Map* obj,
    std::stringstream& ss,
    int indent,
    const std::string& indentstr,
    bool nice
);

void stringifyArr(
    const dynamic::List* list,
    std::stringstream& ss,
    int indent,
    const std::string& indentstr,
    bool nice
);

void stringifyObj(
    const dv::value& obj,
    std::stringstream& ss,
    int indent,
    const std::string& indentstr,
    bool nice
);

void stringifyList(
    const dv::value& list,
    std::stringstream& ss,
    int indent,
    const std::string& indentstr,
    bool nice
);

void stringifyValue(const dynamic::Value& value, std::stringstream& ss, int indent, const std::string& indentstr, bool nice) {
    if (auto map = std::get_if<dynamic::Map_sptr>(&value)) {
        stringifyObj(map->get(), ss, indent, indentstr, nice);
    } else if (auto listptr = std::get_if<dynamic::List_sptr>(&value)) {
        stringifyArr(listptr->get(), ss, indent, indentstr, nice);
    } else if (auto bytesptr = std::get_if<dynamic::ByteBuffer_sptr>(&value)) {
        auto bytes = bytesptr->get();
        ss << "\"" << util::base64_encode(bytes->data(), bytes->size());
        ss << "\"";
    } else if (auto flag = std::get_if<bool>(&value)) {
        ss << (*flag ? "true" : "false");
    } else if (auto num = std::get_if<number_t>(&value)) {
        ss << std::setprecision(15) << *num;
    } else if (auto num = std::get_if<integer_t>(&value)) {
        ss << *num;
    } else if (auto str = std::get_if<std::string>(&value)) {
        ss << util::escape(*str);
    } else {
        ss << "null";
    }
}

void stringifyValue(
    const dv::value& value,
    std::stringstream& ss,
    int indent,
    const std::string& indentstr,
    bool nice
) {
    switch (value.getType()) {
        case dv::value_type::Object:
            stringifyObj(value, ss, indent, indentstr, nice);
            break;
        case dv::value_type::List:
            stringifyList(value, ss, indent, indentstr, nice);
            break;
        case dv::value_type::Bytes: {
            const auto& bytes = value.asBytes();
            ss << "\"" << util::base64_encode(bytes.data(), bytes.size());
            ss << "\"";
            break;
        }
        case dv::value_type::String:
            ss << util::escape(value.asString());
            break;
        case dv::value_type::Number:
            ss << std::setprecision(15) << value.asNumber();
            break;
        case dv::value_type::Integer:
            ss << value.asInteger();
            break;
        case dv::value_type::Boolean:
            ss << (value.asBoolean() ? "true" : "false");
            break;
        case dv::value_type::None:
            ss << "null";
            break; 
    }
}

void stringifyArr(
    const dynamic::List* list,
    std::stringstream& ss,
    int indent,
    const std::string& indentstr,
    bool nice
) {
    if (list == nullptr) {
        ss << "nullptr";
        return;
    }
    if (list->values.empty()) {
        ss << "[]";
        return;
    }
    ss << "[";
    for (size_t i = 0; i < list->size(); ++i) {
        if (i > 0 || nice) {
            newline(ss, nice, indent, indentstr);
        }
        const dynamic::Value& value = list->values[i];
        stringifyValue(value, ss, indent + 1, indentstr, nice);
        if (i + 1 < list->size()) {
            ss << ',';
        }
    }
    if (nice) {
        newline(ss, true, indent - 1, indentstr);
    }
    ss << ']';
}

void stringifyObj(const dynamic::Map* obj, std::stringstream& ss, int indent, const std::string& indentstr, bool nice) {
    if (obj == nullptr) {
        ss << "nullptr";
        return;
    }
    if (obj->values.empty()) {
        ss << "{}";
        return;
    }
    ss << "{";
    size_t index = 0;
    for (auto& entry : obj->values) {
        const std::string& key = entry.first;
        if (index > 0 || nice) newline(ss, nice, indent, indentstr);
        const dynamic::Value& value = entry.second;
        ss << util::escape(key) << ": ";
        stringifyValue(value, ss, indent + 1, indentstr, nice);
        ++index;
        if (index < obj->values.size()) ss << ',';
    }
    if (nice) newline(ss, true, indent - 1, indentstr);
    ss << '}';
}

void stringifyList(
    const dv::value& list,
    std::stringstream& ss,
    int indent,
    const std::string& indentstr,
    bool nice
) {
    if (list.empty()) {
        ss << "[]";
        return;
    }
    ss << "[";
    for (size_t i = 0; i < list.size(); i++) {
        if (i > 0 || nice) {
            newline(ss, nice, indent, indentstr);
        }
        const auto& value = list[i];
        stringifyValue(value, ss, indent + 1, indentstr, nice);
        if (i + 1 < list.size()) {
            ss << ',';
        }
    }
    if (nice) {
        newline(ss, true, indent - 1, indentstr);
    }
    ss << ']';
}

void stringifyObj(
    const dv::value& obj,
    std::stringstream& ss,
    int indent,
    const std::string& indentstr,
    bool nice
) {
    if (obj.empty()) {
        ss << "{}";
        return;
    }
    ss << "{";
    size_t index = 0;
    for (auto& [key, value] : obj.asObject()) {
        if (index > 0 || nice) {
            newline(ss, nice, indent, indentstr);
        }
        ss << util::escape(key) << ": ";
        stringifyValue(value, ss, indent + 1, indentstr, nice);
        index++;
        if (index < obj.size()) {
            ss << ',';
        }
    }
    if (nice) {
        newline(ss, true, indent - 1, indentstr);
    }
    ss << '}';
}

std::string json::stringify(const dynamic::Map* obj, bool nice, const std::string& indent) {
    std::stringstream ss;
    stringifyObj(obj, ss, 1, indent, nice);
    return ss.str();
}

std::string json::stringify(
    const dynamic::List* arr, bool nice, const std::string& indent
) {
    std::stringstream ss;
    stringifyArr(arr, ss, 1, indent, nice);
    return ss.str();
}

std::string json::stringify(
    const dynamic::Value& value, 
    bool nice, 
    const std::string& indent)
{
    std::stringstream ss;
    stringifyValue(value, ss, 1, indent, nice);
    return ss.str();
}

std::string json::stringifyDV(
    const dv::value& value, bool nice, const std::string& indent
) {
    std::stringstream ss;
    stringifyValue(value, ss, 1, indent, nice);
    return ss.str();
}

Parser::Parser(
    std::string_view filename, 
    std::string_view source
) : BasicParser(filename, source) {}

std::unique_ptr<dynamic::Map> Parser::parse() {
    char next = peek();
    if (next != '{') {
        LOG_ERROR("'{' expected");
        throw error("'{' expected");
    }
    return parseObject();
}

std::unique_ptr<dynamic::Map> Parser::parseObject() {
    expect('{');
    auto obj = std::make_unique<dynamic::Map>();
    auto& map = obj->values;
    while (peek() != '}') {
        if (peek() == '#') {
            skipLine();
            continue;
        }
        std::string key = parseName();
        char next = peek();
        if (next != ':') {
            LOG_ERROR("':' expected");
            throw error("':' expected");
        }
        pos++;
        map.insert(std::make_pair(key, parseValue()));
        next = peek();
        if (next == ',') {
            pos++;
        } else if (next == '}') {
            break;
        } else {
            LOG_ERROR("',' expected");
            throw error("',' expected");
        }
    }
    pos++;
    return obj;
}

std::unique_ptr<dynamic::List> Parser::parseList() {
    expect('[');
    auto arr = std::make_unique<dynamic::List>();
    auto& values = arr->values;
    while (peek() != ']') {
        if (peek() == '#') {
            skipLine();
            continue;
        }
        values.push_back(parseValue());

        char next = peek();
        if (next == ',') {
            pos++;
        } else if (next == ']') {
            break;
        } else {
            LOG_ERROR("',' expected");
            throw error("',' expected");
        }
    }
    pos++;
    return arr;
}

dynamic::Value Parser::parseValue() {
    char next = peek();
    if (next == '-' || next == '+' || is_digit(next)) {
        auto numeric = parseNumber();
        if (std::holds_alternative<integer_t>(numeric)) {
            return std::get<integer_t>(numeric);
        }
        return std::get<number_t>(numeric);
    }
    if (is_identifier_start(next)) {
        std::string literal = parseName();
        if (literal == "true") {
            return true;
        } else if (literal == "false") {
            return false;
        } else if (literal == "inf") {
            return INFINITY;
        } else if (literal == "nan") {
            return NAN;
        }
        LOG_ERROR("Invalid literal");
        throw error("Invalid literal");
    }
    if (next == '{') {
        return dynamic::Map_sptr(parseObject().release());
    }
    if (next == '[') {
        return dynamic::List_sptr(parseList().release());
    }
    if (next == '"' || next == '\'') {
        pos++;
        return parseString(next);
    }
    LOG_ERROR("Unexpected character '{}'", next);
    throw error("Unexpected character '" + std::string({next}) + "'");
}

dynamic::Map_sptr json::parse(std::string_view filename, std::string_view source) {
    Parser parser(filename, source);
    return parser.parse();
}

dynamic::Map_sptr json::parse(std::string_view source) {
    return parse("<string>", source);
}

ParserDV::ParserDV(std::string_view filename, std::string_view source)
    : BasicParser(filename, source) {
}

dv::value ParserDV::parse() {
    char next = peek();
    if (next != '{') {
        LOG_ERROR("'{' expected");
        throw error("'{' expected");
    }
    return parseObject();
}

dv::value ParserDV::parseObject() {
    expect('{');
    auto object = dv::object();
    while (peek() != '}') {
        if (peek() == '#') {
            skipLine();
            continue;
        }
        std::string key = parseName();
        char next = peek();
        if (next != ':') {
            LOG_ERROR("':' expected");
            throw error("':' expected");
        }
        pos++;
        object[key] = parseValue();
        next = peek();
        if (next == ',') {
            pos++;
        } else if (next == '}') {
            break;
        } else {
            LOG_ERROR("',' expected");
            throw error("',' expected");
        }
    }
    pos++;
    return object;
}

dv::value ParserDV::parseList() {
    expect('[');
    auto list = dv::list();
    while (peek() != ']') {
        if (peek() == '#') {
            skipLine();
            continue;
        }
        list.add(parseValue());

        char next = peek();
        if (next == ',') {
            pos++;
        } else if (next == ']') {
            break;
        } else {
            LOG_ERROR("',' expected");
            throw error("',' expected");
        }
    }
    pos++;
    return list;
}

dv::value ParserDV::parseValue() {
    char next = peek();
    if (next == '-' || next == '+' || is_digit(next)) {
        auto numeric = parseNumber();
        if (std::holds_alternative<integer_t>(numeric)) {
            return std::get<integer_t>(numeric);
        }
        return std::get<number_t>(numeric);
    }
    if (is_identifier_start(next)) {
        std::string literal = parseName();
        if (literal == "true") {
            return true;
        } else if (literal == "false") {
            return false;
        } else if (literal == "inf") {
            return INFINITY;
        } else if (literal == "nan") {
            return NAN;
        }
        LOG_ERROR("Invalid literal");
        throw error("Invalid literal");
    }
    if (next == '{') {
        return parseObject();
    }
    if (next == '[') {
        return parseList();
    }
    if (next == '"' || next == '\'') {
        pos++;
        return parseString(next);
    }
    LOG_ERROR("Unexpected character '{}'", next);
    throw error("Unexpected character '" + std::string({next}) + "'");
}

dv::value json::parseDV(
    std::string_view filename, std::string_view source
) {
    ParserDV parser(filename, source);
    return parser.parse();
}

dv::value json::parseDV(std::string_view source) {
    return parseDV("<string>", source);
}
