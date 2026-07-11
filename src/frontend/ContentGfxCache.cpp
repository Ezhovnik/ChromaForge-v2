#include <frontend/ContentGfxCache.h>

#include <string>

#include <assets/Assets.h>
#include <content/Content.h>
#include <graphics/core/Atlas.h>
#include <voxels/Block.h>
#include <core_content_defs.h>
#include <frontend/UIDocument.h>
#include <content/ContentPack.h>
#include <debug/Logger.h>
#include <settings.h>

ContentGfxCache::ContentGfxCache(
    const Content& content,
    const Assets& assets,
    const GraphicsSettings& settings
)
    : content(content), assets(assets), settings(settings) {
    refresh();
}

void ContentGfxCache::refresh(const Block& def, const Atlas& atlas) {
    for (uint side = 0; side < 6; ++side) {
        std::string tex = def.textureFaces[side];
        if (def.culling == CullingMode::Optional && !settings.denseRender.get() && atlas.has(tex + "_opaque")) {
            tex = tex + "_opaque";
        }
        if (atlas.has(tex)) {
            sideregions[def.rt.id * 6 + side] = atlas.get(tex);
        } else if (atlas.has(TEXTURE_NOTFOUND)) {
            sideregions[def.rt.id * 6 + side] = atlas.get(TEXTURE_NOTFOUND);
        }
    }
    if (def.model.type == BlockModelType::Custom) {
        auto model = assets.require<model::Model>(def.model.name);
        for (auto& mesh : model.meshes) {
            size_t pos = mesh.texture.find(':');
            if (pos == std::string::npos) continue;

            if (auto region = atlas.getIf(mesh.texture.substr(pos + 1))) {
                for (auto& vertex : mesh.vertices) {
                    vertex.uv = region->apply(vertex.uv);
                }
            }
        }
        models[def.rt.id] = std::move(model);
    }
}

void ContentGfxCache::refresh() {
    auto indices = content.getIndices();
    sideregions = std::make_unique<UVRegion[]>(indices->blocks.count() * 6);
    const auto& atlas = assets.require<Atlas>("blocks");

    const auto& blocks = indices->blocks.getIterable();
    for (blockid_t i = 0; i < blocks.size(); ++i) {
        auto def = blocks[i];
        refresh(*def, atlas);
    }
}

ContentGfxCache::~ContentGfxCache() = default;

const model::Model& ContentGfxCache::getModel(blockid_t id) const {
    const auto& found = models.find(id);
    if (found == models.end()) {
        LOG_ERROR("Model not found");
        throw std::runtime_error("Model not found");
    }
    return found->second;
}
