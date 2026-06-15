#include <voxels/GlobalChunks.h>

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
#include <coders/json.h>
#include <voxels/blocks_agent.h>

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

GlobalChunks::GlobalChunks(
    Level* level
) : level(level),
    indices(level ? level->content->getIndices() : nullptr)
{
    chunksMap.max_load_factor(CHUNKS_MAP_MAX_LOAD_FACTOR);
}

std::shared_ptr<Chunk> GlobalChunks::fetch(int x, int z) {
    const auto& found = chunksMap.find(keyfrom(x, z));
    if (found == chunksMap.end()) {
        return nullptr;
    }
    return found->second;
}

void GlobalChunks::erase(int x, int z) {
    chunksMap.erase(keyfrom(x, z));
}

static inline auto load_inventories(
    WorldRegions& regions,
    const Chunk& chunk,
    const ContentUnitIndices<Block>& defs
) {
    auto invs = regions.fetchInventories(chunk.chunk_x, chunk.chunk_z);
    auto iterator = invs.begin();
    while (iterator != invs.end()) {
        uint index = iterator->first;
        const auto& def = defs.require(chunk.voxels[index].id);
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
    return invs;
}

std::shared_ptr<Chunk> GlobalChunks::create(int x, int z) {
	const auto& found = chunksMap.find(keyfrom(x, z));
    if (found != chunksMap.end()) return found->second;

	auto chunk = std::make_shared<Chunk>(x, z);
    chunksMap[keyfrom(x, z)] = chunk;

	World* world = level->getWorld();
	auto& regions = world->wfile.get()->getRegions();

	auto& localChunksMap = chunksMap;

	if (auto data = regions.getVoxels(chunk->chunk_x, chunk->chunk_z)) {
		const auto& indices = *level->content->getIndices();

		chunk->decode(data.get());
		check_voxels(indices, *chunk);

		chunk->setBlockInventories(
            load_inventories(regions, *chunk, indices.blocks)
        );

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

void GlobalChunks::pinChunk(std::shared_ptr<Chunk> chunk) {
    pinnedChunks[{chunk->chunk_x, chunk->chunk_z}] = std::move(chunk);
}

void GlobalChunks::unpinChunk(int x, int z) {
    pinnedChunks.erase({x, z});
}

size_t GlobalChunks::size() const {
    return chunksMap.size();
}

void GlobalChunks::incref(Chunk* chunk) {
    auto key = reinterpret_cast<ptrdiff_t>(chunk);
    const auto& found = refCounters.find(key);
    if (found == refCounters.end()) {
        refCounters[key] = 1;
        return;
    }
    found->second++;
}

void GlobalChunks::decref(Chunk* chunk) {
    auto key = reinterpret_cast<ptrdiff_t>(chunk);
    const auto& found = refCounters.find(key);
    if (found == refCounters.end()) {
        abort();
    }
    if (--found->second == 0) {
        union {
            int pos[2];
            long long key;
        } ekey;
        ekey.pos[0] = chunk->chunk_x;
        ekey.pos[1] = chunk->chunk_z;

        save(chunk);
        chunksMap.erase(ekey.key);
        refCounters.erase(found);
    }
}

void GlobalChunks::save(Chunk* chunk) {
    if (chunk == nullptr) return;

    AABB aabb(
        glm::vec3(chunk->chunk_x * CHUNK_WIDTH, -INFINITY, chunk->chunk_z * CHUNK_DEPTH),
        glm::vec3(
            (chunk->chunk_x + 1) * CHUNK_WIDTH, INFINITY, (chunk->chunk_z + 1) * CHUNK_DEPTH
        )
    );
    auto entities = level->entities->getAllInside(aabb);
    auto root = dv::object();
    root["data"] = level->entities->serialize(entities);
    if (!entities.empty()) {
        level->entities->despawn(std::move(entities));
        chunk->flags.entities = true;
    }
    level->getWorld()->wfile->getRegions().put(
        chunk,
        chunk->flags.entities ? json::to_binary(root, true) : std::vector<ubyte>()
    );
}

void GlobalChunks::saveAll() {
    for (const auto& [_, chunk] : chunksMap) {
        save(chunk.get());
    }
}

void GlobalChunks::putChunk(std::shared_ptr<Chunk> chunk) {
    chunksMap[keyfrom(chunk->chunk_x, chunk->chunk_z)] = std::move(chunk);
}

const AABB* GlobalChunks::isObstacleAt(float x, float y, float z) const {
    return blocks_agent::is_obstacle_at(*this, x, y, z);
}
