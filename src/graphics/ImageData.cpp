#include "ImageData.h"

#include <assert.h>
#include <stdexcept>
#include "../logger/Logger.h"

template<typename T>
inline T min(T a, T b) {
    return (a > b) ? b : a;
}

template<typename T>
inline T max(T a, T b) {
	return (a > b) ? a : b;
}

ImageData::ImageData(ImageFormat format, uint width, uint height) : format(format), width(width), height(height) {
    switch (format) {
        case ImageFormat::rgb888: data = new ubyte[width * height * 3]{}; break;
        case ImageFormat::rgba8888: data = new ubyte[width * height * 4]{}; break;
        default:
            LOG_ERROR("Format is not supported");
            throw std::runtime_error("Format is not supported");
    }
}

ImageData::ImageData(ImageFormat format, uint width, uint height, void* data) : format(format), width(width), height(height), data(data) {
}

ImageData::~ImageData() {
    switch (format) {
        case ImageFormat::rgb888:
        case ImageFormat::rgba8888:
            delete[] (ubyte*)data;
            break;
    }
}

void ImageData::blit(const ImageData* image, int x, int y) {
    if (format != image->format) {
        LOG_ERROR("Mismatching format");
        throw std::runtime_error("Mismatching format");
    }
    uint comps;
    switch (format) {
        case ImageFormat::rgb888: comps = 3; break;
        case ImageFormat::rgba8888: comps = 4; break;
        default:
            LOG_ERROR("Only unsigned byte formats supported");
            throw std::runtime_error("Only unsigned byte formats supported");    
    }
    ubyte* pixels = static_cast<ubyte*>(data);
    ubyte* source = static_cast<ubyte*>(image->getData());
    uint srcwidth = image->getWidth();
    uint srcheight = image->getHeight();

    for (uint srcy = max(0, -y); (int)srcy < min(srcheight, height - y); ++srcy) {
        for (uint srcx = max(0, -x); (int)srcx < min(srcwidth, width - x); ++srcx) {
            uint dstx = srcx + x;
            uint dsty = srcy + y;
            uint dstidx = (dsty * width + dstx) * comps;
            uint srcidx = (srcy * srcwidth + srcx) * comps;
            for (uint c = 0; c < comps; c++) {
                pixels[dstidx + c] = source[srcidx + c];
            }
        }
    }
}

void ImageData::extrude(int x, int y, int w, int h) {
    uint comps;
    switch (format) {
        case ImageFormat::rgb888: comps = 3; break;
        case ImageFormat::rgba8888: comps = 4; break;
        default:
            LOG_ERROR("Only unsigned byte formats supported");
            throw std::runtime_error("Only unsigned byte formats supported");    
    }
    ubyte* pixels = static_cast<ubyte*>(data);

    int rx = x + w - 1;
    int ry = y + h - 1;

    if (x > 0 && (uint)x < width && y > 0 && (uint)y < height) {
        uint srcidx = (y * width + x) * comps;
        uint dstidx = ((y - 1) * width + x - 1) * comps;
        for (uint c = 0; c < comps; c++) {
            pixels[dstidx + c] = pixels[srcidx + c];
        }
    }

    if (rx >= 0 && (uint)rx < width-1 && y > 0 && (uint)y < height) {
        uint srcidx = (y * width + rx) * comps;
        uint dstidx = ((y - 1) * width + rx + 1) * comps;
        for (uint c = 0; c < comps; c++) {
            pixels[dstidx + c] = pixels[srcidx + c];
        }
    }

    if (x > 0 && (uint)x < width && ry >= 0 && (uint)ry < height-1) {
        uint srcidx = (ry * width + x) * comps;
        uint dstidx = ((ry + 1) * width + x - 1) * comps;
        for (uint c = 0; c < comps; c++) {
            pixels[dstidx + c] = pixels[srcidx + c];
        }
    }

    if (rx >= 0 && (uint)rx < width-1 && ry >= 0 && (uint)ry < height-1) {
        uint srcidx = (ry * width + rx) * comps;
        uint dstidx = ((ry + 1) * width + rx + 1) * comps;
        for (uint c = 0; c < comps; c++) {
            pixels[dstidx + c] = pixels[srcidx + c];
        }
    }

    if (x > 0 && (uint)x < width) {
        for (uint ey = max(y, 0); (int)ey < y + h; ey++) {
            uint srcidx = (ey * width + x) * comps;
            uint dstidx = (ey * width + x - 1) * comps;
            for (uint c = 0; c < comps; c++) {
                pixels[dstidx + c] = pixels[srcidx + c];
            }
        }
    }

    if (y > 0 && (uint)y < height) {
        for (uint ex = max(x, 0); (int)ex < x + w; ex++) {
            uint srcidx = (y * width + ex) * comps;
            uint dstidx = ((y-1) * width + ex) * comps;
            for (uint c = 0; c < comps; c++) {
                pixels[dstidx + c] = pixels[srcidx + c];
            }
        }
    }

    if (rx >= 0 && (uint)rx < width-1) {
        for (uint ey = max(y, 0); (int)ey < y + h; ey++) {
            uint srcidx = (ey * width + rx) * comps;
            uint dstidx = (ey * width + rx + 1) * comps;
            for (uint c = 0; c < comps; c++) {
                pixels[dstidx + c] = pixels[srcidx + c];
            }
        }
    }

    if (ry >= 0 && (uint)ry < height-1) {
        for (uint ex = max(x, 0); (int)ex < x + w; ex++) {
            uint srcidx = (ry * width + ex) * comps;
            uint dstidx = ((ry+1) * width + ex) * comps;
            for (uint c = 0; c < comps; c++) {
                pixels[dstidx + c] = pixels[srcidx + c];
            }
        }
    }
}

