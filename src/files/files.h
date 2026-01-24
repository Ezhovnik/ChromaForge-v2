#ifndef FILES_FILES_H_
#define FILES_FILES_H_

#include <filesystem>
#include <string>
#include "../typedefs.h"

namespace files {
    extern bool write_bytes(std::filesystem::path filename, const char* data, size_t size);
    extern uint append_bytes(std::filesystem::path filename, const char* data, size_t size);
    extern bool read(std::filesystem::path filename, char* data, size_t size);
    extern char* read_bytes(std::filesystem::path filename, size_t& length);

    extern std::string read_string(std::filesystem::path filename);
    extern bool write_string(std::filesystem::path filename, const std::string content);
}

#endif // FILES_FILES_H_
