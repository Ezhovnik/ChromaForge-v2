#include "json.h"

#include <math.h>
#include <sstream>
#include <memory>

#include "../logger/Logger.h"

using namespace json;

// Добавление отступов или пробела в зависимости от формата вывода
inline void newline(std::stringstream& ss, bool nice, uint indent, const std::string indentstr) {
    if (nice) {
        ss << "\n";
        for (uint i = 0; i < indent; i++) {
            ss << indentstr;
        }
    } else {
        ss << ' ';
    }
}

// Предварительные объявления для рекурсивных вызовов
void stringify(Value* value, std::stringstream& ss, int indent, std::string indentstr, bool nice);
void stringifyObj(JObject* obj, std::stringstream& ss, int indent, std::string indentstr, bool nice);

// Преобразование значения JSON в строку
void stringify(Value* value, std::stringstream& ss, int indent, std::string indentstr, bool nice) {
    if (value->type == valtype::object) stringifyObj(value->value.obj, ss, indent, indentstr, nice);

    else if (value->type == valtype::array) {
        std::vector<Value*>& list = value->value.arr->values;
        if (list.empty()) {
            ss << "[]";
            return;
        }
        ss << '[';
        for (uint i = 0; i < list.size(); i++) {
            Value* value = list[i];
            if (i > 0 || nice) newline(ss, nice, indent, indentstr);
            stringify(value, ss, indent+1, indentstr, nice);
            if (i + 1 < list.size()) ss << ',';
        }
        if (nice) newline(ss, true, indent - 1, indentstr);
        ss << ']';
    } else if (value->type == valtype::boolean) {
        ss << (value->value.boolean ? "true" : "false");
    } else if (value->type == valtype::number) {
        ss << value->value.num;
    } else if (value->type == valtype::string) {
        ss << escape_string(*value->value.str);
    }
}

// Преобразование объекта JSON в строку
void stringifyObj(JObject* obj, std::stringstream& ss, int indent, std::string indentstr, bool nice) {
    if (obj->map.empty()) {
        ss << "{}";
        return;
    }
    ss << "{";
    uint index = 0;
    for (auto entry : obj->map) {
        const std::string& key = entry.first;
        if (index > 0 || nice) newline(ss, nice, indent, indentstr);
        
        Value* value = entry.second;
        ss << escape_string(key) << ": ";
        stringify(value, ss, indent+1, indentstr, nice);
        index++;
        if (index < obj->map.size()) ss << ',';
    }
    if (nice) newline(ss, true, indent-1, indentstr);
    ss << '}';
}

// Публичный интерфейс для преобразования объекта JSON в строку
std::string json::stringify(JObject* obj, bool nice, std::string indent) {
    std::stringstream ss;
    stringifyObj(obj, ss, 1, indent, nice);
    return ss.str();
}

// Деструктор массива JSON
JArray::~JArray() {
    for (auto value : values) {
        delete value;
    }
}

// Методы доступа к элементам массива с проверкой типов

std::string JArray::str(size_t index) const {
    return *values[index]->value.str;
}

double JArray::num(size_t index) const {
    return values[index]->value.num;
}

JObject* JArray::obj(size_t index) const {
    return values[index]->value.obj;
}

JArray* JArray::arr(size_t index) const {
    return values[index]->value.arr;
}

bool JArray::flag(size_t index) const {
    return values[index]->value.boolean;
}

// Методы добавления элементов в массив

JArray& JArray::put(std::string value) {
    valvalue val;
    val.str = new std::string(value);
    values.push_back(new Value(valtype::string, val));
    return *this;
}

JArray& JArray::put(double value) {
    valvalue val;
    val.num = value;
    values.push_back(new Value(valtype::number, val));
    return *this;
}

JArray& JArray::put(bool value) {
    valvalue val;
    val.boolean = value;
    values.push_back(new Value(valtype::boolean, val));
    return *this;
}

JArray& JArray::put(JObject* value) {
    valvalue val;
    val.obj = value;
    values.push_back(new Value(valtype::object, val));
    return *this;
}

JArray& JArray::put(JArray* value) {
    valvalue val;
    val.arr = value;
    values.push_back(new Value(valtype::array, val));
    return *this;
}

JArray& JArray::put(float value) {
    valvalue val;
    val.num = value;
    values.push_back(new Value(valtype::number, val));
    return *this;
}

// Деструктор объекта JSON
JObject::~JObject() {
    for (auto entry : map) {
        delete entry.second;
    }
}

// Методы получения значений из объекта с проверкой существования

void JObject::str(std::string key, std::string& dst) const {
    auto found = map.find(key);
    if (found != map.end()) dst = *found->second->value.str;
}

void JObject::num(std::string key, double& dst) const {
    auto found = map.find(key);
    if (found != map.end()) dst = found->second->value.num;
}

void JObject::num(std::string key, int& dst) const {
    auto found = map.find(key);
    if (found != map.end()) dst = found->second->value.num;
}

void JObject::num(std::string key, uint& dst) const {
    auto found = map.find(key);
    if (found != map.end()) dst = found->second->value.num;
}

void JObject::num(std::string key, float& dst) const {
    auto found = map.find(key);
    if (found != map.end()) dst = found->second->value.num;
}

