#pragma once

#include <string>
#include <memory>

#include <io/fwd.h>
#include <util/Buffer.h>
#include <util/EnumMetadata.h>
#include <util/span.h>
#include <typedefs.h>

class ImageData;

namespace imageio {
    enum class ImageFileFormat {
        PNG
    };

    CHROMA_ENUM_METADATA(ImageFileFormat)
        {"png", ImageFileFormat::PNG},
    CHROMA_ENUM_END

    inline const std::string PNG = ".png";

    bool is_read_supported(const std::string& extension);
    bool is_write_supported(const std::string& extension);

    std::unique_ptr<ImageData> read(const io::path& filename, bool flipVertically = true);
    void write(const io::path& file, const ImageData* image);

    util::Buffer<ubyte> encode(ImageFileFormat format, const ImageData& image);
    std::unique_ptr<ImageData> decode(ImageFileFormat format, util::span<ubyte> src);
}
