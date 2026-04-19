#include "LightMap.h"

#include <assert.h>

#include <util/data_io.h>

static_assert(sizeof(light_t) == 2, "Replace the dataio calls with the new light_t value");

void LightMap::set(const LightMap* light_map) {
	set(light_map->map);
}

void LightMap::set(const light_t* map) {
	for (size_t i = 0; i < CHUNK_VOLUME; ++i) {
        this->map[i] = map[i];
    }
}

std::unique_ptr<ubyte[]> LightMap::encode() const {
	auto buffer = std::make_unique<ubyte[]>(LIGHTMAP_DATA_LEN);
	for (uint i = 0; i < CHUNK_VOLUME; i += 2) {
		buffer[i / 2] = ((map[i] >> 12) & 0xF) | ((map[i + 1] >> 8) & 0xF0);
	}
	return buffer;
}

std::unique_ptr<light_t[]> LightMap::decode(const ubyte* buffer) {
	auto lights = std::make_unique<light_t[]>(CHUNK_VOLUME);
	for (uint i = 0; i < CHUNK_VOLUME; i += 2) {
		ubyte b = buffer[i / 2];
		lights[i] = ((b & 0xF) << 12);
		lights[i + 1] = ((b & 0xF0) << 8);
	} 
	return lights;
}
