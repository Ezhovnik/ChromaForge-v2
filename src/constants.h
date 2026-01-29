#ifndef SRC_CONSTANTS_H_
#define SRC_CONSTANTS_H_

#include <limits.h>

#include "typedefs.h"

inline constexpr int CHUNK_WIDTH = 16;
inline constexpr int CHUNK_HEIGHT = 256;
inline constexpr int CHUNK_DEPTH = 16;
inline constexpr int CHUNK_VOLUME = CHUNK_WIDTH * CHUNK_HEIGHT * CHUNK_DEPTH;

inline constexpr blockid_t BLOCK_VOID = (blockid_t)(2 << (sizeof(blockid_t) * CHAR_BIT)) - 1;

inline uint vox_index(int x, int y, int z, int w, int d) {
	return (y * d + z) * w + x;
}

inline constexpr int ATLAS_MARGIN_SIZE = 2;

#define RES_FOLDER "../res/"
#define TEXTURES_FOLDER "../res/textures"
#define SHADERS_FOLDER "../res/shaders"
#define FONTS_FOLDER "../res/fonts"

#endif // SRC_CONSTANTS_H_
