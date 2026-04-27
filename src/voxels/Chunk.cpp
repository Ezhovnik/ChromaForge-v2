#include <voxels/Chunk.h>

#include <math.h>
#include <utility>

#include <glm/glm.hpp>
#include <glm/gtc/noise.hpp>

#include <items/Inventory.h>
#include <voxels/voxel.h>
#include <lighting/LightMap.h>
#include <content/ContentReport.h>
#include <util/data_io.h>

// Конструктор
Chunk::Chunk(int chunk_x, int chunk_z) : chunk_x(chunk_x), chunk_z(chunk_z) {
    bottom = 0;
	top = CHUNK_HEIGHT;
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
    inventories[vox_index(x, y, z)] = std::move(inventory);
    flags.unsaved = true;
}

std::shared_ptr<Inventory> Chunk::getBlockInventory(uint x, uint y, uint z) const {
    if (x >= CHUNK_WIDTH || y >= CHUNK_HEIGHT || z >= CHUNK_DEPTH) return nullptr;
    const auto& found = inventories.find(vox_index(x, y, z));
    if (found == inventories.end()) return nullptr;
    return found->second;
}

void Chunk::removeBlockInventory(uint x, uint y, uint z) {
	if (inventories.erase(vox_index(x, y, z))) flags.unsaved = true;
}

void Chunk::setBlockInventories(chunk_inventories_map map) {
	inventories = std::move(map);
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

std::unique_ptr<ubyte[]> Chunk::encode() const {
    auto buffer = std::make_unique<ubyte[]>(CHUNK_DATA_LEN);
    auto dst = reinterpret_cast<uint16_t*>(buffer.get());
    for (uint i = 0; i < CHUNK_VOLUME; ++i) {
        dst[i] = dataio::h2le(voxels[i].id);
        dst[CHUNK_VOLUME + i] = dataio::h2le(blockstate2int(voxels[i].state));
    }
    return buffer;
}

bool Chunk::decode(const ubyte* data) {
    auto src = reinterpret_cast<const uint16_t*>(data);
    for (uint i = 0; i < CHUNK_VOLUME; ++i) {
        voxel& vox = voxels[i];

        vox.id = dataio::le2h(src[i]);
        vox.state = int2blockstate(dataio::le2h(src[CHUNK_VOLUME + i]));
    }
    return true;
}

void Chunk::convert(ubyte* data, const ContentReport* report) {
    auto buffer = reinterpret_cast<uint16_t*>(data);
    for (uint i = 0; i < CHUNK_VOLUME; ++i) {
        blockid_t id = dataio::le2h(buffer[i]);
        blockid_t replacement = report->blocks.getId(id);
        buffer[i] = dataio::h2le(replacement);
    }
}
