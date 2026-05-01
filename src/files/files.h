#pragma once

#include <string>
#include <stdlib.h>
#include <filesystem>
#include <vector>
#include <fstream>

#include <typedefs.h>
#include <util/Buffer.h>
#include <data/dv.h>

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
        const dv::value& obj,
        bool nice=true
    );
    bool write_binary_json(
        const std::filesystem::path& filename,
        const dv::value& obj,
        bool compression=false
    );

    bool read(
        const std::filesystem::path& filename,
        char* data,
        size_t size
    );
    util::Buffer<ubyte> read_bytes_buffer(
        const std::filesystem::path&
    );
    std::unique_ptr<ubyte[]> read_bytes(
        const std::filesystem::path& filename,
        size_t& length
    );
    std::vector<ubyte> read_bytes(
        const std::filesystem::path& filename
    );
    std::string read_string(
        const std::filesystem::path& filename
    );
    dv::value read_json(
        const std::filesystem::path& file
    );
    dv::value read_binary_json(
        const std::filesystem::path& file
    );
    std::vector<std::string> read_list(
        const std::filesystem::path& file
    );
    dv::value read_toml(
        const std::filesystem::path& file
    );
}
