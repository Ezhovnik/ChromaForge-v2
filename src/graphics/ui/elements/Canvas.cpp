#include <graphics/ui/elements/Canvas.h>

#include <graphics/core/Batch2D.h>
#include <graphics/core/DrawContext.h>
#include <graphics/core/Texture.h>

gui::Canvas::Canvas(
    GUI& gui, ImageFormat inFormat, glm::uvec2 inSize
) : UINode(gui, inSize) {
    auto data = std::make_shared<ImageData>(inFormat, inSize.x, inSize.y);
    mTexture = Texture::from(data.get());
    mData = std::move(data);
}

void gui::Canvas::draw(const DrawContext& pctx, const Assets& assets) {
    auto pos = calcPos();
    auto col = calcColor();

    auto batch = pctx.getBatch2D();
    batch->texture(mTexture.get());
    batch->rect(pos.x, pos.y, size.x, size.y, 0, 0, 0, {}, false, false, col);
}
