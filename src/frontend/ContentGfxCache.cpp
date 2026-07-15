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

void ContentGfxCache::refreshVariant(
    const Block& def,
    const Variant& variant,
    uint8_t variantIndex,
    const Atlas& atlas
) {
    bool denseRender = settings.denseRender.get();
    for (uint side = 0; side < 6; ++side) {
        std::string tex = variant.textureFaces[side];
        std::string texOpaque = tex + "_opaque";

        if (!atlas.has(tex)) {
            tex = TEXTURE_NOTFOUND;
        }
        if (!atlas.has(texOpaque)) {
            texOpaque = tex;
        } else if (variant.culling == CullingMode::Optional && !denseRender) {
            tex = texOpaque;
        }
        size_t index = getRegionIndex(def.rt.id, variantIndex, side, false);
        sideregions[index] = atlas.get(tex);
        sideregions[index + 1] = atlas.get(texOpaque);
    }
    if (variant.model.type == BlockModelType::Custom) {
        auto model = assets.require<model::Model>(variant.model.name);
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

void ContentGfxCache::refresh(const Block& def, const Atlas& atlas) {
    refreshVariant(def, def.defaults, 0, atlas);
    if (def.variants) {
        const auto& variants = def.variants->variants;
        for (int i = 1; i < variants.size() - 1; ++i) {
            refreshVariant(def, variants[i], i, atlas);
        }
        def.variants->variants.at(0) = def.defaults;
    }
}

void ContentGfxCache::refresh() {
    auto indices = content.getIndices();
    size_t size = indices->blocks.count() * GFXC_SIDES * GFXC_MAX_VARIANTS * 2;

    LOG_INFO("UV cache size is {} B", (sizeof(UVRegion) * size));

    sideregions = std::make_unique<UVRegion[]>(size);
    const auto& atlas = assets.require<Atlas>("blocks");

    const auto& blocks = indices->blocks.getIterable();
    for (blockid_t i = 0; i < blocks.size(); ++i) {
        refresh(*blocks[i], atlas);
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
