#define CHROMA_ENABLE_REFLECTION
#include <coders/imageio.h>

#include <functional>
#include <stdexcept>
#include <unordered_map>

#include <coders/png.h>
#include <graphics/core/ImageData.h>
#include <debug/Logger.h>
#include <io/io.h>

using image_reader = std::function<std::unique_ptr<ImageData>(const ubyte*, size_t, bool)>;
using image_writer = std::function<void(const std::string&, const ImageData*)>;

static std::unordered_map<imageio::ImageFileFormat, image_reader> readers {
    {imageio::ImageFileFormat::PNG, png::loadImage},
};

static std::unordered_map<imageio::ImageFileFormat, image_writer> writers {
    {imageio::ImageFileFormat::PNG, png::writeImage},
};

bool imageio::is_read_supported(const std::string& extension) {
    return extension == ".png";
}

bool imageio::is_write_supported(const std::string& extension) {
    return extension == ".png";
}

std::unique_ptr<ImageData> imageio::read(const io::path& file, bool flipVertically) {
    imageio::ImageFileFormat format;
    if (!imageio::ImageFileFormatMeta.getItem(file.extension().substr(1), format)) {
        throw std::runtime_error("Unsupported image format");
    }
    auto found = readers.find(format);
    if (found == readers.end()) {
        throw std::runtime_error("File format is not supported (read): '" + file.string() + "'");
    }
    auto bytes = io::read_bytes_buffer(file);
    try {
        return std::unique_ptr<ImageData>(found->second(bytes.data(), bytes.size(), flipVertically));
    } catch (const std::runtime_error& err) {
        throw std::runtime_error("Could not to load image '" + file.string() + "'");
    }
}

std::unique_ptr<ImageData> imageio::decode(
    imageio::ImageFileFormat format, util::span<ubyte> src
) {
    auto found = readers.find(format);
    try {
        return std::unique_ptr<ImageData>(found->second(src.data(), src.size(), true));
    } catch (const std::runtime_error& err) {
        throw std::runtime_error("Could not to decode image: " + std::string(err.what()));
    }
}

void imageio::write(const io::path& file, const ImageData* image) {
    imageio::ImageFileFormat format;
    if (!imageio::ImageFileFormatMeta.getItem(file.extension().substr(1), format)) {
        throw std::runtime_error("Unsupported image format");
    }
    auto found = writers.find(format);
    if (found == writers.end()) {
        throw std::runtime_error("File format is not supported (write): '" + file.string() + "'");
    }
    return found->second(io::resolve(file).u8string(), image);
}

util::Buffer<ubyte> imageio::encode(
    ImageFileFormat format, const ImageData& image
) {
    switch (format) {
        case ImageFileFormat::PNG:
            return png::encode_image(image);
        default:
            throw std::runtime_error("File format is not supported for encoding");
    }
}
