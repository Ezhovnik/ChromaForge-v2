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

inline constexpr uint VOXEL_USER_BITS = 8;
inline constexpr uint VOXEL_USER_BITS_OFFSET = sizeof(blockstate_t) * 8 - VOXEL_USER_BITS;

inline constexpr blockid_t BLOCK_VOID = std::numeric_limits<blockid_t>::max();
inline constexpr itemid_t ITEM_VOID = std::numeric_limits<itemid_t>::max();
inline constexpr blockid_t BLOCK_AIR = 0;
inline constexpr itemid_t ITEM_EMPTY = 0;

inline constexpr int ITEM_ICON_SIZE = 48;

inline constexpr double PI = 3.14159265358979323846;

inline constexpr uint vox_index(uint x, uint y, uint z, uint w = CHUNK_WIDTH, uint d = CHUNK_DEPTH) {
	return (y * d + z) * w + x;
}

#define SHADERS_FOLDER "shaders"
#define TEXTURES_FOLDER "textures"
#define FONTS_FOLDER "fonts"

#endif // SRC_CONSTANTS_H_
