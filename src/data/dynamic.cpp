#include "dynamic.h"

#include <stdexcept>

#include "../debug/Logger.h"

using namespace dynamic;

List::~List() {
}

std::string List::str(size_t index) const {
    const auto& val = values[index];
    switch (val->type) {
        case ValueType::String: return std::get<std::string>(val->value);
        case ValueType::Boolean: return std::get<bool>(val->value) ? "true" : "false";
        case ValueType::Number: return std::to_string(std::get<double>(val->value));
        case ValueType::Integer: return std::to_string(std::get<int64_t>(val->value));
        default: {
            LOG_ERROR("Type error");
            throw std::runtime_error("Type error");
        }
    }
}

number_t List::num(size_t index) const {
    const auto& val = values[index];
    switch (val->type) {
        case ValueType::Number: return std::get<number_t>(val->value);
        case ValueType::Integer: return std::get<integer_t>(val->value);
        case ValueType::String: return std::stoll(std::get<std::string>(val->value));
        case ValueType::Boolean: return std::get<bool>(val->value);
        default: {
            LOG_ERROR("Type error");
            throw std::runtime_error("Type error");
        }
    }
}

integer_t List::integer(size_t index) const {
    const auto& val = values[index];
    switch (val->type) {
        case ValueType::Number: return std::get<number_t>(val->value);
        case ValueType::Integer: return std::get<integer_t>(val->value);
        case ValueType::String: return std::stoll(std::get<std::string>(val->value));
        case ValueType::Boolean: return std::get<bool>(val->value);
        default: {
            LOG_ERROR("Type error");
            throw std::runtime_error("Type error");
        }
    }
}

Map* List::map(size_t index) const {
    if (values[index]->type != ValueType::Map) {
        LOG_ERROR("Type error");
        throw std::runtime_error("Type error");
    }
    return std::get<Map*>(values[index]->value);
}

List* List::list(size_t index) const {
    if (values[index]->type != ValueType::List) {
        LOG_ERROR("Type error");
        throw std::runtime_error("Type error");
    }
    return std::get<List*>(values[index]->value);
}

bool List::flag(size_t index) const {
    const auto& val = values[index];
    switch (val->type) {
        case ValueType::Integer: return std::get<integer_t>(val->value);
        case ValueType::Boolean: return std::get<bool>(val->value);
        default: {
            LOG_ERROR("Type error");
            throw std::runtime_error("Type error");
        }
    }
}

List& List::put(std::string value) {
    values.push_back(std::make_unique<Value>(ValueType::String, value));
    return *this;
}

List& List::put(uint value) {
    return put((int64_t)value);
}

List& List::put(int value) {
    return put((int64_t)value);
}

List& List::put(int64_t value) {
    values.push_back(std::make_unique<Value>(ValueType::Integer, value));
    return *this;
}

List& List::put(uint64_t value) {
    return put((int64_t)value);
}

List& List::put(double value) {
    values.push_back(std::make_unique<Value>(ValueType::Number, value));
    return *this;
}

List& List::put(float value) {
    return put((double)value);
}

List& List::put(bool value) {
    values.push_back(std::make_unique<Value>(ValueType::Boolean, value));
    return *this;
}

List& List::put(Map* value) {
    values.push_back(std::make_unique<Value>(ValueType::Map, value));
    return *this;
}

List& List::put(List* value) {
    values.push_back(std::make_unique<Value>(ValueType::List, value));
    return *this;
}

List& List::put(std::unique_ptr<Value> value) {
    values.emplace_back(std::move(value));
    return *this;
}

