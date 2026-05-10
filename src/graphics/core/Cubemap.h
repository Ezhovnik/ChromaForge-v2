#pragma once

#include <graphics/core/GLTexture.h>

class Cubemap : public GLTexture {
public:
    Cubemap(uint width, uint height, ImageFormat format);

    virtual void bind() const override;
    virtual void unbind() const override;
};
