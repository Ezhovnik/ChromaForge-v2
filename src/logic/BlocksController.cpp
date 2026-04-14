#include "BlocksController.h"

#include <cassert>

#include "../voxels/voxel.h"
#include "../voxels/Block.h"
#include "../voxels/Chunk.h"
#include "../voxels/Chunks.h"
#include "../world/Level.h"
#include "../content/Content.h"
#include "../lighting/Lighting.h"
#include "../util/timeutil.h"
#include "scripting/scripting.h"
#include "../math/rand.h"
#include "../items/Inventories.h"
#include "../debug/Logger.h"
#include "../constants.h"

Clock::Clock(int sparkRate, int sparkParts) : sparkRate(sparkRate), sparkParts(sparkParts) {
}

bool Clock::update(float delta) {
    sparkTimer += delta;
    float delay = 1.0f / float(sparkRate);    
    if (sparkTimer > delay || sparkPartsUndone) {
        if (sparkPartsUndone) {
            sparkPartsUndone--;
        } else {
            sparkTimer = fmod(sparkTimer, delay);
            sparkPartsUndone = sparkParts - 1;
        }
        return true;
    }
    return false;
}

int Clock::getParts() const {
    return sparkParts;
}

int Clock::getPart() const {
    return sparkParts - sparkPartsUndone - 1;
}

int Clock::getSparkRate() const {
    return sparkRate;
}

int Clock::getSparkId() const {
    return sparkId;
}

BlocksController::BlocksController(
    Level* level, 
    uint padding
) : level(level), 
    chunks(level->chunks.get()), 
    lighting(level->lighting.get()), 
    randSparkClock(20, 3), 
    padding(padding), 
    blocksSparkClock(20, 1),
    worldSparkClock(20, 1) {}

void BlocksController::updateSides(int x, int y, int z) {
    updateBlock(x - 1, y, z);
    updateBlock(x + 1, y, z);
    updateBlock(x, y - 1, z);
    updateBlock(x, y + 1, z);
    updateBlock(x, y, z - 1);
    updateBlock(x, y, z + 1);
}

void BlocksController::breakBlock(Player* player, const Block* def, int x, int y, int z) {
    chunks->setVoxel(x, y, z, BLOCK_AIR, {});
    lighting->onBlockSet(x, y, z, BLOCK_AIR);
    scripting::on_block_broken(player, def, x, y, z);
    updateSides(x, y, z);
}

void BlocksController::updateBlock(int x, int y, int z) {
    voxel* vox = chunks->getVoxel(x, y, z);
    if (vox == nullptr) return;
    auto def = level->content->getIndices()->blocks.get(vox->id);
    if (def->grounded) {
        const auto& vec = get_ground_direction(def, vox->state.rotation);
        if (!chunks->isSolidBlock(x + vec.x, y + vec.y, z + vec.z)) {
            breakBlock(nullptr, def, x, y, z);
            return;
        }
    }
    if (def->rt.funcsset.update) scripting::update_block(def, x, y, z);
}

void BlocksController::update(float delta) {
    if (randSparkClock.update(delta)) randomSpark(randSparkClock.getPart(), randSparkClock.getParts());
    if (blocksSparkClock.update(delta)) onBlocksSpark(blocksSparkClock.getPart(), blocksSparkClock.getParts());
    if (worldSparkClock.update(delta)) scripting::on_world_spark();
}

void BlocksController::onBlocksSpark(int sparkId, int parts) {
    auto content = level->content;
    auto indices = content->getIndices();
    int sparkRate = blocksSparkClock.getSparkRate();
    for (size_t id = 0; id < indices->blocks.count(); ++id) {
        if ((id + sparkId) % parts != 0) continue;
        auto def = indices->blocks.get(id);
        auto interval = def->sparkInterval;
        if (def->rt.funcsset.onblocksspark && sparkId / parts % interval == 0) {
            scripting::on_blocks_spark(def, sparkRate / interval);
        }
    }
}

void BlocksController::randomSpark(int sparkId, int parts) {
    const uint w = chunks->width;
    const uint d = chunks->depth;
    int segments = 4;
    int segheight = CHUNK_HEIGHT / segments;
    auto indices = level->content->getIndices();
    for (uint z = padding; z < d - padding; ++z){
        for (uint x = padding; x < w - padding; ++x){
            int index = z * w + x;
            if ((index + sparkId) % parts != 0) continue;
            auto& chunk = chunks->chunks[index];
            if (chunk == nullptr || !chunk->flags.lighted) continue;
            for (int s = 0; s < segments; ++s) {
                for (int i = 0; i < 4; ++i) {
                    int bx = random.rand() % CHUNK_WIDTH;
                    int by = random.rand() % segheight + s * segheight;
                    int bz = random.rand() % CHUNK_DEPTH;
                    const voxel& vox = chunk->voxels[(by * CHUNK_DEPTH + bz) * CHUNK_WIDTH + bx];
                    // std::cout << bx << " " << by << " " << bz << " " << vox.id << std::endl;
                    Block* block = indices->blocks.get(vox.id);
                    if (block != nullptr && block->rt.funcsset.randupdate) {
                        scripting::random_update_block(
                            block, 
                            chunk->chunk_x * CHUNK_WIDTH + bx, by, 
                            chunk->chunk_z * CHUNK_DEPTH + bz
                        );
                    }
                }
            }
        }
	}
}

int64_t BlocksController::createBlockInventory(int x, int y, int z) {
	auto chunk = chunks->getChunkByVoxel(x, y, z);
	if (chunk == nullptr) return 0;
	int lx = x - chunk->chunk_x * CHUNK_WIDTH;
	int lz = z - chunk->chunk_z * CHUNK_DEPTH;
	auto inv = chunk->getBlockInventory(lx, y, lz);
	if (inv == nullptr) {
        auto indices = level->content->getIndices();
        auto def = indices->blocks.get(chunk->voxels[vox_index(lx, y, lz)].id);
        int invsize = def->inventorySize;
        if (invsize == 0) return 0;
		inv = level->inventories->create(invsize);
        chunk->addBlockInventory(inv, lx, y, lz);
	}
    return inv->getId();
}

void BlocksController::bindInventory(int64_t invId, int x, int y, int z) {
    auto chunk = chunks->getChunkByVoxel(x, y, z);
	if (chunk == nullptr) {
        LOG_ERROR("Block does not exists");
		throw std::runtime_error("Block does not exists");
	}
    if (invId <= 0) {
        LOG_ERROR("Unable to bind virtual inventory");
        throw std::runtime_error("Unable to bind virtual inventory");
    }
	int lx = x - chunk->chunk_x * CHUNK_WIDTH;
	int lz = z - chunk->chunk_z * CHUNK_DEPTH;
    chunk->addBlockInventory(level->inventories->get(invId), lx, y, lz);
}

void BlocksController::unbindInventory(int x, int y, int z) {
    auto chunk = chunks->getChunkByVoxel(x, y, z);
	if (chunk == nullptr) {
        LOG_ERROR("Block does not exists");
		throw std::runtime_error("block does not exists");
	}
    int lx = x - chunk->chunk_x * CHUNK_WIDTH;
	int lz = z - chunk->chunk_z * CHUNK_DEPTH;
    chunk->removeBlockInventory(lx, y, lz);
}
