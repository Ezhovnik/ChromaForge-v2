#include <voxels/ChunksStorage.h>

#include <assert.h>
#include <algorithm>

#include <voxels/Chunk.h>
#include <voxels/Block.h>
#include <content/Content.h>
#include <world/Level.h>
#include <world/World.h>
#include <files/WorldFiles.h>
#include <math/voxmaths.h>
#include <lighting/Lightmap.h>
#include <debug/Logger.h>
#include <core_content_defs.h>
#include <items/Inventories.h>
#include <objects/Entities.h>

static void check_voxels(const ContentIndices& indices, Chunk& chunk) {
    bool corrupted = false;
	blockid_t defsCount = indices.blocks.count();
	for (size_t i = 0; i < CHUNK_VOLUME; ++i) {
        blockid_t id = chunk.voxels[i].id;
        if (id >= defsCount) {
			if (!corrupted) {
				if (!ENGINE_DEBUG_BUILD) {
					LOG_WARN("Corruped block id = {} detected at {} of chunk {}x {}z", id, i, chunk.chunk_x, chunk.chunk_z);
					corrupted = true;
				} else {
					abort();
				}
			}
			chunk.voxels[i].id = BLOCK_AIR;
        }
    }
}

ChunksStorage::ChunksStorage(
	Level* level
) : level(level),
	chunksMap(std::make_shared<util::WeakPtrsMap<glm::ivec2, Chunk>>()) {}

std::shared_ptr<Chunk> ChunksStorage::fetch(int x, int z) {
    std::lock_guard lock(*chunksMap);
    return chunksMap->fetch({x, z});
}

std::shared_ptr<Chunk> ChunksStorage::create(int x, int z) {
	if (auto ptr = chunksMap->fetch({x, z})) return ptr;

	World* world = level->getWorld();
	auto& regions = world->wfile.get()->getRegions();

	auto& localChunksMap = chunksMap;
    auto chunk = std::shared_ptr<Chunk>(
        new Chunk(x, z),
        [localChunksMap, x, z](Chunk* ptr) {
            std::lock_guard lock(*localChunksMap);
            localChunksMap->erase({x, z});
            delete ptr;
        }
    );
    (*chunksMap)[glm::ivec2(chunk->chunk_x, chunk->chunk_z)] = chunk;

	if (auto data = regions.getVoxels(chunk->chunk_x, chunk->chunk_z)) {
		const auto& indices = *level->content->getIndices();

		chunk->decode(data.get());
		check_voxels(indices, *chunk);

		auto invs = regions.fetchInventories(chunk->chunk_x, chunk->chunk_z);
		auto iterator = invs.begin();
        while (iterator != invs.end()) {
            uint index = iterator->first;
            const auto& def = indices.blocks.require(chunk->voxels[index].id);
            if (def.inventorySize == 0) {
                iterator = invs.erase(iterator);
                continue;
            }
            auto& inventory = iterator->second;
            if (def.inventorySize != inventory->size()) {
                inventory->resize(def.inventorySize);
            }
            ++iterator;
        }
		chunk->setBlockInventories(std::move(invs));

		auto entitiesData = regions.fetchEntities(chunk->chunk_x, chunk->chunk_z);
        if (entitiesData.getType() == dv::value_type::Object) {
            level->entities->loadEntities(std::move(entitiesData));
			chunk->flags.entities = true;
        }

		chunk->flags.loaded = true;
		for (auto& entry : chunk->inventories) {
			level->inventories->store(entry.second);
		}
	}

	if (auto lights = regions.getLights(chunk->chunk_x, chunk->chunk_z)) {
		chunk->light_map.set(lights.get());
		chunk->flags.loadedLights = true;
	}

	chunk->blocksMetadata = regions.getBlocksData(chunk->chunk_x, chunk->chunk_z);
	return chunk;
}
