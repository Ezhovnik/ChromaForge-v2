#include "Chunk.h"

#include <math.h>

#include <glm/glm.hpp>
#include <glm/gtc/noise.hpp>

#include "../items/Inventory.h"
#include "voxel.h"
#include "../lighting/LightMap.h"
#include "../content/ContentLUT.h"

// Конструктор
Chunk::Chunk(int chunk_x, int chunk_z) : chunk_x(chunk_x), chunk_z(chunk_z) {
    bottom = 0;
	top = CHUNK_HEIGHT;

	for (uint i = 0; i < CHUNK_VOLUME; ++i) {
		voxels[i].id = BLOCK_AIR;
		voxels[i].states = 0;
	}
}

// Проверяет, является ли чанк пустым (однородным).
bool Chunk::isEmpty() {
    int id = -1;
	for (uint i = 0; i < CHUNK_VOLUME; ++i){
		if (voxels[i].id != id){
			if (id != -1) return false;
			else id = voxels[i].id;
		}
	}
	return true;
}

void Chunk::updateHeights() {
	for (uint i = 0; i < CHUNK_VOLUME; i++) {
		if (voxels[i].id != 0) {
			bottom = i / (CHUNK_DEPTH * CHUNK_WIDTH);
			break;
		}
	}

	for (int i = CHUNK_VOLUME - 1; i >= 0; i--) {
		if (voxels[i].id != 0) {
			top = i / (CHUNK_DEPTH * CHUNK_WIDTH) + 1;
			break;
		}
	}
}

void Chunk::addBlockInventory(std::shared_ptr<Inventory> inventory, uint x, uint y, uint z) {
    inventories[vox_index(x, y, z)] = inventory;
    setUnsaved(true);
}

std::shared_ptr<Inventory> Chunk::getBlockInventory(uint x, uint y, uint z) const {
    if (x >= CHUNK_WIDTH || y >= CHUNK_HEIGHT || z >= CHUNK_DEPTH) return nullptr;
    const auto& found = inventories.find(vox_index(x, y, z));
    if (found == inventories.end()) return nullptr;
    return found->second;
}

void Chunk::removeBlockInventory(uint x, uint y, uint z) {
	if (inventories.erase(vox_index(x, y, z))) setUnsaved(true);
}

void Chunk::setBlockInventories(chunk_inventories_map map) {
	inventories = map;
}

// Создает полную копию текущего чанка.
std::unique_ptr<Chunk> Chunk::clone() const {
	auto other = std::make_unique<Chunk>(chunk_x, chunk_z);
	for (uint i = 0; i < CHUNK_VOLUME; ++i) {
		other->voxels[i] = voxels[i];
    }
	other->light_map.set(&light_map);
	return other;
}

// Формат: [voxel_ids...][voxel_states...];
ubyte* Chunk::encode() const {
	ubyte* buffer = new ubyte[CHUNK_DATA_LEN];
	for (uint i = 0; i < CHUNK_VOLUME; ++i) {
		buffer[i] = voxels[i].id >> 8;
        buffer[CHUNK_VOLUME + i] = voxels[i].id & 0xFF;
		buffer[CHUNK_VOLUME * 2 + i] = voxels[i].states >> 8;
        buffer[CHUNK_VOLUME * 3 + i] = voxels[i].states & 0xFF;
	}
	return buffer;
}

bool Chunk::decode(const ubyte* data) {
	for (uint i = 0; i < CHUNK_VOLUME; ++i) {
		voxel& vox = voxels[i];

		ubyte bid1 = data[i];
        ubyte bid2 = data[CHUNK_VOLUME + i];
        
        ubyte bst1 = data[CHUNK_VOLUME * 2 + i];
        ubyte bst2 = data[CHUNK_VOLUME * 3 + i];

		vox.id = (blockid_t(bid1) << 8) | (blockid_t(bid2));
        vox.states = (blockstate_t(bst1) << 8) | (blockstate_t(bst2));
	}
	return true;
}

void Chunk::convert(ubyte* data, const ContentLUT* lut) {
    for (uint i = 0; i < CHUNK_VOLUME; ++i) {
        blockid_t id = ((blockid_t(data[i]) << 8) | blockid_t(data[CHUNK_VOLUME + i]));
        blockid_t replacement = lut->getBlockId(id);
        data[i] = replacement >> 8;
        data[CHUNK_VOLUME + i] = replacement & 0xFF;
    }
}
