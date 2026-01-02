#include "LightMap.h"

LightMap::LightMap(){
	map = new unsigned short[CHUNK_VOLUME];
	for (unsigned int i = 0; i < CHUNK_VOLUME; ++i){
		map[i] = 0x0000;
	}
}

LightMap::~LightMap(){
	if (map != nullptr) delete[] map;
}

void LightMap::set(const LightMap* light_map) {
	for (unsigned int i = 0; i < CHUNK_VOLUME; i++){
		map[i] = light_map->map[i];
	}
}
