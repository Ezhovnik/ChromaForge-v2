#ifndef LIGHTING_LIGHTING_H_
#define LIGHTING_LIGHTING_H_

#include "../typedefs.h"

class Content;
class Chunks;
class LightSolver;

class Lighting {
    const Content* const content;
    Chunks* chunks = nullptr;
    LightSolver* solverR = nullptr;
    LightSolver* solverG = nullptr;
    LightSolver* solverB = nullptr;
    LightSolver* solverS = nullptr;

    blockid_t airID;
public:
    Lighting(const Content* content, Chunks* chunks);
	~Lighting();

    void clear();
    void preBuildSkyLight(int chunk_x, int chunk_z);
    void buildSkyLight(int chunk_x, int chunk_z);
    void onChunkLoaded(int chunk_x, int chunk_z);
    void onBlockSet(int x, int y, int z, int id);
};

#endif // LIGHTING_LIGHTING_H_
