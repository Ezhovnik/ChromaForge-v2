#include "BlocksController.h"

#include <iostream>

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

BlocksController::BlocksController(Level* level, uint padding) : level(level), chunks(level->chunks), lighting(level->lighting), randSparkClock(20, 3), padding(padding) {
}

void BlocksController::updateSides(int x, int y, int z) {
    updateBlock(x - 1, y, z);
    updateBlock(x + 1, y, z);
    updateBlock(x, y - 1, z);
    updateBlock(x, y + 1, z);
    updateBlock(x, y, z - 1);
    updateBlock(x, y, z + 1);
}

void BlocksController::breakBlock(Player* player, const Block* def, int x, int y, int z) {
    chunks->setVoxel(x, y, z, 0, 0);
    lighting->onBlockSet(x, y, z, 0);
    if (def->rt.funcsset.onbroken) scripting::on_block_broken(player, def, x, y, z);
    updateSides(x, y, z);
}

void BlocksController::updateBlock(int x, int y, int z) {
    voxel* vox = chunks->getVoxel(x, y, z);
    if (vox == nullptr) return;
    const Block* def = level->content->indices->getBlockDef(vox->id);
    if (def->grounded && !chunks->isSolidBlock(x, y - 1, z)) {
        breakBlock(nullptr, def, x, y, z);
        return;
    }
    if (def->rt.funcsset.update) scripting::update_block(def, x, y, z);
}

void BlocksController::update(float delta) {
    if (randSparkClock.update(delta)) randomSpark(randSparkClock.getPart(), randSparkClock.getParts());
}

void BlocksController::randomSpark(int sparkId, int parts) {
    const uint w = chunks->width;
    const uint d = chunks->depth;
    auto indices = level->content->indices;
    for (uint z = padding; z < d - padding; ++z){
        for (uint x = padding; x < w - padding; ++x){
            int index = z * w + x;
            if ((index + sparkId) % parts != 0) continue;
            std::shared_ptr<Chunk> chunk = chunks->chunks[index];
            if (chunk == nullptr || !chunk->isLighted()) continue;
            int segments = 4;
            int segheight = CHUNK_HEIGHT / segments;
            for (int s = 0; s < segments; ++s) {
                for (int i = 0; i < 3; ++i) {
                    int bx = abs(RandomGenerator::get<int>()) % CHUNK_WIDTH;
                    int by = abs(RandomGenerator::get<int>()) % segheight + s * segheight;
                    int bz = abs(RandomGenerator::get<int>()) % CHUNK_DEPTH;
                    const voxel& vox = chunk->voxels[(by * CHUNK_DEPTH + bz) * CHUNK_WIDTH + bx];
                    // std::cout << bx << " " << by << " " << bz << " " << vox.id << std::endl;
                    Block* block = indices->getBlockDef(vox.id);
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
