#ifndef FILES_FILES_H_
#define FILES_FILES_H_

#include <string>
#include <stdlib.h>
#include <filesystem>
#include <vector>
#include <fstream>

#include "../typedefs.h"

namespace dynamic {
    class Map;
}

namespace files {
    class rafile {
        std::ifstream file;
        size_t filelength;
    public:
        rafile(std::filesystem::path filename);

        void seekg(std::streampos pos);
        void read(char* buffer, std::streamsize size);
        size_t length() const;
    };

    extern bool write_bytes(std::filesystem::path, const char* data, size_t size);
    extern uint append_bytes(std::filesystem::path, const char* data, size_t size);
    extern bool write_string(std::filesystem::path filename, const std::string content);
    extern bool write_json(std::filesystem::path filename, const dynamic::Map* obj, bool nice=true);
    extern bool write_binary_json(std::filesystem::path filename, const dynamic::Map* obj);

    extern bool read(std::filesystem::path, char* data, size_t size);
    extern char* read_bytes(std::filesystem::path, size_t& length);
    extern std::string read_string(std::filesystem::path filename);
    extern std::unique_ptr<dynamic::Map> read_json(std::filesystem::path file);
    extern std::unique_ptr<dynamic::Map> read_binary_json(std::filesystem::path file);
    extern std::vector<std::string> read_list(std::filesystem::path file);
}

#endif // FILES_FILES_H_
