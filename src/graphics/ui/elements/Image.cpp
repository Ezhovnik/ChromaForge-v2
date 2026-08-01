#include <graphics/ui/elements/Image.h>

#include <utility>

#include <graphics/core/DrawContext.h>
#include <graphics/core/Batch2D.h>
#include <graphics/core/Texture.h>
#include <assets/Assets.h>
#include <math/UVRegion.h>
#include <graphics/core/Atlas.h>
#include <assets/assets_util.h> 

using namespace gui;

Image::Image(
    GUI& gui, std::string texture, glm::vec2 size
) : UINode(gui, size),
    texture(std::move(texture))
{
    setInteractive(false);
}

util::TextureRegion Image::refreshTexture(const Assets& assets) {
    auto region = util::get_texture_region(assets, texture, fallback);
    if (region.texture && autoresize) {
        setSize(glm::vec2(
            region.texture->getWidth() * region.region.getWidth(),
            region.texture->getHeight() * region.region.getHeight()
        ));
    }
    return region;
}

void Image::draw(const DrawContext& pctx, const Assets& assets) {
    glm::vec2 pos = calcPos();
    auto batch = pctx.getBatch2D();

    auto textureRegion = refreshTexture(assets);

    batch->setRegion(textureRegion.region);
    batch->texture(textureRegion.texture);

    batch->rect(
        pos.x, pos.y,
        size.x, size.y, 
        0, 0,
        0,
        region,
        false, true,
        calcColor()
    );
}

void Image::setAutoResize(bool flag) {
    autoresize = flag;
}

bool Image::isAutoResize() const {
    return autoresize;
}

const std::string& Image::getFallback() const {
    return fallback;
}

const std::string& Image::getTexture() const {
    return texture;
}

void Image::setTexture(const std::string& name) {
    texture = name;
}

void Image::setFallback(const std::string& name) {
    fallback = name;
}

void Image::setRegion(const UVRegion& region) {
    this->region = region;
}

const UVRegion& Image::getRegion() const {
    return region;
}
