#ifndef FILES_FILES_H_
#define FILES_FILES_H_

#include <string>
#include <stdlib.h>
#include <filesystem>
#include <vector>
#include <fstream>

#include "typedefs.h"

namespace dynamic {
    class Map;
}

namespace files {
    class rafile {
        std::ifstream file;
        size_t filelength;
    public:
        rafile(const std::filesystem::path& filename);

        void seekg(std::streampos pos);
        void read(char* buffer, std::streamsize size);
        size_t length() const;
    };

    bool write_bytes(
        const std::filesystem::path& filename,
        const ubyte* data,
        size_t size
    );
    uint append_bytes(
        const std::filesystem::path& filename,
        const ubyte* data,
        size_t size
    );
    bool write_string(
        const std::filesystem::path& filename,
        const std::string content
    );
    bool write_json(
        const std::filesystem::path& filename,
        const dynamic::Map* obj,
        bool nice=true
    );
    bool write_binary_json(
        const std::filesystem::path& filename,
        const dynamic::Map* obj,
        bool compression=false
    );

    bool read(
        const std::filesystem::path& filename,
        char* data,
        size_t size
    );
    std::unique_ptr<ubyte[]> read_bytes(
        const std::filesystem::path& filename,
        size_t& length
    );
    std::vector<ubyte> read_bytes(const std::filesystem::path& filename);
    std::string read_string(
        const std::filesystem::path& filename
    );
    std::shared_ptr<dynamic::Map> read_json(
        const std::filesystem::path& file
    );
    std::shared_ptr<dynamic::Map> read_binary_json(
        const std::filesystem::path& file
    );
    std::vector<std::string> read_list(
        const std::filesystem::path& file
    );
    std::shared_ptr<dynamic::Map> read_toml(
        const std::filesystem::path& file
    );
}

#endif // FILES_FILES_H_
