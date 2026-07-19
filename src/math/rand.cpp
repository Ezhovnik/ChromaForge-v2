#include <math/rand.h>

namespace util {
    static const char* uuid_hex_chars = "0123456789abcdef";
    static const char* uuid_hex_variant_chars = "89ab";

    std::string generate_uuid() {
        std::string uuid;
        uuid.resize(36);

        for (int i = 0; i < 8; ++i) {
            uuid[i] = uuid_hex_chars[RandomGenerator::get<int>(0, 15)];
        }
        uuid[8] = '-';
        for (int i = 9; i < 13; ++i) {
            uuid[i] = uuid_hex_chars[RandomGenerator::get<int>(0, 15)];
        }
        uuid[13] = '-';
        uuid[14] = '4';
        for (int i = 15; i < 18; ++i) {
            uuid[i] = uuid_hex_chars[RandomGenerator::get<int>(0, 15)];
        }
        uuid[18] = '-';
        uuid[19] = uuid_hex_variant_chars[RandomGenerator::get<int>(0, 3)];
        for (int i = 20; i < 23; ++i) {
            uuid[i] = uuid_hex_chars[RandomGenerator::get<int>(0, 15)];
        }
        uuid[23] = '-';
        for (int i = 24; i < 36; ++i) {
            uuid[i] = uuid_hex_chars[RandomGenerator::get<int>(0, 15)];
        }

        return uuid;
    }
} // namespace util
