#ifndef VOXELS_CHUNKSCONTROLLER_H_
#define VOXELS_CHUNKSCONTROLLER_H_

#include "../typedefs.h"

class Level;
class Chunks;
class Lighting;
class WorldFiles;
class VoxelRenderer;
class ChunksLoader;
class WorldGenerator;

class ChunksController {
private:
	Level* level;
	Chunks* chunks;
	Lighting* lighting;
    WorldGenerator* generator;

    uint chunksPadding;

    int64_t avgDurationMcs = 1000;

	blockid_t bedrockID = 0;
public:
	ChunksController(Level* level, Chunks* chunks, Lighting* lighting, uint chunksPadding);
	~ChunksController();

    void update(int64_t maxDuration);
	bool loadVisible();
};

#endif /* VOXELS_CHUNKSCONTROLLER_H_ */
