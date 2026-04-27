#include <data/dynamic.h>

#include <coders/json.h>

using namespace dynamic;

std::ostream& operator<<(std::ostream& stream, const dynamic::Value& value) {
    stream << json::stringify(value, false, " ");
    return stream;
}

std::ostream& operator<<(std::ostream& stream, const dynamic::Map& value) {
    stream << json::stringify(&value, false, " ");
    return stream;
}

std::ostream& operator<<(std::ostream& stream, const dynamic::Map_sptr& value) {
    stream << json::stringify(value, false, " ");
    return stream;
}

std::ostream& operator<<(std::ostream& stream, const dynamic::List& value) {
    stream << json::stringify(&value, false, " ");
    return stream;
}

std::ostream& operator<<(std::ostream& stream, const dynamic::List_sptr& value) {
    stream << json::stringify(value, false, " ");
    return stream;
}

std::string List::str(size_t index) const {
    const auto& value = values[index];
    switch (static_cast<Type>(value.index())) {
        case Type::String: return std::get<std::string>(value);
        case Type::Boolean: return std::get<bool>(value) ? "true" : "false";
        case Type::Number: return std::to_string(std::get<double>(value));
        case Type::Integer: return std::to_string(std::get<int64_t>(value));
        default: {
            LOG_ERROR("Type error");
            throw std::runtime_error("Type error");
        }
    }
}

number_t List::num(size_t index) const {
    const auto& value = values[index];
    switch (static_cast<Type>(value.index())) {
        case Type::Number: return std::get<number_t>(value);
        case Type::Integer: return std::get<integer_t>(value);
        case Type::String: return std::stoll(std::get<std::string>(value));
        case Type::Boolean: return std::get<bool>(value);
        default: {
            LOG_ERROR("Type error");
            throw std::runtime_error("Type error");
        }
    }
}

integer_t List::integer(size_t index) const {
    const auto& value = values[index];
    switch (static_cast<Type>(value.index())) {
        case Type::Number: return std::get<number_t>(value);
        case Type::Integer: return std::get<integer_t>(value);
        case Type::String: return std::stoll(std::get<std::string>(value));
        case Type::Boolean: return std::get<bool>(value);
        default: {
            LOG_ERROR("Type error");
            throw std::runtime_error("Type error");
        }
    }
}

const Map_sptr& List::map(size_t index) const {
    if (auto* val = std::get_if<Map_sptr>(&values[index])) {
        return *val;
    }
    LOG_ERROR("Type error");
    throw std::runtime_error("Type error");
}

List* List::list(size_t index) const {
    if (auto* val = std::get_if<List_sptr>(&values[index])) {
        return val->get();
    }
    LOG_ERROR("Type error");
    throw std::runtime_error("Type error");
}

bool List::flag(size_t index) const {
    const auto& value = values[index];
    switch (static_cast<Type>(value.index())) {
        case Type::Integer: return std::get<integer_t>(value);
        case Type::Boolean: return std::get<bool>(value);
        default: {
            LOG_ERROR("Type error");
            throw std::runtime_error("Type error");
        }
    }
}

List& List::put(const Value& value) {
    values.emplace_back(value);
    return *this;
}

Value* List::getValueWriteable(size_t index) {
    return &values.at(index);
}

List& List::putList() {
    auto arr = create_list();
    put(arr);
    return *arr;
}

Map& List::putMap() {
    auto map = create_map();
    put(map);
    return *map;
}

ByteBuffer& List::putBytes(size_t size) {
    auto bytes = create_bytes(size);
    put(bytes);
    return *bytes;
}

void List::remove(size_t index) {
    values.erase(values.begin() + index);
}

void Map::str(const std::string& key, std::string& dst) const {
    dst = get(key, dst);
}

std::string Map::get(const std::string& key, const std::string& def) const {
    auto found = values.find(key);
    if (found == values.end()) return def;
    auto& value = found->second;
    switch (static_cast<Type>(value.index())) {
        case Type::String: return std::get<std::string>(value);
        case Type::Boolean: return std::get<bool>(value) ? "true" : "false";
        case Type::Number: return std::to_string(std::get<number_t>(value));
        case Type::Integer: return std::to_string(std::get<integer_t>(value));
        default: {
            LOG_ERROR("Type error");
            throw std::runtime_error("Type error");
        }
    } 
}

number_t Map::get(const std::string& key, double def) const {
    auto found = values.find(key);
    if (found == values.end()) return def;
    auto& value = found->second;
    switch (static_cast<Type>(value.index())) {
        case Type::Number: return std::get<number_t>(value);
        case Type::Integer: return std::get<integer_t>(value);
        case Type::String: return std::stoull(std::get<std::string>(value));
        case Type::Boolean: return std::get<bool>(value);
        default: {
            LOG_ERROR("Type error");
            throw std::runtime_error("Type error");
        }
    }
}