JObject* JObject::obj(std::string key) const {
    auto found = map.find(key);
    if (found != map.end()) return found->second->value.obj;
    return nullptr;
}

JArray* JObject::arr(std::string key) const {
    auto found = map.find(key);
    if (found != map.end()) return found->second->value.arr;
    return nullptr;
}

void JObject::flag(std::string key, bool& dst) const {
    auto found = map.find(key);
    if (found != map.end()) dst = found->second->value.boolean;
}

// Методы добавления значений в объект с заменой существующих

JObject& JObject::put(std::string key, double value) {
    auto found = map.find(key);
    if (found != map.end()) delete found->second;
    valvalue val;
    val.num = value;
    map.insert(make_pair(key, new Value(valtype::number, val)));
    return *this;
}

JObject& JObject::put(std::string key, std::string value){
    auto found = map.find(key);
    if (found != map.end()) delete found->second;
    valvalue val;
    val.str = new std::string(value);
    map.insert(make_pair(key, new Value(valtype::string, val)));
    return *this;
}

JObject& JObject::put(std::string key, JObject* value){
    auto found = map.find(key);
    if (found != map.end()) delete found->second;
    valvalue val;
    val.obj = value;
    map.insert(make_pair(key, new Value(valtype::object, val)));
    return *this;
}

JObject& JObject::put(std::string key, JArray* value){
    auto found = map.find(key);
    if (found != map.end()) delete found->second;
    valvalue val;
    val.arr = value;
    map.insert(make_pair(key, new Value(valtype::array, val)));
    return *this;
}

JObject& JObject::put(std::string key, bool value){
    auto found = map.find(key);
    if (found != map.end()) delete found->second;
    valvalue val;
    val.boolean = value;
    map.insert(make_pair(key, new Value(valtype::boolean, val)));
    return *this;
}

JObject& JObject::put(std::string key, uint value) {
    return put(key, (double)value);
}

JObject& JObject::put(std::string key, int value) {
    return put(key, (double)value);
}

JObject& JObject::put(std::string key, float value) {
    return put(key, (double)value);
}

// Конструктор значения JSON
Value::Value(valtype type, valvalue value) : type(type), value(value) {
}

// Деструктор значения JSON - освобождает память в зависимости от типа
Value::~Value() {
    switch (type) {
        case valtype::object: delete value.obj; break;
        case valtype::array: delete value.arr; break;
        case valtype::string: delete value.str; break;
        default:
            break;
    }
}

// Конструктор парсера
Parser::Parser(std::string filename, std::string source) : BasicParser(filename, source) {    
}

// Основной метод парсинга JSON
JObject* Parser::parse() {
    char next = peek();
    if (next != '{') throw error("'{' expected");

    return parseObject();
}

// Парсинг JSON объекта
JObject* Parser::parseObject() {
    expect('{');
    std::unique_ptr<JObject> obj(new JObject());
    std::unordered_map<std::string, Value*>& map = obj->map;
    while (peek() != '}') {
        std::string key = parseName();
        char next = peek();
        if (next != ':') throw error("':' expected");
        pos++;
        map.insert(make_pair(key, parseValue()));
        next = peek();

        if (next == ',') pos++;
        else if (next == '}') break;
        else throw error("',' expected");
    }
    pos++;
    return obj.release();
}

// Парсинг JSON массива
JArray* Parser::parseArray() {
    expect('[');
    std::unique_ptr<JArray> arr(new JArray());
    std::vector<Value*>& values = arr->values;
    while (peek() != ']') {
        values.push_back(parseValue());

        char next = peek();
        if (next == ',') {
            pos++;
        } else if (next == ']') {
            break;
        } else {
            throw error("',' expected");
        }
    }
    pos++;
    return arr.release();
}

// Парсинг любого значения JSON
Value* Parser::parseValue() {
    char next = peek();
    valvalue val;
    if (is_identifier_start(next)) {
        std::string literal = parseName();
        if (literal == "true") {
            val.boolean = true;
            return new Value(valtype::boolean, val);
        } else if (literal == "false") {
            val.boolean = false;
            return new Value(valtype::boolean, val);
        } else if (literal == "inf") {
            val.num = INFINITY;
            return new Value(valtype::number, val);
        } else if (literal == "nan") {
            val.num = NAN;
            return new Value(valtype::number, val);
        }
        throw error("Invalid literal");
    }
    if (next == '{') {
        val.obj = parseObject();
        return new Value(valtype::object, val);
    }
    if (next == '[') {
        val.arr = parseArray();
        return new Value(valtype::array, val);
    }
    if (next == '-' || next == '+') {
        pos++;
        val.num = parseNumber(next == '-' ? -1 : 1);
        return new Value(valtype::number, val);
    }
    if (is_digit(next)) {
        val.num = parseNumber(1);
        return new Value(valtype::number, val);  
    }
    if (next == '"' || next == '\'') {
        pos++;
        val.str = new std::string(parseString(next));
        return new Value(valtype::string, val);
    }
    throw error("Unexpected character '" + std::string({next}) + "'");
}

// Публичный интерфейс парсинга JSON из файла
JObject* json::parse(std::string filename, std::string source) {
    Parser parser(filename, source);
    return parser.parse();
}

// Публичный интерфейс парсинга JSON из строки
JObject* json::parse(std::string source) {
    return parse("<string>", source);
}
