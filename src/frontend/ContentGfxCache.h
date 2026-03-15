#ifndef FRONTEND_BLOCKS_GFX_CACHE_H_
#define FRONTEND_BLOCKS_GFX_CACHE_H_

#include <memory>
#include <string>
#include <unordered_map>

#include "../graphics/core/UVRegion.h"
#include "../typedefs.h"

class Content;
class Assets;
class UIDocument;

using uidocuments_map = std::unordered_map<std::string, std::shared_ptr<UIDocument>>;

class ContentGfxCache {
    const Content* content;
    std::unique_ptr<UVRegion[]> sideregions;
    uidocuments_map layouts;
public:
    ContentGfxCache(const Content* content, Assets* assets);
    ~ContentGfxCache();

    inline const UVRegion& getRegion(blockid_t id, int side) const {
        return sideregions[id * 6 + side];
    }

    std::shared_ptr<UIDocument> getLayout(const std::string& id);

    const Content* getContent() const;
};

#endif // FRONTEND_BLOCKS_GFX_CACHE_H_
