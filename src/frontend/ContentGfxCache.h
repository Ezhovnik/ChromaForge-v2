#pragma once

#include <memory>
#include <stdexcept>
#include <unordered_map>

#include <math/UVRegion.h>
#include <typedefs.h>
#include <graphics/commons/Model.h>

class Content;
class Assets;
class Atlas;
class Block;
struct GraphicsSettings;

class ContentGfxCache {
private:
    const Content& content;
    const Assets& assets;
    const GraphicsSettings& settings;
    std::unique_ptr<UVRegion[]> sideregions;
    std::unordered_map<blockid_t, model::Model> models;
public:
    ContentGfxCache(
        const Content& content,
        const Assets& assets,
        const GraphicsSettings& settings
    );
    ~ContentGfxCache();

    inline const UVRegion& getRegion(blockid_t id, int side) const {
        return sideregions[id * 6 + side];
    }

    const model::Model& getModel(blockid_t id) const;

    void refresh(const Block& block, const Atlas& atlas);
    void refresh();
};