void ImageData::fixAlphaColor() {
    ubyte* pixels = static_cast<ubyte*>(data); 

    for (int ly = 0; ly < height - 1; ly++) {
        for (int lx = 0; lx < width - 1; lx++) {
            if (pixels[((ly) * width + lx) * 4 + 3]) {
                for (int c = 0; c < 3; c++) {
                    int val = pixels[((ly) * width + lx) * 4 + c];
                    if (pixels[((ly) * width + lx + 1) * 4 + 3] == 0) {
                        pixels[((ly) * width + lx + 1) * 4 + c] = val;
                    }
                    if (pixels[((ly + 1) * width + lx) * 4 + 3] == 0) {
                        pixels[((ly + 1) * width + lx) * 4 + c] = val;
                    }
                }
            }
        }
    }
}

ImageData* add_atlas_margins(ImageData* image, int grid_size) {
    assert(image->getFormat() == ImageFormat::rgba8888); // Поддерживается только RGBA
    assert(image->getWidth() == image->getHeight());

    int srcwidth = image->getWidth();
    int srcheight = image->getHeight();
    int dstwidth = srcwidth + grid_size * 2;
    int dstheight = srcheight + grid_size * 2;

    const ubyte* srcdata = (const ubyte*)image->getData(); 
    ubyte* dstdata = new ubyte[dstwidth * dstheight * 4];

    int imgres = image->getWidth() / grid_size; 
    for (int row = 0; row < grid_size; row++) {
        for (int col = 0; col < grid_size; col++) {
            int sox = col * imgres;
            int soy = row * imgres;
            int dox = 1 + col * (imgres + 2);
            int doy = 1 + row * (imgres + 2);
            for (int ly = -1; ly <= imgres; ly++) {
                for (int lx = -1; lx <= imgres; lx++) {
                    int sy = max(min(ly, imgres - 1), 0);
                    int sx = max(min(lx, imgres - 1), 0);
                    for (int c = 0; c < 4; c++)
                        dstdata[((doy + ly) * dstwidth + dox + lx) * 4 + c] = srcdata[((soy + sy) * srcwidth + sox + sx) * 4 + c];
                }
            }
            
            for (int ly = 0; ly < imgres; ly++) {
                for (int lx = 0; lx < imgres; lx++) {
                    if (srcdata[((soy + ly) * srcwidth + sox + lx) * 4 + 3]) {
                        for (int c = 0; c < 3; c++) {
                            int val = srcdata[((soy + ly) * srcwidth + sox + lx) * 4 + c];
                            if (dstdata[((doy+ly) * dstwidth + dox + lx + 1) * 4 + 3] == 0) dstdata[((doy+ly) * dstwidth + dox + lx + 1) * 4 + c] = val;
                            if (dstdata[((doy+ly + 1) * dstwidth + dox + lx) * 4 + 3] == 0) dstdata[((doy+ly + 1) * dstwidth + dox + lx) * 4 + c] = val;
                        }
                    }
                }
            }
        }
    }

    return new ImageData(image->getFormat(), dstwidth, dstheight, dstdata);
}

void ImageData::flipX() {
    uint size;
    switch (format) {
        case ImageFormat::rgb888:
        case ImageFormat::rgba8888: {
            size = (format == ImageFormat::rgba8888) ? 4 : 3;
            ubyte* pixels = (ubyte*)data;
            for (uint y = 0; y < height; y++) {
                for (uint x = 0; x < width/2; x++) {
                    for (uint c = 0; c < size; c++) {
                        ubyte temp = pixels[(y * width + x) * size + c];
                        pixels[(y * width + x) * size + c] = pixels[(y * width + (width - x - 1)) * size + c];
                        pixels[(y * width + (width - x - 1)) * size + c] = temp;
                    }
                }
            }
            break;
        }
        default:
            LOG_ERROR("Format is not supported");
            throw std::runtime_error("Format is not supported");
    }
}

void ImageData::flipY() {
    uint size;
    switch (format) {
        case ImageFormat::rgb888:
        case ImageFormat::rgba8888: {
            size = (format == ImageFormat::rgba8888) ? 4 : 3;
            ubyte* pixels = (ubyte*)data;
            for (uint y = 0; y < height/2; y++) {
                for (uint x = 0; x < width; x++) {
                    for (uint c = 0; c < size; c++) {
                        ubyte temp = pixels[(y * width + x) * size + c];
                        pixels[(y * width + x) * size + c] = pixels[((height-y-1) * width + x) * size + c];
                        pixels[((height-y-1) * width + x) * size + c] = temp;
                    }
                }
            }
            break;
        }
        default:
            LOG_ERROR("Format is not supported");
            throw std::runtime_error("Format is not supported");
    }
}
