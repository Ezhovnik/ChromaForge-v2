#ifndef LIGHTING_LIGHTMAP_H_
#define LIGHTING_LIGHTMAP_H_

#include "../voxels/Chunk.h"
#include "../typedefs.h"
#include "../constants.h"

class LightMap {
public:
	light_t* map;
    int highestPoint = 0;

	LightMap();
	~LightMap();

    void set(const LightMap* light_map);

    inline ushort get(int x, int y, int z){
		return (map[(y * CHUNK_DEPTH + z) * CHUNK_WIDTH + x]);
	}

	inline ubyte get(int x, int y, int z, int channel){
		return (map[(y * CHUNK_DEPTH + z) * CHUNK_WIDTH + x] >> (channel << 2)) & 0xF;
	}

	inline ubyte getR(int x, int y, int z){
		return map[(y * CHUNK_DEPTH + z) * CHUNK_WIDTH + x] & 0xF;
	}

	inline ubyte getG(int x, int y, int z){
		return (map[(y * CHUNK_DEPTH + z) * CHUNK_WIDTH + x] >> 4) & 0xF;
	}

	inline ubyte getB(int x, int y, int z){
		return (map[(y * CHUNK_DEPTH + z) * CHUNK_WIDTH + x] >> 8) & 0xF;
	}

	inline ubyte getS(int x, int y, int z){
		return (map[(y * CHUNK_DEPTH + z) * CHUNK_WIDTH + x] >> 12) & 0xF;
	}

	inline void setR(int x, int y, int z, int value){
		const int index = (y * CHUNK_DEPTH + z) * CHUNK_WIDTH + x;
		map[index] = (map[index] & 0xFFF0) | value;
	}

	inline void setG(int x, int y, int z, int value){
		const int index = (y * CHUNK_DEPTH + z) * CHUNK_WIDTH + x;
		map[index] = (map[index] & 0xFF0F) | (value << 4);
	}

	inline void setB(int x, int y, int z, int value){
		const int index = (y * CHUNK_DEPTH + z) * CHUNK_WIDTH + x;
		map[index] = (map[index] & 0xF0FF) | (value << 8);
	}

	inline void setS(int x, int y, int z, int value){
		const int index = (y * CHUNK_DEPTH + z) * CHUNK_WIDTH + x;
		map[index] = (map[index] & 0x0FFF) | (value << 12);
	}

	inline void set(int x, int y, int z, int channel, int value){
		const int index = (y * CHUNK_DEPTH + z) * CHUNK_WIDTH + x;
		map[index] = (map[index] & (0xFFFF & (~(0xF << (channel*4))))) | (value << (channel << 2));
	}

    inline const light_t* getLights() const {
		return map;
	}

	inline light_t* getLightsWriteable() {
		return map;
	}

	static inline light_t extract(light_t light, ubyte channel) {
		return (light >> (channel << 2)) & 0xF;
	}

	static inline light_t combine(int r, int g, int b, int s) {
		return r | (g << 4) | (b << 8) | (s << 12);
	}
};

#endif // LIGHTING_LIGHTMAP_H_
