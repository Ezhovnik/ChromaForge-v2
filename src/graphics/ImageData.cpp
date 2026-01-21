#include "ImageData.h"

ImageData::ImageData(ImageFormat format, uint width, uint height, ubyte* data) 
    : format(format), width(width), height(height), data(data) {
}

ImageData::~ImageData() {
    switch (format) {
        case ImageFormat::rgb888:
        case ImageFormat::rgba8888:
            delete[] data;
            break;
    }
}
