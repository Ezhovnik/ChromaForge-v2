#pragma once

#include <graphics/core/Texture.h>

class Cubemap : public Texture {
public:
    Cubemap(uint width, uint height, ImageFormat format);

    virtual void bind() const override;
    virtual void unbind() const override;

    virtual void reload(const ubyte* data, uint w, uint h) override;
};
