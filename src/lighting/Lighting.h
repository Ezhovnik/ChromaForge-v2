#ifndef LIGHTING_LIGHTING_H_
#define LIGHTING_LIGHTING_H_

#include <memory>

#include "typedefs.h"

class Content;
class Chunks;
class LightSolver;
class Chunk;
class ContentIndices;

class Lighting {
    const Content* const content;
    Chunks* chunks = nullptr;
    std::unique_ptr<LightSolver> solverR;
	std::unique_ptr<LightSolver> solverG;
	std::unique_ptr<LightSolver> solverB;
	std::unique_ptr<LightSolver> solverS;
public:
    Lighting(const Content* content, Chunks* chunks);
	~Lighting();

    void clear();
    void buildSkyLight(int chunk_x, int chunk_z);
    void onChunkLoaded(int chunk_x, int chunk_z, bool expand);
    void onBlockSet(int x, int y, int z, blockid_t id);

    static void preBuildSkyLight(Chunk* chunk, const ContentIndices* indices);
};

#endif // LIGHTING_LIGHTING_H_