Value* List::getValueWriteable(size_t index) const {
    if (index > values.size()) {
        LOG_ERROR("Index error");
        throw std::runtime_error("Index error");
    }
    return values.at(index).get();
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

std::string Map::getStr(std::string key) const {
    if (values.find(key) == values.end()) {
        LOG_ERROR("Missing key '{}'", key);
        throw std::runtime_error("Missing key '" + key + "'");
    }
    return getStr(key, "");
}

double Map::getNum(std::string key) const {
    if (values.find(key) == values.end()) {
        LOG_ERROR("Missing key '{}'", key);
        throw std::runtime_error("Missing key '" + key + "'");
    }
    return getNum(key, 0);
}

int64_t Map::getInt(std::string key) const {
    if (values.find(key) == values.end()) {
        LOG_ERROR("Missing key '{}'", key);
        throw std::runtime_error("Missing key '" + key + "'");
    }
    return getInt(key, 0);
}

bool Map::getBool(std::string key) const {
    if (values.find(key) == values.end()) {
        LOG_ERROR("Missing key '{}'", key);
        throw std::runtime_error("Missing key '" + key + "'");
    }
    return getBool(key, false);
}

void Map::str(std::string key, std::string& dst) const {
    dst = getStr(key, dst);
}

std::string Map::getStr(std::string key, const std::string& def) const {
    auto found = values.find(key);
    if (found == values.end()) return def;
    auto& val = found->second;
    switch (val->type) {
        case ValueType::String: return std::get<std::string>(val->value);
        case ValueType::Boolean: return std::get<bool>(val->value) ? "true" : "false";
        case ValueType::Number: return std::to_string(std::get<number_t>(val->value));
        case ValueType::Integer: return std::to_string(std::get<integer_t>(val->value));
        default: {
            LOG_ERROR("Type error");
            throw std::runtime_error("Type error");
        }
    } 
}

number_t Map::getNum(std::string key, double def) const {
    auto found = values.find(key);
    if (found == values.end()) return def;
    auto& val = found->second;
    switch (val->type) {
        case ValueType::Number: return std::get<number_t>(val->value);
        case ValueType::Integer: return std::get<integer_t>(val->value);
        case ValueType::String: return std::stoull(std::get<std::string>(val->value));
        case ValueType::Boolean: return std::get<bool>(val->value);
        default: {
            LOG_ERROR("Type error");
            throw std::runtime_error("Type error");
        }
    }
}

integer_t Map::getInt(std::string key, integer_t def) const {
    auto found = values.find(key);
    if (found == values.end()) return def;
    auto& val = found->second;
    switch (val->type) {
        case ValueType::Number: return std::get<number_t>(val->value);
        case ValueType::Integer: return std::get<integer_t>(val->value);
        case ValueType::String: return std::stoull(std::get<std::string>(val->value));
        case ValueType::Boolean: return std::get<bool>(val->value);
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
        case ValueType::Integer: return std::get<integer_t>(val->value);
        case ValueType::Boolean: return std::get<bool>(val->value);
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
        if (val->type != ValueType::Map) return nullptr;
        return std::get<Map*>(val->value);
    }
    return nullptr;
}

List* Map::list(std::string key) const {
    auto found = values.find(key);
    if (found != values.end()) return std::get<List*>(found->second->value);
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
    values[key] = std::make_unique<Value>(ValueType::Integer, value);
    return *this;
}

Map& Map::put(std::string key, uint64_t value) {
    return put(key, (int64_t)value);
}

Map& Map::put(std::string key, float value) {
    return put(key, (double)value);
}

Map& Map::put(std::string key, double value) {
    values[key] = std::make_unique<Value>(ValueType::Number, value);
    return *this;
}

Map& Map::put(std::string key, std::string value){
    values[key] = std::make_unique<Value>(ValueType::String, value);
    return *this;
}

Map& Map::put(std::string key, const char* value) {
    return put(key, std::string(value));
}

Map& Map::put(std::string key, Map* value){
    values[key] = std::make_unique<Value>(ValueType::Map, value);
    return *this;
}

Map& Map::put(std::string key, List* value){
    values[key] = std::make_unique<Value>(ValueType::List, value);
    return *this;
}

Map& Map::put(std::string key, bool value){
    values[key] = std::make_unique<Value>(ValueType::Boolean, value);
    return *this;
}

Map& Map::put(std::string key, std::unique_ptr<Value> value) {
    values.emplace(key, value.release());
    return *this;
}

size_t Map::size() const {
    return values.size();
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
        case ValueType::Map: delete std::get<Map*>(value); break;
        case ValueType::List: delete std::get<List*>(value); break;
        default:
            break;
    }
}

std::unique_ptr<Value> Value::boolean(bool value) {
    return std::make_unique<Value>(ValueType::Boolean, value);
}

std::unique_ptr<Value> Value::of(number_u value) {
    if (std::holds_alternative<integer_t>(value)) {
        return std::make_unique<Value>(ValueType::Integer, std::get<integer_t>(value));
    } else {
        return std::make_unique<Value>(ValueType::Number, std::get<number_t>(value));
    }
}

std::unique_ptr<Value> Value::of(const std::string& value) {
    return std::make_unique<Value>(ValueType::String, value);
}
