#ifndef SRC_CONSTANTS_H_
#define SRC_CONSTANTS_H_

#include "typedefs.h"
#include <climits>

inline constexpr int CHUNK_WIDTH = 16;
inline constexpr int CHUNK_HEIGHT = 256;
inline constexpr int CHUNK_DEPTH = 16;
inline constexpr int CHUNK_VOLUME = CHUNK_WIDTH * CHUNK_HEIGHT * CHUNK_DEPTH;

inline constexpr blockid_t BLOCK_VOID = (blockid_t)(2 << (sizeof(blockid_t) * CHAR_BIT)) - 1;

inline uint vox_index(int x, int y, int z, int width, int depth) {
	return (y * depth + z) * width + x;
}

#endif // SRC_CONSTANTS_H_
