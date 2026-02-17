#ifndef FILES_FILES_H_
#define FILES_FILES_H_

#include <string>
#include <stdlib.h>
#include <filesystem>

#include "../typedefs.h"

namespace json {
    class JObject;
}

namespace files {
    extern bool write_bytes(std::filesystem::path filename, const char* data, size_t size);
    extern uint append_bytes(std::filesystem::path filename, const char* data, size_t size);
    extern bool read(std::filesystem::path filename, char* data, size_t size);
    extern char* read_bytes(std::filesystem::path filename, size_t& length);
    extern std::string read_string(std::filesystem::path filename);
    extern bool write_string(std::filesystem::path filename, const std::string content);

    extern json::JObject* read_json(std::filesystem::path file);
}

#endif // FILES_FILES_H_
