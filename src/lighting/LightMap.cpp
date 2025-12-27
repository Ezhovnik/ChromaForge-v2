#include "LightMap.h"

LightMap::LightMap(){
	map = new unsigned short[CHUNK_VOLUME];
	for (unsigned int i = 0; i < CHUNK_VOLUME; ++i){
		map[i] = 0x0000;
	}
}

LightMap::~LightMap(){
	delete[] map;
}
