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
#include <coders/json.h>

inline long long keyfrom(int x, int z) {
    union {
        int pos[2];
        long long key;
    } ekey;
    ekey.pos[0] = x;
    ekey.pos[1] = z;
    return ekey.key;
}

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

ChunksStorage::ChunksStorage(Level* level) : level(level) {}

std::shared_ptr<Chunk> ChunksStorage::fetch(int x, int z) {
    const auto& found = chunksMap.find(keyfrom(x, z));
    if (found == chunksMap.end()) {
        return nullptr;
    }
    return found->second;
}

void ChunksStorage::erase(int x, int z) {
    chunksMap.erase(keyfrom(x, z));
}

std::shared_ptr<Chunk> ChunksStorage::create(int x, int z) {
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

void ChunksStorage::pinChunk(std::shared_ptr<Chunk> chunk) {
    pinnedChunks[{chunk->chunk_x, chunk->chunk_z}] = std::move(chunk);
}

void ChunksStorage::unpinChunk(int x, int z) {
    pinnedChunks.erase({x, z});
}

size_t ChunksStorage::size() const {
    return chunksMap.size();
}

voxel* ChunksStorage::get(int x, int y, int z) const {
    if (y < 0 || y >= CHUNK_HEIGHT) return nullptr;

    int cx = floordiv<CHUNK_WIDTH>(x);
    int cz = floordiv<CHUNK_DEPTH>(z);

    const auto& found = chunksMap.find(keyfrom(cx, cz));
    if (found == chunksMap.end()) {
        return nullptr;
    }
    const auto& chunk = found->second;
    int lx = x - cx * CHUNK_WIDTH;
    int lz = z - cz * CHUNK_DEPTH;
    return &chunk->voxels[(y * CHUNK_DEPTH + lz) * CHUNK_WIDTH + lx];
}

void ChunksStorage::incref(Chunk* chunk) {
    auto key = reinterpret_cast<ptrdiff_t>(chunk);
    const auto& found = refCounters.find(key);
    if (found == refCounters.end()) {
        refCounters[key] = 1;
        return;
    }
    found->second++;
}

void ChunksStorage::decref(Chunk* chunk) {
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

void ChunksStorage::save(Chunk* chunk) {
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

void ChunksStorage::saveAll() {
    for (const auto& [_, chunk] : chunksMap) {
        save(chunk.get());
    }
}
