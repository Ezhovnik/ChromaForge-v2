#ifndef GRAPHICS_IMAGE_DATA_H_
#define GRAPHICS_IMAGE_DATA_H_

#include <cstdint>

#include "../typedefs.h"

enum class ImageFormat {
    rgb888,
    rgba8888
};

class ImageData {
    ImageFormat format;

    uint width;
    uint height;

    ubyte* data;
public:
    ImageData(ImageFormat format, uint width, uint height, ubyte* data);
    ~ImageData();

    ubyte* getData() const {
        return data;
    }

    ImageFormat getFormat() const {
        return format;
    }

    uint getWidth() const {
        return width;
    }

    uint getHeight() const {
        return height;
    }
};

#endif // GRAPHICS_IMAGE_DATA_H_
