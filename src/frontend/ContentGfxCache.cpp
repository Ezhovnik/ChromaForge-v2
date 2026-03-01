#include "ContentGfxCache.h"

#include <string>

#include "../assets/Assets.h"
#include "../content/Content.h"
#include "../graphics/Atlas.h"
#include "../voxels/Block.h"
#include "../core_defs.h"

ContentGfxCache::ContentGfxCache(const Content* content, Assets* assets) {
    const ContentIndices* contentIds = content->indices;
    sideregions = new UVRegion[contentIds->countBlockDefs() * 6];
	Atlas* atlas = assets->getAtlas("blocks");

	bool hasNotFound = atlas->has(TEXTURE_NOTFOUND);

	for (uint i = 0; i < contentIds->countBlockDefs(); ++i) {
		Block* def = contentIds->getBlockDef(i);
		for (uint side = 0; side < 6; ++side) {
			std::string tex = def->textureFaces[side];
			if (atlas->has(tex)) sideregions[i * 6 + side] = atlas->get(tex);
            else if (hasNotFound) sideregions[i * 6 + side] = atlas->get(TEXTURE_NOTFOUND);
		}

		for (uint side = 0; side < def->modelTextures.size(); ++side) {
			std::string tex = def->modelTextures[side];
			if (atlas->has(tex)) def->modelUVs.push_back(atlas->get(tex));
			else if (hasNotFound) def->modelUVs.push_back(atlas->get(TEXTURE_NOTFOUND));
		}
    }
}

ContentGfxCache::~ContentGfxCache() {
	delete[] sideregions;
}
