#include <coders/binary_json.h>

#include <stdexcept>

#include <coders/byte_utils.h>
#include <debug/Logger.h>
#include <coders/zip.h>
#include <data/dv.h>
#include <util/Buffer.h>

using namespace json;

static void to_binary(ByteBuilder& builder, const dv::value& value) {
    switch (value.getType()) {
        case dv::value_type::None: {
            LOG_ERROR("None value is not implemented");
            throw std::runtime_error("None value is not implemented");
        } case dv::value_type::Object: {
            const auto bytes = json::to_binary(value);
            builder.put(bytes.data(), bytes.size());
            break;
        } case dv::value_type::List: {
            builder.put(BJSON_TYPE_LIST);
            for (const auto& element : value) {
                to_binary(builder, element);
            }
            builder.put(BJSON_END);
            break;
        } case dv::value_type::Bytes: {
            const auto& bytes = value.asBytes();
            builder.put(BJSON_TYPE_BYTES);
            builder.putInt32(bytes.size());
            builder.put(bytes.data(), bytes.size());
            break;
        } case dv::value_type::Integer: {
            auto val = value.asInteger();
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
        case dv::value_type::Number:
            builder.put(BJSON_TYPE_NUMBER);
            builder.putFloat64(value.asNumber());
            break;
        case dv::value_type::Boolean:
            builder.put(BJSON_TYPE_FALSE + value.asBoolean());
            break;
        case dv::value_type::String:
            builder.put(BJSON_TYPE_STRING);
            builder.put(value.asString());
            break;
    }
}

std::vector<ubyte> json::to_binary(const dv::value& object, bool compress) {
    if (compress) {
        auto bytes = to_binary(object, false);
        return zip::compress(bytes.data(), bytes.size());
    }
    ByteBuilder builder;
    builder.put(BJSON_TYPE_DOCUMENT);
    builder.putInt32(0);

    for (const auto& [key, value] : object.asObject()) {
        builder.putCStr(key.c_str());
        to_binary(builder, value);
    }
    builder.put(BJSON_END);

    builder.setInt32(1, builder.size());
    return builder.build();
}

static dv::value list_from_binary(ByteReader& reader);
static dv::value object_from_binary(ByteReader& reader);

static dv::value value_from_binary(ByteReader& reader) {
    ubyte typecode = reader.get();
    switch (typecode) {
        case BJSON_TYPE_DOCUMENT:
            reader.getInt32();
            return object_from_binary(reader);
        case BJSON_TYPE_LIST:
            return list_from_binary(reader);
        case BJSON_TYPE_BYTE:
            return reader.get();
        case BJSON_TYPE_INT16:
            return reader.getInt16();
        case BJSON_TYPE_INT32:
            return reader.getInt32();
        case BJSON_TYPE_INT64:
            return reader.getInt64();
        case BJSON_TYPE_NUMBER:
            return reader.getFloat64();
        case BJSON_TYPE_FALSE:
        case BJSON_TYPE_TRUE:
            return (typecode - BJSON_TYPE_FALSE) != 0;
        case BJSON_TYPE_STRING:
            return reader.getString();
        case BJSON_TYPE_NULL:
            return nullptr;
        case BJSON_TYPE_BYTES: {
            int32_t size = reader.getInt32();
            if (size < 0) {
                LOG_ERROR("Invalid byte-buffer size {}", size);
                throw std::runtime_error(
                    "invalid byte-buffer size " + std::to_string(size)
                );
            }
            if (size > reader.remaining()) {
                LOG_ERROR("buffer_size > remaining_size", size);
                throw std::runtime_error(
                    "buffer_size > remaining_size " + std::to_string(size)
                );
            }
            auto bytes = std::make_shared<util::Buffer<ubyte>>(reader.pointer(), size);
            reader.skip(size);
            return bytes;
        }
    }
    LOG_ERROR("Type support not implemented for <{}>", typecode);
    throw std::runtime_error(
        "type support not implemented for <" + std::to_string(typecode) + ">"
    );
}

static dv::value list_from_binary(ByteReader& reader) {
    auto list = dv::list();
    while (reader.peek() != BJSON_END) {
        list.add(value_from_binary(reader));
    }
    reader.get();
    return list;
}

static dv::value object_from_binary(ByteReader& reader) {
    auto obj = dv::object();
    while (reader.peek() != BJSON_END) {
        const char* key = reader.getCString();
        obj[key] = value_from_binary(reader);
    }
    reader.get();
    return obj;
}

dv::value json::from_binary(const ubyte* src, size_t size) {
    if (size < 2) {
        LOG_ERROR("Bytes length is less than 2");
        throw std::runtime_error("Bytes length is less than 2");
    }

    if (src[0] == zip::MAGIC[0] && src[1] == zip::MAGIC[1]) {
        auto data = zip::decompress(src, size);
        return from_binary(data.data(), data.size());
    } else {
        ByteReader reader(src, size);
        return value_from_binary(reader);
    }
}
