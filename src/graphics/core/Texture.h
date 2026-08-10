#pragma once

#include <memory>

#include <typedefs.h>
#include <math/UVRegion.h>
#include <graphics/core/ImageData.h>
#include <graphics/core/commons.h>

class Texture : public Bindable {
protected:
    uint id;
    uint width, height; ///< Размеры текстуры в пикселях
    uint format;
public:
    Texture(uint id, uint width, uint height, ImageFormat imageFormat);
    Texture(const ubyte* data, uint width, uint height, ImageFormat format);
    virtual ~Texture();

    virtual void bind() const override;
    virtual void unbind() const override;
    virtual void reload(const ubyte* data, uint w, uint h);
    void reload(const ImageData& image);
    void reloadPartial(
        const ImageData& image,
        uint x, uint y,
        uint w, uint h
    );
    virtual void resize(uint w, uint h);

    void setNearestFilter();
    void setMipMapping(bool flag, bool pixelated);

    std::unique_ptr<ImageData> readData();
    uint getId() const;

    UVRegion getUVRegion() const {
        return UVRegion(0.0f, 0.0f, 1.0f, 1.0f);
    }

    uint getWidth() const {
        return width;
    }
    uint getHeight() const {
        return height;
    }

    static std::unique_ptr<Texture> from(const ImageData* image);
    static uint MAX_RESOLUTION;
};
