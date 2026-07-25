#include <coders/imageio.h>

#include <functional>
#include <unordered_map>

#include <coders/png.h>
#include <graphics/core/ImageData.h>
#include <debug/Logger.h>
#include <io/io.h>

using image_reader = std::function<std::unique_ptr<ImageData>(const ubyte*, size_t, bool)>;
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

std::unique_ptr<ImageData> imageio::read(const io::path& file, bool flipVertically) {
    auto found = readers.find(file.extension());
    if (found == readers.end()) {
        THROW_ERR("File format is not supported (read): '{}'", file.string());
    }
    auto bytes = io::read_bytes_buffer(file);
    try {
        return std::unique_ptr<ImageData>(found->second(bytes.data(), bytes.size(), flipVertically));
    } catch (const std::runtime_error& err) {
        THROW_ERR("Could not to load image '{}'", file.string());
    }
}

void imageio::write(const io::path& file, const ImageData* image) {
    auto found = writers.find(file.extension());
    if (found == writers.end()) {
        THROW_ERR("File format is not supported (write): '{}'", file.string());
    }
    return found->second(io::resolve(file).u8string(), image);
}
