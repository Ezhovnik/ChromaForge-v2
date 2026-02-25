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

struct block_funcs_set {
	bool init = false;
	bool update = false;
    bool onplaced = false;
    bool onbroken = false;
    bool oninteract = false;
    bool randupdate = false;
};

struct CoordSystem {
	glm::ivec3 axisX;
	glm::ivec3 axisY;
	glm::ivec3 axisZ;
	glm::ivec3 fix;

    CoordSystem() = default;
	CoordSystem(glm::ivec3 axisX, glm::ivec3 axisY, glm::ivec3 axisZ);

	void transform(AABB& aabb) const;

    static bool isVectorHasNegatives(glm::ivec3 vec) {
		if (vec.x < 0 || vec.y < 0 || vec.z < 0) return true;
		else return false;
	}
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
    bool grounded = false;

    AABB hitbox;

    BlockRotProfile rotations;

	struct {
        blockid_t id;
		bool solid = true;
		bool emissive = false;
		AABB hitboxes[BlockRotProfile::MAX_COUNT];
        block_funcs_set funcsset {};
	} rt;

    Block(std::string name);
    Block(std::string name, std::string texture);
};

#endif // VOXELS_BLOCK_H_
