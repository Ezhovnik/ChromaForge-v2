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
struct Variant;

inline constexpr int GFXC_MAX_VARIANTS = 16;
inline constexpr int GFXC_SIDES = 6;

class ContentGfxCache {
private:
    const Content& content;
    const Assets& assets;
    const GraphicsSettings& settings;
    std::unique_ptr<UVRegion[]> sideregions;
    std::unordered_map<blockid_t, model::Model> models;

    void refreshVariant(
        const Block& def,
        const Variant& variant,
        uint8_t variantIndex,
        const Atlas& atlas
    );
public:
    ContentGfxCache(
        const Content& content,
        const Assets& assets,
        const GraphicsSettings& settings
    );
    ~ContentGfxCache();

    static inline size_t getRegionIndex(
        blockid_t id, uint8_t variant, int side, bool opaque
    ) {
        return ((id * GFXC_SIDES + side) * GFXC_MAX_VARIANTS + variant) * 2 + opaque;
    }

    inline const UVRegion& getRegion(
        blockid_t id, uint8_t variant, int side, bool dense
    ) const {
        return sideregions[getRegionIndex(id, variant, side, !dense)];
    }

    const model::Model& getModel(blockid_t id) const;

    void refresh(const Block& block, const Atlas& atlas);
    void refresh();
};
