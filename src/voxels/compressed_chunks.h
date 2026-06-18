#pragma once

#include <typedefs.h>

#include <vector>

class Chunk;

namespace compressed_chunks {
    std::vector<ubyte> encode(const Chunk& chunk);
    void decode(Chunk& chunk, const ubyte* src, size_t size);
}
