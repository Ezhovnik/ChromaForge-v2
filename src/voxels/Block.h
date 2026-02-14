#ifndef VOXELS_BLOCK_H_
#define VOXELS_BLOCK_H_

#include <string>

#include "../typedefs.h"

enum class BlockModel {
    None, Cube, X
};

#define FACE_MX 0
#define FACE_PX 1
#define FACE_MY 2
#define FACE_PY 3
#define FACE_MZ 4
#define FACE_PZ 5

class Block {
public:
    static Block* blocks[256];

    std::string const name;
	blockid_t id;
    std::string textureFaces[6]; // -x, +x, -y, +y, -z, +z
    ubyte emission[3];
    ubyte drawGroup = 0;
    BlockModel model = BlockModel::Cube;

    bool lightPassing = false;
    bool skyLightPassing = false;
    bool obstacle = true;
    bool selectable = true;
    bool breakable = true;
    bool rotatable = false;

    float hitboxScale = 1;

    Block(std::string name, std::string texture = "");
};

#endif // VOXELS_BLOCK_H_
