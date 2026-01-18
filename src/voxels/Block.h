#ifndef VOXELS_BLOCK_H_
#define VOXELS_BLOCK_H_

#include <array>
#include <memory>

#include "../typedefs.h"

#define FACE_MX 0
#define FACE_PX 1
#define FACE_MY 2
#define FACE_PY 3
#define FACE_MZ 4
#define FACE_PZ 5

namespace Block_models {
    inline constexpr int AIR = 0;
    inline constexpr int CUBE = 1;
    inline constexpr int X = 2;
}

class Block {
public:
    static std::array<std::unique_ptr<Block>, 256> blocks;

    const uint id;

    int textureFaces[6]; // -x, +x, -y, +y, -z, +z
    ubyte emission[3];
    ubyte drawGroup = 0;
    ubyte model = Block_models::CUBE;

    bool lightPassing = false;
    bool skyLightPassing = false;
    bool obstacle = true;
    bool selectable = true;
    bool breakable = true;
    bool rotatable = false;

    float hitboxScale = 1;
	float hitboxY = 1;

    Block(uint id, int texture);
};

#endif // VOXELS_BLOCK_H_