integer_t Map::get(const std::string& key, integer_t def) const {
    auto found = values.find(key);
    if (found == values.end()) return def;
    auto& value = found->second;
    switch (static_cast<Type>(value.index())) {
        case Type::Number: return std::get<number_t>(value);
        case Type::Integer: return std::get<integer_t>(value);
        case Type::String: return std::stoull(std::get<std::string>(value));
        case Type::Boolean: return std::get<bool>(value);
        default: {
            LOG_ERROR("Type error");
            throw std::runtime_error("Type error");
        }
    }
}

bool Map::get(const std::string& key, bool def) const {
    auto found = values.find(key);
    if (found == values.end()) return def;
    auto& value = found->second;
    switch (static_cast<Type>(value.index())) {
        case Type::Integer: return std::get<integer_t>(value);
        case Type::Boolean: return std::get<bool>(value);
        default: {
            LOG_ERROR("Type error");
            throw std::runtime_error("Type error");
        }
    }
}

void Map::remove(const std::string& key) {
    values.erase(key);
}

void Map::num(const std::string& key, double& dst) const {
    dst = get(key, dst);
}

void Map::num(const std::string& key, float& dst) const {
    dst = get(key, static_cast<number_t>(dst));
}

void Map::num(const std::string& key, ubyte& dst) const {
    dst = get(key, static_cast<integer_t>(dst));
}

void Map::num(const std::string& key, int& dst) const {
    dst = get(key, static_cast<integer_t>(dst));
}

void Map::num(const std::string& key, int64_t& dst) const {
    dst = get(key, dst);
}

void Map::num(const std::string& key, uint64_t& dst) const {
    dst = get(key, static_cast<integer_t>(dst));
}

void Map::num(const std::string& key, uint& dst) const {
    dst = get(key, static_cast<integer_t>(dst));
}

Map_sptr Map::map(const std::string& key) const {
    auto found = values.find(key);
    if (found != values.end()) {
        if (auto* val = std::get_if<Map_sptr>(&found->second)) {
            return *val;
        }
    }
    return nullptr;
}

List_sptr Map::list(const std::string& key) const {
    auto found = values.find(key);
    if (found != values.end()) return std::get<List_sptr>(found->second);
    return nullptr;
}

ByteBuffer_sptr Map::bytes(const std::string& key) const {
    auto found = values.find(key);
    if (found != values.end()) return std::get<ByteBuffer_sptr>(found->second);
    return nullptr;
}

void Map::flag(const std::string& key, bool& dst) const {
    dst = get(key, dst);
}

Map& Map::put(const std::string& key, const Value& value) {
    values[key] = value;
    return *this;
}

size_t Map::size() const {
    return values.size();
}

List& Map::putList(const std::string& key) {
    auto arr = create_list();
    put(key, arr);
    return *arr;
}

Map& Map::putMap(const std::string& key) {
    auto obj = create_map();
    put(key, obj);
    return *obj;
}

ByteBuffer& Map::putBytes(const std::string& key, size_t size) {
    auto bytes = create_bytes(size);
    put(key, bytes);
    return *bytes;
}

bool Map::has(const std::string& key) const {
    return values.find(key) != values.end();
}

static const std::string TYPE_NAMES[] {
    "none",
    "map",
    "list",
    "string",
    "number",
    "bool",
    "integer",
};

const std::string& dynamic::type_name(const Value& value) {
    return TYPE_NAMES[value.index()];
}

List_sptr dynamic::create_list(std::initializer_list<Value> values) {
    return std::make_shared<List>(values);
}

Map_sptr dynamic::create_map(std::initializer_list<std::pair<const std::string, Value>> entries) {
    return std::make_shared<Map>(entries);
}

ByteBuffer_sptr dynamic::create_bytes(size_t size) {
    return std::make_shared<ByteBuffer>(size);
}

number_t dynamic::get_number(const Value& value) {
    if (auto num = std::get_if<number_t>(&value)) {
        return *num;
    } else if (auto num = std::get_if<integer_t>(&value)) {
        return *num;
    }
    LOG_ERROR("Cannot cast {} to number", type_name(value));
    throw std::runtime_error("Cannot cast " + type_name(value) + " to number");
}

integer_t dynamic::get_integer(const Value& value) {
    if (auto num = std::get_if<integer_t>(&value)) return *num;
    LOG_ERROR("Cannot cast {} to integer", type_name(value));
    throw std::runtime_error("Cannot cast " + type_name(value) + " to integer");
}
