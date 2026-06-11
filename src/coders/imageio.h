#pragma once

#include <string>
#include <memory>
#include <filesystem>

class ImageData;

namespace imageio {
    inline const std::string PNG = ".png";

    bool is_read_supported(const std::string& extension);
    bool is_write_supported(const std::string& extension);

    std::unique_ptr<ImageData> read(const std::filesystem::path& filename, bool flipVertically = true);
    void write(const std::string& filename, const ImageData* image);
}
