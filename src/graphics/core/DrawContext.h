#pragma once

#include <typedefs.h>
#include <graphics/core/Viewport.h>
#include <graphics/core/commons.h>

class Batch2D;
class Framebuffer;

class DrawContext {
private:
    const DrawContext* parent;
    Viewport viewport;
    Batch2D* const g2d;
    Flushable* flushable = nullptr;
    Framebuffer* fbo = nullptr;

    bool depthMask = true;
    bool depthTest = false;
    bool cullFace = false;

    BlendMode blendMode = BlendMode::Normal;

    int scissorsCount = 0;
    float lineWidth = 1.0f;
public:
    DrawContext(const DrawContext* parent, Viewport viewport, Batch2D* g2d);
    ~DrawContext();

    Batch2D* getBatch2D() const;
    const Viewport& getViewport() const;
    DrawContext sub(Flushable* flushable=nullptr) const;

    void setViewport(const Viewport& viewport);
    void setFramebuffer(Framebuffer* fbo);
    void setDepthMask(bool flag);
    void setDepthTest(bool flag);
    void setCullFace(bool flag);
    void setBlendMode(BlendMode mode);
    void setScissors(glm::vec4 area);
    void setLineWidth(float width);
};
