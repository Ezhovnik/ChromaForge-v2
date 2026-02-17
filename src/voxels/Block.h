#ifndef VOXELS_BLOCK_H_
#define VOXELS_BLOCK_H_

#include <string>

#include "../math/AABB.h"
#include "../typedefs.h"

enum class BlockModel {
    None, // Невидимый блок
    Cube, // Полноценный блок
    X, // Крест-накрест (трава, растения)
    AABB // Форма, повторяющая хитбокс
};

#define FACE_MX 0
#define FACE_PX 1
#define FACE_MY 2
#define FACE_PY 3
#define FACE_MZ 4
#define FACE_PZ 5

#define BLOCK_AABB_GRID 16

class Block {
public:
    static Block* blocks[256];

    std::string const name;
    std::string textureFaces[6]; // -x, +x, -y, +y, -z, +z
    ubyte emission[4];
    ubyte drawGroup = 0;
    BlockModel model = BlockModel::Cube;

    bool lightPassing = false;
    bool skyLightPassing = false;
    bool obstacle = true;
    bool selectable = true;
    bool breakable = true;
    bool rotatable = false;
    bool replaceable = false;

    AABB hitbox;

	struct {
        blockid_t id;
		bool solid = true;
		bool emissive = false;
		bool hitboxGrid[BLOCK_AABB_GRID][BLOCK_AABB_GRID][BLOCK_AABB_GRID];
	} rt;

    Block(std::string name, std::string texture = "");
};

#endif // VOXELS_BLOCK_H_
