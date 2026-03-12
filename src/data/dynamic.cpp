#include "dynamic.h"

#include <stdexcept>

#include "../logger/Logger.h"

using namespace dynamic;

List::~List() {
}

std::string List::str(size_t index) const {
    const auto& val = values[index];
    switch (val->type) {
        case ValueType::string: return *val->value.str;
        case ValueType::boolean: return val->value.boolean ? "true" : "false";
        case ValueType::number: return std::to_string(val->value.decimal);
        case ValueType::integer: return std::to_string(val->value.integer);
        default: {
            LOG_ERROR("Type error");
            throw std::runtime_error("Type error");
        }
    }
}

double List::num(size_t index) const {
    const auto& val = values[index];
    switch (val->type) {
        case ValueType::number: return val->value.decimal;
        case ValueType::integer: return val->value.integer;
        case ValueType::string: return std::stoll(*val->value.str);
        case ValueType::boolean: return val->value.boolean;
        default: {
            LOG_ERROR("Type error");
            throw std::runtime_error("Type error");
        }
    }
}

int64_t List::integer(size_t index) const {
    const auto& val = values[index];
    switch (val->type) {
        case ValueType::number: return val->value.decimal;
        case ValueType::integer: return val->value.integer;
        case ValueType::string: return std::stoll(*val->value.str);
        case ValueType::boolean: return val->value.boolean;
        default: {
            LOG_ERROR("Type error");
            throw std::runtime_error("Type error");
        }
    }
}

Map* List::map(size_t index) const {
    if (values[index]->type != ValueType::map) {
        LOG_ERROR("Type error");
        throw std::runtime_error("Type error");
    }
    return values[index]->value.map;
}

List* List::list(size_t index) const {
    if (values[index]->type != ValueType::list) {
        LOG_ERROR("Type error");
        throw std::runtime_error("Type error");
    }
    return values[index]->value.list;
}

bool List::flag(size_t index) const {
    const auto& val = values[index];
    switch (val->type) {
        case ValueType::integer: return val->value.integer;
        case ValueType::boolean: return val->value.boolean;
        default: {
            LOG_ERROR("Type error");
            throw std::runtime_error("Type error");
        }
    }
}

List& List::put(std::string value) {
    valvalue val;
    val.str = new std::string(value);
    values.push_back(std::make_unique<Value>(ValueType::string, val));
    return *this;
}

List& List::put(uint value) {
    return put((int64_t)value);
}

List& List::put(int value) {
    return put((int64_t)value);
}

List& List::put(int64_t value) {
    valvalue val;
    val.integer = value;
    values.push_back(std::make_unique<Value>(ValueType::integer, val));
    return *this;
}

List& List::put(uint64_t value) {
    return put((int64_t)value);
}

List& List::put(double value) {
    valvalue val;
    val.decimal = value;
    values.push_back(std::make_unique<Value>(ValueType::number, val));
    return *this;
}

List& List::put(float value) {
    return put((double)value);
}

List& List::put(bool value) {
    valvalue val;
    val.boolean = value;
    values.push_back(std::make_unique<Value>(ValueType::boolean, val));
    return *this;
}

List& List::put(Map* value) {
    valvalue val;
    val.map = value;
    values.push_back(std::make_unique<Value>(ValueType::map, val));
    return *this;
}

List& List::put(List* value) {
    valvalue val;
    val.list = value;
    values.push_back(std::make_unique<Value>(ValueType::list, val));
    return *this;
}

List& List::putList() {
    List* arr = new List();
    put(arr);
    return *arr;
}

Map& List::putMap() {
    Map* map = new Map();
    put(map);
    return *map;
}

void List::remove(size_t index) {
    values.erase(values.begin() + index);
}

Map::~Map() {
}

void Map::str(std::string key, std::string& dst) const {
    dst = getStr(key, dst);
}

std::string Map::getStr(std::string key, const std::string& def) const {
    auto found = values.find(key);
    if (found == values.end()) return def;
    auto& val = found->second;
    switch (val->type) {
        case ValueType::string: return *val->value.str;
        case ValueType::boolean: return val->value.boolean ? "true" : "false";
        case ValueType::number: return std::to_string(val->value.decimal);
        case ValueType::integer: return std::to_string(val->value.integer);
        default: {
            LOG_ERROR("Type error");
            throw std::runtime_error("Type error");
        }
    } 
}

