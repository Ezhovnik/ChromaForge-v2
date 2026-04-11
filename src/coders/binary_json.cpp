#include "binary_json.h"

#include <stdexcept>

#include "byte_utils.h"
#include "../debug/Logger.h"
#include "zip.h"
#include "../data/dynamic.h"

using namespace json;
using namespace dynamic;

static void to_binary(ByteBuilder& builder, const Value& value) {
    switch (static_cast<Type>(value.index())) {
        case Type::None: {
            LOG_ERROR("None value is not implemented");
            throw std::runtime_error("none value is not implemented");
        } case Type::Map: {
            auto bytes = to_binary(std::get<Map_sptr>(value).get());
            builder.put(bytes.data(), bytes.size());
            break;
        } case Type::List: {
            builder.put(BJSON_TYPE_LIST);
            for (auto& element : std::get<List_sptr>(value)->values) {
                to_binary(builder, element);
            }
            builder.put(BJSON_END);
            break;
        } case Type::Integer: {
            auto val = std::get<integer_t>(value);
            if (val >= 0 && val <= 255) {
                builder.put(BJSON_TYPE_BYTE);
                builder.put(val);
            } else if (val >= INT16_MIN && val <= INT16_MAX){
                builder.put(BJSON_TYPE_INT16);
                builder.putInt16(val);
            } else if (val >= INT32_MIN && val <= INT32_MAX) {
                builder.put(BJSON_TYPE_INT32);
                builder.putInt32(val);
            } else {
                builder.put(BJSON_TYPE_INT64);
                builder.putInt64(val);
            }
            break;
        }
        case Type::Number:
            builder.put(BJSON_TYPE_NUMBER);
            builder.putFloat64(std::get<number_t>(value));
            break;
        case Type::Boolean:
            builder.put(BJSON_TYPE_FALSE + std::get<bool>(value));
            break;
        case Type::String:
            builder.put(BJSON_TYPE_STRING);
            builder.put(std::get<std::string>(value));
            break;
    }
}

static std::unique_ptr<List> array_from_binary(ByteReader& reader);
static std::unique_ptr<Map> object_from_binary(ByteReader& reader);

std::vector<ubyte> json::to_binary(const Map* obj, bool compress) {
    if (compress) {
        auto bytes = to_binary(obj, false);
        return zip::compress(bytes.data(), bytes.size());
    }
    ByteBuilder builder;
    builder.put(BJSON_TYPE_DOCUMENT);
    builder.putInt32(0);

    for (auto& entry : obj->values) {
        builder.putCStr(entry.first.c_str());
        to_binary(builder, entry.second);
    }
    builder.put(BJSON_END);

    builder.setInt32(1, builder.size());
    return builder.build();
}

std::vector<ubyte> json::to_binary(const Value& value, bool compress) {
    if (auto map = std::get_if<Map_sptr>(&value)) {
        return to_binary(map->get(), compress);
    }
    LOG_ERROR("Map is only supported as the root element");
    throw std::runtime_error("Map is only supported as the root element");
}

static Value value_from_binary(ByteReader& reader) {
    ubyte typecode = reader.get();
    switch (typecode) {
        case BJSON_TYPE_DOCUMENT:
            reader.getInt32();
            return Map_sptr(object_from_binary(reader).release());
        case BJSON_TYPE_LIST:
            return List_sptr(array_from_binary(reader).release());
        case BJSON_TYPE_BYTE:
            return static_cast<integer_t>(reader.get());
        case BJSON_TYPE_INT16:
            return static_cast<integer_t>(reader.getInt16());
        case BJSON_TYPE_INT32:
            return static_cast<integer_t>(reader.getInt32());
        case BJSON_TYPE_INT64:
            return reader.getInt64();
        case BJSON_TYPE_NUMBER:
            return reader.getFloat64();
        case BJSON_TYPE_FALSE:
        case BJSON_TYPE_TRUE:
            return (typecode - BJSON_TYPE_FALSE) != 0;
        case BJSON_TYPE_STRING:
            return reader.getString();
        default:
            LOG_ERROR("Type {} is not supported", typecode);
            throw std::runtime_error("Type " + std::to_string(typecode) + " is not supported");
    }
}

static std::unique_ptr<List> array_from_binary(ByteReader& reader) {
    auto array = std::make_unique<List>();
    while (reader.peek() != BJSON_END) {
        array->put(value_from_binary(reader));
    }
    reader.get();
    return array;
}

static std::unique_ptr<Map> object_from_binary(ByteReader& reader) {
    auto obj = std::make_unique<Map>();
    while (reader.peek() != BJSON_END) {
        const char* key = reader.getCString();
        obj->put(key, value_from_binary(reader));
    }
    reader.get();
    return obj;
}

std::shared_ptr<Map> json::from_binary(const ubyte* src, size_t size) {
    if (size < 2) {
        LOG_ERROR("Bytes length is less than 2");
        throw std::runtime_error("Bytes length is less than 2");
    }

    if (src[0] == zip::MAGIC[0] && src[1] == zip::MAGIC[1]) {
        auto data = zip::decompress(src, size);
        return from_binary(data.data(), data.size());
    } else {
        ByteReader reader(src, size);
        Value value = value_from_binary(reader);
        if (auto map = std::get_if<Map_sptr>(&value)) return *map;
        LOG_ERROR("Root value is not an object");
        throw std::runtime_error("Root value is not an object");
    }
}
