#ifndef VOXELS_CHUNKSCONTROLLER_H_
#define VOXELS_CHUNKSCONTROLLER_H_

class Chunks;
class Lighting;
class WorldFiles;
class Level;

class ChunksController {
private:
	Chunks* chunks;
	Lighting* lighting;
	Level* level;
public:
	ChunksController(Level* level, Chunks* chunks, Lighting* lighting);
	~ChunksController();

	bool loadVisible(WorldFiles* worldFiles);
};

#endif // VOXELS_CHUNKSCONTROLLER_H_