double Map::getNum(std::string key, double def) const {
    auto found = values.find(key);
    if (found == values.end()) return def;
    auto& val = found->second;
    switch (val->type) {
        case ValueType::number: return val->value.decimal;
        case ValueType::integer: return val->value.integer;
        case ValueType::string: return std::stoull(*val->value.str);
        case ValueType::boolean: return val->value.boolean;
        default: {
            LOG_ERROR("Type error");
            throw std::runtime_error("Type error");
        }
    }
}

int64_t Map::getInt(std::string key, int64_t def) const {
    auto found = values.find(key);
    if (found == values.end()) return def;
    auto& val = found->second;
    switch (val->type) {
        case ValueType::number: return val->value.decimal;
        case ValueType::integer: return val->value.integer;
        case ValueType::string: return std::stoull(*val->value.str);
        case ValueType::boolean: return val->value.boolean;
        default: {
            LOG_ERROR("Type error");
            throw std::runtime_error("Type error");
        }
    }
}

bool Map::getBool(std::string key, bool def) const {
    auto found = values.find(key);
    if (found == values.end()) return def;
    auto& val = found->second;
    switch (val->type) {
        case ValueType::integer: return val->value.integer;
        case ValueType::boolean: return val->value.boolean;
        default: {
            LOG_ERROR("Type error");
            throw std::runtime_error("Type error");
        }
    }
}

void Map::num(std::string key, double& dst) const {
    dst = getNum(key, dst);
}

void Map::num(std::string key, float& dst) const {
    dst = getNum(key, dst);
}

void Map::num(std::string key, ubyte& dst) const {
    dst = getInt(key, dst);
}

void Map::num(std::string key, int& dst) const {
    dst = getInt(key, dst);
}

void Map::num(std::string key, int64_t& dst) const {
    dst = getInt(key, dst);
}

void Map::num(std::string key, uint64_t& dst) const {
    dst = getInt(key, dst);
}

void Map::num(std::string key, uint& dst) const {
    dst = getInt(key, dst);
}

Map* Map::map(std::string key) const {
    auto found = values.find(key);
    if (found != values.end()) {
        auto& val = found->second;
        if (val->type != ValueType::map) return nullptr;
        return val->value.map;
    }
    return nullptr;
}

List* Map::list(std::string key) const {
    auto found = values.find(key);
    if (found != values.end()) return found->second->value.list;
    return nullptr;
}

void Map::flag(std::string key, bool& dst) const {
    dst = getBool(key, dst);
}

Map& Map::put(std::string key, uint value) {
    return put(key, (int64_t)value);
}

Map& Map::put(std::string key, int value) {
    return put(key, (int64_t)value);
}

Map& Map::put(std::string key, int64_t value) {
    valvalue val;
    val.integer = value;
    values[key] = std::make_unique<Value>(ValueType::integer, val);
    return *this;
}

Map& Map::put(std::string key, uint64_t value) {
    return put(key, (int64_t)value);
}

Map& Map::put(std::string key, float value) {
    return put(key, (double)value);
}

Map& Map::put(std::string key, double value) {
    valvalue val;
    val.decimal = value;
    values[key] = std::make_unique<Value>(ValueType::number, val);
    return *this;
}

Map& Map::put(std::string key, std::string value){
    valvalue val;
    val.str = new std::string(value);
    values[key] = std::make_unique<Value>(ValueType::string, val);
    return *this;
}

Map& Map::put(std::string key, const char* value) {
    return put(key, std::string(value));
}

Map& Map::put(std::string key, Map* value){
    valvalue val;
    val.map = value;
    values[key] = std::make_unique<Value>(ValueType::map, val);
    return *this;
}

Map& Map::put(std::string key, List* value){
    valvalue val;
    val.list = value;
    values[key] = std::make_unique<Value>(ValueType::list, val);
    return *this;
}

Map& Map::put(std::string key, bool value){
    valvalue val;
    val.boolean = value;
    values[key] = std::make_unique<Value>(ValueType::boolean, val);
    return *this;
}

List& Map::putList(std::string key) {
    List* arr = new List();
    put(key, arr);
    return *arr;
}

Map& Map::putMap(std::string key) {
    Map* obj = new Map();
    put(key, obj);
    return *obj;
}

bool Map::has(std::string key) {
    return values.find(key) != values.end();
}

Value::Value(ValueType type, valvalue value) : type(type), value(value) {
}

Value::~Value() {
    switch (type) {
        case ValueType::map: delete value.map; break;
        case ValueType::list: delete value.list; break;
        case ValueType::string: delete value.str; break;
        default:
            break;
    }
}
