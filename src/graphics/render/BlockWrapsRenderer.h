#pragma once

#include <string>
#include <memory>
#include <unordered_map>
#include <array>
#include <map>

#include <graphics/render/MainBatch.h>
#include <typedefs.h>
#include <assets/assets_util.h>
#include <voxels/Block.h>

class Assets;
class Player;
class Level;
class DrawContext;
class Chunks;
struct voxel;
class Texture;

struct BlockWrapper {
    glm::ivec3 position;
    std::array<std::string, 6> textureFaces {};
    float emission = 0.0f;

    // Render cache
    util::TextureRegion texRegions[6] {};
    UVRegion uvRegions[6] {};
    BlockModelType modelType {};
    uint8_t cullingBits = 0xFF;
    uint8_t dirtySides = 0xFF;
};

class BlockWrapsRenderer {
    const Assets& assets;
    const Level& level;
    const Chunks& chunks;
    std::unique_ptr<MainBatch> batch;

    std::multimap<const Texture*, BlockWrapper*> renderOrder;
    std::unordered_map<uint64_t, std::unique_ptr<BlockWrapper>> wrappers;
    uint64_t nextWrapper = 1;

    void draw(BlockWrapper& wrapper, const Texture* texture);

    void refreshWrapper(BlockWrapper& wrapper);
public:
    BlockWrapsRenderer(
        const Assets& assets, const Level& level, const Chunks& chunks
    );
    ~BlockWrapsRenderer();

    void draw(const DrawContext& ctx, const Player& player);

    uint64_t add(
        const glm::ivec3& position,
        const std::string& texture,
        float emission
    );

    BlockWrapper* get(uint64_t id) const;

    void remove(uint64_t id);
};
