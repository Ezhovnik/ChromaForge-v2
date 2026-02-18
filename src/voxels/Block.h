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

constexpr int FACE_MX = 0;
constexpr int FACE_PX = 1;
constexpr int FACE_MY = 2;
constexpr int FACE_PY = 3;
constexpr int FACE_MZ = 4;
constexpr int FACE_PZ = 5;

constexpr int BLOCK_AABB_GRID = 16;

struct CoordSystem {
	glm::ivec3 axisX;
	glm::ivec3 axisY;
	glm::ivec3 axisZ;
	glm::ivec3 fix;
    glm::ivec3 fix2;

	void transform(AABB& aabb);
};

struct BlockRotProfile {
	static const int MAX_COUNT = 16;
    std::string name;
	CoordSystem variants[MAX_COUNT];

	static const BlockRotProfile PIPE;
    static const BlockRotProfile PANE;
};

class Block {
public:
    static Block* blocks[256];

    std::string const name;
    std::string textureFaces[6]; // -x, +x, -y, +y, -z, +z
    ubyte emission[4] {0, 0, 0, 0};
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

    BlockRotProfile rotations;

	struct {
        blockid_t id;
		bool solid = true;
		bool emissive = false;
		AABB hitboxes[BlockRotProfile::MAX_COUNT];
	} rt;

    Block(std::string name);
    Block(std::string name, std::string texture);
};

#endif // VOXELS_BLOCK_H_
