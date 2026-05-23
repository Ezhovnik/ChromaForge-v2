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

ContentGfxCache::ContentGfxCache(const Content* content, Assets* assets) : content(content) {
    const ContentIndices* contentIds = content->getIndices();
    sideregions = std::make_unique<UVRegion[]>(contentIds->blocks.count() * 6);
    Atlas* atlas = assets->get<Atlas>("blocks");

    const auto& blocks = contentIds->blocks.getIterable();
    for (blockid_t i = 0; i < blocks.size(); ++i) {
        auto def = blocks[i];
        for (uint side = 0; side < 6; ++side) {
            const std::string& tex = def->textureFaces[side];
            if (atlas->has(tex)) {
                sideregions[i * 6 + side] = atlas->get(tex);
            } else if (atlas->has(TEXTURE_NOTFOUND)) {
                sideregions[i * 6 + side] = atlas->get(TEXTURE_NOTFOUND);
            }
        }
        if (def->model == BlockModel::Custom) {
            auto model = assets->require<model::Model>(def->modelName);
            if (def->modelName.find(':') == std::string::npos) {
                for (auto& mesh : model.meshes) {
                    size_t pos = mesh.texture.find(':');
                    if (pos == std::string::npos) {
                        continue;
                    }
                    if (auto region = atlas->getIf(mesh.texture.substr(pos+1))) {
                        for (auto& vertex : mesh.vertices) {
                            vertex.uv = region->apply(vertex.uv);
                        }
                    }
                }
            }
            models[def->rt.id] = std::move(model);
        }
    }
}

ContentGfxCache::~ContentGfxCache() = default;

const Content* ContentGfxCache::getContent() const {
    return content;
}

const model::Model& ContentGfxCache::getModel(blockid_t id) const {
    const auto& found = models.find(id);
    if (found == models.end()) {
        LOG_ERROR("Model not found");
        throw std::runtime_error("Model not found");
    }
    return found->second;
}
