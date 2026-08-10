#pragma once

#include <memory>

#include <typedefs.h>
#include <graphics/core/commons.h>

class Texture;

class Framebuffer : public Bindable {
    uint fbo;
    uint depth;
    uint width;
    uint height;
    std::shared_ptr<Texture> texture;
public:
    Framebuffer(uint fbo, uint depth, std::unique_ptr<Texture> texture);
    Framebuffer(uint width, uint height, bool alpha=false);
    ~Framebuffer();

    void setTexture(std::unique_ptr<Texture> texture);

    void bind() const override;
    void unbind() const override;

    void resize(uint width, uint height);

    Texture* getTexture() const;
    std::shared_ptr<Texture> getSharedTexture() const;

    uint getWidth() const;
    uint getHeight() const;

    uint getFBO() const;
};
