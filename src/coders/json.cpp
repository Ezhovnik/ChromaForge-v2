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

void stringify(const dynamic::Value* value, std::stringstream& ss, int indent, const std::string& indentstr, bool nice);
void stringifyObj(const dynamic::Map* obj, std::stringstream& ss, int indent, const std::string& indentstr, bool nice);

void stringify(const dynamic::Value* value, std::stringstream& ss, int indent, const std::string& indentstr, bool nice) {
    if (value->type == dynamic::ValueType::Map) {
        auto map = std::get<dynamic::Map*>(value->value);
        stringifyObj(map, ss, indent, indentstr, nice);
    } else if (value->type == dynamic::ValueType::List) {
        auto list = std::get<dynamic::List*>(value->value);
        if (list->size() == 0) {
            ss << "[]";
            return;
        }
        ss << '[';
        for (uint i = 0; i < list->size(); ++i) {
            dynamic::Value* value = list->get(i);
            if (i > 0 || nice) newline(ss, nice, indent, indentstr);
            stringify(value, ss, indent + 1, indentstr, nice);
            if (i + 1 < list->size()) ss << ',';
        }
        if (nice) newline(ss, true, indent - 1, indentstr);
        ss << ']';
    } else if (value->type == dynamic::ValueType::Boolean) {
        ss << (std::get<bool>(value->value) ? "true" : "false");
    } else if (value->type == dynamic::ValueType::Number) {
        ss << std::setprecision(15);
        ss << std::get<number_t>(value->value);
    } else if (value->type == dynamic::ValueType::Integer) {
        ss << std::get<integer_t>(value->value);
    } else if (value->type == dynamic::ValueType::String) {
        ss << util::escape(std::get<std::string>(value->value));
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
        dynamic::Value* value = entry.second.get();
        ss << util::escape(key) << ": ";
        stringify(value, ss, indent + 1, indentstr, nice);
        index++;
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


Parser::Parser(std::string filename, std::string source) : BasicParser(filename, source) {    
}

dynamic::Map* Parser::parse() {
    char next = peek();
    if (next != '{') {
        LOG_ERROR("'{' expected");
        throw error("'{' expected");
    }
    return parseObject();
}

dynamic::Map* Parser::parseObject() {
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
    return obj.release();
}

dynamic::List* Parser::parseList() {
    expect('[');
    auto arr = std::make_unique<dynamic::List>();
    auto& values = arr->values;
    while (peek() != ']') {
        if (peek() == '#') {
            skipLine();
            continue;
        }
        values.push_back(std::unique_ptr<dynamic::Value>(parseValue()));

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
    return arr.release();
}

dynamic::Value* Parser::parseValue() {
    char next = peek();
    dynamic::valvalue val;
    if (next == '-' || next == '+') {
        pos++;
        number_u num;
        dynamic::ValueType type;
        if (parseNumber(next == '-' ? -1 : 1, num)) {
            val = std::get<integer_t>(num);
            type = dynamic::ValueType::Integer;
        } else {
            val = std::get<number_t>(num);
            type = dynamic::ValueType::Number;
        }
        return new dynamic::Value(type, val);
    }
    if (is_identifier_start(next)) {
        std::string literal = parseName();
        if (literal == "true") {
            val = true;
            return new dynamic::Value(dynamic::ValueType::Boolean, val);
        } else if (literal == "false") {
            val = false;
            return new dynamic::Value(dynamic::ValueType::Boolean, val);
        } else if (literal == "inf") {
            val = INFINITY;
            return new dynamic::Value(dynamic::ValueType::Number, val);
        } else if (literal == "nan") {
            val = NAN;
            return new dynamic::Value(dynamic::ValueType::Number, val);
        }
        LOG_ERROR("Invalid literal");
        throw error("Invalid literal");
    }
    if (next == '{') {
        val = parseObject();
        return new dynamic::Value(dynamic::ValueType::Map, val);
    }
    if (next == '[') {
        val = parseList();
        return new dynamic::Value(dynamic::ValueType::List, val);
    }
    if (is_digit(next)) {
        number_u num;
        dynamic::ValueType type;
        if (parseNumber(1, num)) {
            val = std::get<integer_t>(num);
            type = dynamic::ValueType::Integer;
        } else {
            val = std::get<number_t>(num);
            type = dynamic::ValueType::Number;
        }
        return new dynamic::Value(type, val);  
    }
    if (next == '"' || next == '\'') {
        pos++;
        val = parseString(next);
        return new dynamic::Value(dynamic::ValueType::String, val);
    }
    LOG_ERROR("Unexpected character '{}'", next);
    throw error("Unexpected character '" + std::string({next}) + "'");
}

std::unique_ptr<dynamic::Map> json::parse(std::string filename, std::string source) {
    Parser parser(filename, source);
    return std::unique_ptr<dynamic::Map>(parser.parse());
}

std::unique_ptr<dynamic::Map> json::parse(std::string source) {
    return parse("<string>", source);
}
