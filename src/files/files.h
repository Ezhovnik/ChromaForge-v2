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

    bool write_bytes(std::filesystem::path, const ubyte* data, size_t size);
    uint append_bytes(std::filesystem::path, const ubyte* data, size_t size);
    bool write_string(std::filesystem::path filename, const std::string content);
    bool write_json(std::filesystem::path filename, const dynamic::Map* obj, bool nice=true);
    bool write_binary_json(std::filesystem::path filename, const dynamic::Map* obj, bool compression=false);

    bool read(std::filesystem::path, char* data, size_t size);
    std::unique_ptr<ubyte[]> read_bytes(std::filesystem::path, size_t& length);
    std::string read_string(std::filesystem::path filename);
    std::unique_ptr<dynamic::Map> read_json(std::filesystem::path file);
    std::unique_ptr<dynamic::Map> read_binary_json(std::filesystem::path file);
    std::vector<std::string> read_list(std::filesystem::path file);
}

#endif // FILES_FILES_H_
