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
    auto textureRegion = util::get_texture_region(assets, wrapper.texture, "");

    auto& shader = assets.require<ShaderProgram>("entity");
    shader.use();
    shader.uniform1i("u_alphaClip", false);

    const UVRegion& cracksRegion = textureRegion.region;
    UVRegion regions[6] {
        cracksRegion, cracksRegion, cracksRegion,
        cracksRegion, cracksRegion, cracksRegion
    };
    batch->setTexture(textureRegion.texture);

    const voxel* vox = chunks.getVoxel(wrapper.position);
    if (vox == nullptr) {
        return;
    }
    if (vox->id != BLOCK_VOID) {
        const auto& def = level.content.getIndices()->blocks.require(vox->id);
        switch (def.getModel(vox->state.userbits).type) {
            case BlockModelType::Cube:
                batch->cube(
                    glm::vec3(wrapper.position) + glm::vec3(0.5f),
                    glm::vec3(1.01f),
                    regions,
                    glm::vec4(1, 1, 1, 0),
                    false
                );
                break;
            case BlockModelType::AABB: {
                const auto& aabb = (def.rotatable ? def.rt.hitboxes[vox->state.rotation] : def.hitboxes).at(0);
                const auto& size = aabb.size();
                regions[0].scale(size.z, size.y);
                regions[1].scale(size.z, size.y);
                regions[2].scale(size.x, size.z);
                regions[3].scale(size.x, size.z);
                regions[4].scale(size.x, size.y);
                regions[5].scale(size.x, size.y);
                batch->cube(
                    glm::vec3(wrapper.position) + aabb.center(),
                    size * glm::vec3(1.01f),
                    regions,
                    glm::vec4(1, 1, 1, 0),
                    false
                );
                break;
            }
            default:
                break;
        }
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
