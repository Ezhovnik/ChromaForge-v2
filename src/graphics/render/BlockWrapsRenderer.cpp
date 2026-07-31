#include <graphics/render/BlockWrapsRenderer.h>

#include <assets/Assets.h>
#include <assets/assets_util.h>
#include <constants.h>
#include <content/Content.h>
#include <graphics/core/Atlas.h>
#include <graphics/core/ShaderProgram.h>
#include <graphics/core/DrawContext.h>
#include <graphics/render/MainBatch.h>
#include <objects/Player.h>
#include <voxels/Block.h>
#include <voxels/Chunks.h>
#include <world/Level.h>

BlockWrapsRenderer::BlockWrapsRenderer(
    const Assets& assets, const Level& level, const Chunks& chunks
) : assets(assets),
    level(level),
    chunks(chunks),
    batch(std::make_unique<MainBatch>(1024)) {}

BlockWrapsRenderer::~BlockWrapsRenderer() = default;

void BlockWrapsRenderer::draw(const BlockWrapper& wrapper) {
    auto& shader = assets.require<ShaderProgram>("entity");
    shader.use();
    shader.uniform1i("u_alphaClip", false);

    util::TextureRegion texRegions[6] {};
    const Texture* texture = nullptr;
    UVRegion uvRegions[6] {};
    for (int i = 0; i < 6; ++i) {
        if (wrapper.cullingBits & (1 << i) == 0) {
            continue;
        }
        auto texRegion = util::get_texture_region(assets, wrapper.textureFaces[i], "");
        texRegions[i] = texRegion;
        uvRegions[i] = texRegion.region;

        if (texture == nullptr) texture = texRegion.texture;
    }
    batch->setTexture(texture);

    const voxel* vox = chunks.getVoxel(wrapper.position);
    if (vox == nullptr || vox->id == BLOCK_VOID) return;

    const auto& def = level.content.getIndices()->blocks.require(vox->id);
    switch (def.getModel(vox->state.userbits).type) {
        case BlockModelType::Cube:
            batch->cube(
                glm::vec3(wrapper.position) + glm::vec3(0.5f),
                glm::vec3(1.01f),
                uvRegions,
                glm::vec4(1, 1, 1, 0),
                false,
                wrapper.cullingBits
            );
            break;
        case BlockModelType::AABB: {
            const auto& aabb =
                (def.rotatable ? def.rt.hitboxes[vox->state.rotation] : def.hitboxes).at(0);
            const auto& size = aabb.size();
            uvRegions[0].scale(size.z, size.y);
            uvRegions[1].scale(size.z, size.y);
            uvRegions[2].scale(size.x, size.z);
            uvRegions[3].scale(size.x, size.z);
            uvRegions[4].scale(size.x, size.y);
            uvRegions[5].scale(size.x, size.y);
            batch->cube(
                glm::vec3(wrapper.position) + aabb.center(),
                size * glm::vec3(1.01f),
                uvRegions,
                glm::vec4(1, 1, 1, 0),
                false,
                wrapper.cullingBits
            );
            break;
        }
        default:
            break;
    }
}

void BlockWrapsRenderer::draw(const DrawContext& pctx, const Player& player) {
    auto ctx = pctx.sub();
    for (const auto& [_, wrapper] : wrappers) {
        draw(*wrapper);
    }
    batch->flush();
}

uint64_t BlockWrapsRenderer::add(
    const glm::ivec3& position, const std::string& texture
) {
    uint64_t id = nextWrapper++;
    wrappers[id] = std::make_unique<BlockWrapper>(
        BlockWrapper {position, texture}
    );
    return id;
}

BlockWrapper* BlockWrapsRenderer::get(uint64_t id) const {
    const auto& found = wrappers.find(id);
    if (found == wrappers.end()) {
        return nullptr;
    }
    return found->second.get();
}

void BlockWrapsRenderer::remove(uint64_t id) {
    wrappers.erase(id);
}
