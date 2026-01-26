#include "LightMap.h"

LightMap::LightMap(){
	map = new ushort[CHUNK_VOLUME];
	for (uint i = 0; i < CHUNK_VOLUME; ++i){
		map[i] = 0x0000;
	}
}

LightMap::~LightMap(){
	delete[] map;
}

void LightMap::set(const LightMap* light_map) {
	for (uint i = 0; i < CHUNK_VOLUME; i++){
		map[i] = light_map->map[i];
	}
}
