#include <graphics/ui/elements/Frame.h>

#include <assets/Assets.h>
#include <graphics/core/Batch2D.h>
#include <graphics/core/Framebuffer.h>
#include <graphics/core/DrawContext.h>
#include <graphics/core/Texture.h>
#include <window/Window.h>
#include <graphics/ui/GUI.h>
#include <frontend/UIDocument.h>

static inline constexpr int MAX_TEXTURE_SIZE = 2048;

gui::Frame::Frame(
    GUI& gui,
    std::string id,
    std::string outputTexture
) : Container(gui, {}),
    fbo(nullptr),
    outputTexture(std::move(outputTexture))
{
    frameId = std::move(id);
}

gui::Frame::~Frame() = default;

void gui::Frame::draw(const DrawContext& pctx, const Assets& assets) {
    if (outputTexture.empty()) {
        Container::draw(pctx, assets);
        return;
    }
    if (fbo == nullptr) return;
    glm::ivec2 size = getSize();
    if (size.x <= 0 ||
        size.y <= 0 ||
        size.x > MAX_TEXTURE_SIZE ||
        size.y > MAX_TEXTURE_SIZE
    ) {
        return;
    }

    setPos({0, pctx.getViewport().y - size.y});

    auto ctx = pctx.sub();
    ctx.setFramebuffer(fbo.get());
    display::clear();
    Container::draw(ctx, assets);
    ctx.getBatch2D()->flush();
}

void gui::Frame::updateOutput(Assets& assets) {
    if (fbo && (fbo->getWidth() != size.x || fbo->getHeight() != size.y)) {
        fbo->resize(size.x, size.y);
        assets.store(fbo->getSharedTexture(), outputTexture);
    } else if (fbo == nullptr) {
        fbo = std::make_unique<Framebuffer>(size.x, size.y, true);
        assets.store(fbo->getSharedTexture(), outputTexture);
    }
}

const std::string& gui::Frame::getOutputTexture() const {
    return outputTexture;
}

const std::string& gui::Frame::getFrameId() const {
    return frameId;
}
