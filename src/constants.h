#ifndef SRC_CONSTANTS_H_
#define SRC_CONSTANTS_H_

#include <limits>

#include "typedefs.h"

inline constexpr int ENGINE_VERSION_MAJOR = 0;
inline constexpr int ENGINE_VERSION_MINOR = 1;
inline constexpr int ENGINE_VERSION_MAINTENANCE = 0;

inline constexpr int CHUNK_WIDTH = 16;
inline constexpr int CHUNK_HEIGHT = 256;
inline constexpr int CHUNK_DEPTH = 16;
inline constexpr int CHUNK_VOLUME = CHUNK_WIDTH * CHUNK_HEIGHT * CHUNK_DEPTH;

inline constexpr blockid_t BLOCK_VOID = std::numeric_limits<blockid_t>::max();

inline uint vox_index(int x, int y, int z, int w = CHUNK_WIDTH, int d = CHUNK_DEPTH) {
	return (y * d + z) * w + x;
}

#define SHADERS_FOLDER "shaders"
#define TEXTURES_FOLDER "textures"
#define FONTS_FOLDER "fonts"

#endif // SRC_CONSTANTS_H_
