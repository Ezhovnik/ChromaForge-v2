#include "Chunks.h"

#include "Chunk.h"

Chunks::Chunks(uint width, uint height, uint depth) : width(width), height(height), depth(depth) {
    volume = width * height * depth;
    chunks = new Chunk*[volume];

    int index = 0;
    for (int y = 0; y < height; ++y) {
        for (int z = 0; z < depth; ++z) {
            for (int x = 0; x < width; ++x) {
                Chunk* chunk = new Chunk(x, y, z);
                chunks[index] = chunk;
                index++;
            }
        }
    }
}

Chunks::~Chunks() {
    for (int i = 0; i < volume; ++i) {
        delete chunks[i];
    }
    delete[] chunks;
}
