#include "json.h"

#include <math.h>
#include <sstream>
#include <iomanip>
#include <memory>

#include "commons.h"
#include "../debug/Logger.h"
#include "../data/dynamic.h"
#include "../util/stringutil.h"

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

void stringifyValue(const dynamic::Value& value, std::stringstream& ss, int indent, const std::string& indentstr, bool nice) {
    if (auto map = std::get_if<dynamic::Map_sptr>(&value)) {
        stringifyObj(map->get(), ss, indent, indentstr, nice);
    } else if (auto listptr = std::get_if<dynamic::List_sptr>(&value)) {
        auto list = *listptr;
        if (list->size() == 0) {
            ss << "[]";
            return;
        }
        ss << '[';
        for (uint i = 0; i < list->size(); ++i) {
            dynamic::Value& value = list->get(i);
            if (i > 0 || nice) newline(ss, nice, indent, indentstr);
            stringifyValue(value, ss, indent + 1, indentstr, nice);
            if (i + 1 < list->size()) ss << ',';
        }
        if (nice) newline(ss, true, indent - 1, indentstr);
        ss << ']';
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

void stringifyObj(const dynamic::Map* obj, std::stringstream& ss, int indent, const std::string& indentstr, bool nice) {
    if (obj->values.empty()) {
        ss << "{}";
        return;
    }
    ss << "{";
    uint index = 0;
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

std::string json::stringify(const dynamic::Map* obj, bool nice, const std::string& indent) {
    std::stringstream ss;
    stringifyObj(obj, ss, 1, indent, nice);
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
        return parseNumber();
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

dynamic::Map_sptr json::parse(const std::string& filename, const std::string& source) {
    Parser parser(filename, source);
    return parser.parse();
}

dynamic::Map_sptr json::parse(const std::string& source) {
    return parse("<string>", source);
}
