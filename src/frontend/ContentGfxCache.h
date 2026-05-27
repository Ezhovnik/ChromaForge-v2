#pragma once

#include <memory>
#include <stdexcept>
#include <unordered_map>

#include <math/UVRegion.h>
#include <typedefs.h>
#include <graphics/commons/Model.h>

class Content;
class Assets;

class ContentGfxCache {
private:
    const Content* content;
    std::unique_ptr<UVRegion[]> sideregions;
    std::unordered_map<blockid_t, model::Model> models;
public:
    ContentGfxCache(const Content* content, const Assets& assets);
    ~ContentGfxCache();

    inline const UVRegion& getRegion(blockid_t id, int side) const {
        return sideregions[id * 6 + side];
    }

    const model::Model& getModel(blockid_t id) const;

    const Content* getContent() const;
};
