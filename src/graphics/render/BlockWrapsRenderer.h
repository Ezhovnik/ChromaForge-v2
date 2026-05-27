#pragma once

#include <string>
#include <memory>
#include <unordered_map>

#include <graphics/render/MainBatch.h>
#include <typedefs.h>

class Assets;
class Player;
class Level;
class DrawContext;

struct BlockWrapper {
    glm::ivec3 position;
    std::string texture;
};

class BlockWrapsRenderer {
    const Assets& assets;
    const Level& level;
    std::unique_ptr<MainBatch> batch;

    std::unordered_map<uint64_t, std::unique_ptr<BlockWrapper>> wrappers;
    uint64_t nextWrapper = 1;

    void draw(const BlockWrapper& wrapper);
public:
    BlockWrapsRenderer(const Assets& assets, const Level& level);
    ~BlockWrapsRenderer();

    void draw(const DrawContext& ctx, const Player& player);

    uint64_t add(const glm::ivec3& position, const std::string& texture);

    BlockWrapper* get(uint64_t id) const;

    void remove(uint64_t id);
};
