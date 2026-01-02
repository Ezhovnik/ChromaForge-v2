#ifndef LIGHTING_LIGHTING_H_
#define LIGHTING_LIGHTING_H_

class Chunks;
class LightSolver;

class Lighting {
    Chunks* chunks = nullptr;
    LightSolver* solverR = nullptr;
    LightSolver* solverG = nullptr;
    LightSolver* solverB = nullptr;
    LightSolver* solverS = nullptr;
public:
    Lighting(Chunks* chunks);
	~Lighting();

    void clear();
    void onChunkLoaded(int chunk_x, int chunk_y, int chunk_z, bool sky);
    void onBlockSet(int x, int y, int z, int id);
};

#endif
