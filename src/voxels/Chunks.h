#ifndef VOXELS_CHUNKS_H_
#define VOXELS_CHUNKS_H_

#include <stdlib.h>

typedef unsigned int uint;

class Chunk;

class Chunks{
public:
    Chunk** chunks;
    size_t volume;
    uint width, height, depth;

    Chunks(uint width, uint height, uint depth);
    ~Chunks();
};

#endif // VOXELS_CHUNKS_H_
