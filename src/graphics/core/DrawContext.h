#pragma once

#include <glm/vec2.hpp>
#include <glm/vec4.hpp>

#include <typedefs.h>
#include <graphics/core/commons.h>

class Batch2D;
class Framebuffer;
class Window;

class DrawContext {
private:
    Window& window;
    const DrawContext* parent;
    glm::uvec2 viewport;
    Batch2D* g2d;
    Flushable* flushable = nullptr;
    Bindable* fbo = nullptr;

    bool depthMask = true;
    bool depthTest = false;
    bool cullFace = false;

    BlendMode blendMode = BlendMode::Normal;

    int scissorsCount = 0;
    float lineWidth = 1.0f;
public:
    DrawContext(
        const DrawContext* parent,
        Window& window,
        Batch2D* g2d
    );
    ~DrawContext();

    Batch2D* getBatch2D() const;
    const glm::uvec2& getViewport() const;
    DrawContext sub(Flushable* flushable=nullptr) const;

    void setViewport(const glm::uvec2& viewport);
    void setFramebuffer(Bindable* fbo);
    void setDepthMask(bool flag);
    void setDepthTest(bool flag);
    void setCullFace(bool flag);
    void setBlendMode(BlendMode mode);
    void setScissors(const glm::vec4& area);
    void setLineWidth(float width);
};
