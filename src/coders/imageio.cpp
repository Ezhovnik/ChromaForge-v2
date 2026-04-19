#include "imageio.h"

#include <filesystem>
#include <functional>
#include <unordered_map>

#include "png.h"
#include "graphics/core/ImageData.h"
#include <debug/Logger.h>

using image_reader = std::function<std::unique_ptr<ImageData>(const std::string&, bool)>;
using image_writer = std::function<void(const std::string&, const ImageData*)>;

static std::unordered_map<std::string, image_reader> readers {
    {".png", png::loadImage},
};

static std::unordered_map<std::string, image_writer> writers {
    {".png", png::writeImage},
};

bool imageio::is_read_supported(const std::string& extension) {
    return readers.find(extension) != readers.end();
}

bool imageio::is_write_supported(const std::string& extension) {
    return writers.find(extension) != writers.end();
}

inline std::string extensionOf(const std::string& filename) {
    return std::filesystem::u8path(filename).extension().u8string();
}

std::unique_ptr<ImageData> imageio::read(const std::string& filename, bool flipVertically) {
    auto found = readers.find(extensionOf(filename));
    if (found == readers.end()) {
        LOG_ERROR("File format is not supported (read): '{}'", filename);
        throw std::runtime_error("File format is not supported (read): '" + filename + "'");
    }
    return std::unique_ptr<ImageData>(found->second(filename, flipVertically));
}

void imageio::write(const std::string& filename, const ImageData* image) {
    auto found = writers.find(extensionOf(filename));
    if (found == writers.end()) {
        LOG_ERROR("File format is not supported (write): '{}'", filename);
        throw std::runtime_error("file format is not supported (write): '" + filename + "'");
    }
    return found->second(filename, image);
}
