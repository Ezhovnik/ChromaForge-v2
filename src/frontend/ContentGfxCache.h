#pragma once

#include <memory>

#include <math/UVRegion.h>
#include <typedefs.h>

class Content;
class Assets;
class UIDocument;

class ContentGfxCache {
private:
    const Content* content;
    std::unique_ptr<UVRegion[]> sideregions;
public:
    ContentGfxCache(const Content* content, Assets* assets);
    ~ContentGfxCache();

    inline const UVRegion& getRegion(blockid_t id, int side) const {
        return sideregions[id * 6 + side];
    }

    const Content* getContent() const;
};
