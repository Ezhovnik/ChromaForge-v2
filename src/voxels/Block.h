#ifndef VOXELS_BLOCK_H_
#define VOXELS_BLOCK_H_

#include <array>
#include <memory>

#include "../typedefs.h"

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
