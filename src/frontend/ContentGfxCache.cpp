#include <frontend/ContentGfxCache.h>

#include <string>

#include <assets/Assets.h>
#include <content/Content.h>
#include <graphics/core/Atlas.h>
#include <voxels/Block.h>
#include <core_content_defs.h>
#include <frontend/UIDocument.h>
#include <content/ContentPack.h>

ContentGfxCache::ContentGfxCache(const Content* content, Assets* assets) : content(content) {
    const ContentIndices* contentIds = content->getIndices();
    sideregions = std::make_unique<UVRegion[]>(contentIds->blocks.count() * 6);
    Atlas* atlas = assets->get<Atlas>("blocks");

    const auto& blocks = contentIds->blocks.getIterable();
    for (uint i = 0; i < blocks.size(); ++i) {
        auto def = blocks[i];
        for (uint side = 0; side < 6; ++side) {
            const std::string& tex = def->textureFaces[side];
            if (atlas->has(tex)) {
                sideregions[i * 6 + side] = atlas->get(tex);
            } else if (atlas->has(TEXTURE_NOTFOUND)) {
                sideregions[i * 6 + side] = atlas->get(TEXTURE_NOTFOUND);
            }
        }
        for (uint side = 0; side < def->modelTextures.size(); ++side) {
            const std::string& tex = def->modelTextures[side];
            if (atlas->has(tex)) {
                def->modelUVs.push_back(atlas->get(tex));
            } else if (atlas->has(TEXTURE_NOTFOUND)) {
                def->modelUVs.push_back(atlas->get(TEXTURE_NOTFOUND));
            }
        }
    }
}

ContentGfxCache::~ContentGfxCache() = default;

const Content* ContentGfxCache::getContent() const {
    return content;
}
