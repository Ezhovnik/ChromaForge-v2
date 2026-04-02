#ifndef GRAPHICS_CORE_GFX_CONTEXT_H_
#define GRAPHICS_CORE_GFX_CONTEXT_H_

#include "../../typedefs.h"
#include "Viewport.h"
#include "commons.h"

class Batch2D;
class Framebuffer;

class DrawContext {
private:
    const DrawContext* parent;
    Viewport viewport;
    Batch2D* const g2d;
    Framebuffer* fbo = nullptr;

    bool depthMask = true;
    bool depthTest = false;
    bool cullFace = false;

    BlendMode blendMode = BlendMode::Normal;

    int scissorsCount = 0;
public:
    DrawContext(const DrawContext* parent, Viewport viewport, Batch2D* g2d);
    ~DrawContext();

    Batch2D* getBatch2D() const;
    const Viewport& getViewport() const;
    DrawContext sub() const;

    void setViewport(const Viewport& viewport);
    void setFramebuffer(Framebuffer* fbo);
    void setDepthMask(bool flag);
    void setDepthTest(bool flag);
    void setCullFace(bool flag);
    void setBlendMode(BlendMode mode);
    void setScissors(glm::vec4 area);
};

#endif // GRAPHICS_CORE_GFX_CONTEXT_H_
