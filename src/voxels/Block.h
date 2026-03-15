#ifndef VOXELS_BLOCK_H_
#define VOXELS_BLOCK_H_

#include <string>
#include <vector>

#include "../math/AABB.h"
#include "../typedefs.h"
#include "../math/UVRegion.h"
#include "../core_content_defs.h"

#define BLOCK_ITEM_SUFFIX ".item"

inline std::string DEFAULT_MATERIAL = CHROMAFORGE_CONTENT_NAMESPACE + ":stone";

enum class BlockModel {
    None, // Невидимый блок
    Cube, // Полноценный блок
    X, // Крест-накрест (трава, растения)
    AABB, // Форма, повторяющая хитбокс
    Custom
};

inline constexpr int FACE_MX = 0;
inline constexpr int FACE_PX = 1;
inline constexpr int FACE_MY = 2;
inline constexpr int FACE_PY = 3;
inline constexpr int FACE_MZ = 4;
inline constexpr int FACE_PZ = 5;

inline constexpr int BLOCK_AABB_GRID = 16;

struct block_funcs_set {
	bool init = false;
	bool update = false;
    bool onplaced = false;
    bool onbroken = false;
    bool oninteract = false;
    bool randupdate = false;
    bool onblocksspark = false;
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
		return (vec.x < 0 || vec.y < 0 || vec.z < 0);
	}
};

struct BlockRotProfile {
	static const int MAX_COUNT = 8;
    std::string name;
	CoordSystem variants[MAX_COUNT];

	static const BlockRotProfile PIPE;
    static const BlockRotProfile PANE;
};

struct BlockMaterial {
	std::string name;
	std::string stepsSound {""};
	std::string placeSound {""};
	std::string breakSound {""};
};

class Block {
public:
    std::string const name;

    std::string caption;

    std::string textureFaces[6]; // -x, +x, -y, +y, -z, +z
    std::vector<std::string> modelTextures = {};
	std::vector<AABB> modelBoxes = {};
	std::vector<glm::vec3> modelExtraPoints = {};
	std::vector<UVRegion> modelUVs = {};
    std::string material = DEFAULT_MATERIAL;

    ubyte emission[4] {0, 0, 0, 0};
    ubyte drawGroup = 0;
    BlockModel model = BlockModel::Cube;
    std::string pickingItem = name + BLOCK_ITEM_SUFFIX;
    std::string scriptName = name.substr(name.find(':') + 1);
    uint inventorySize = 0;
    std::string uiLayout = name;

    bool lightPassing = false;
    bool skyLightPassing = false;
    bool obstacle = true;
    bool selectable = true;
    bool breakable = true;
    bool rotatable = false;
    bool replaceable = false;
    bool grounded = false;
    bool hidden = false;

    std::vector<AABB> hitboxes;

    BlockRotProfile rotations;

	struct {
        blockid_t id;
		bool solid = true;
		bool emissive = false;
		std::vector<AABB> hitboxes[BlockRotProfile::MAX_COUNT];
        block_funcs_set funcsset {};
        itemid_t pickingItem = 0;
	} rt;

    Block(std::string name);
    Block(std::string name, std::string texture);
    Block(const Block&) = delete;
};

#endif // VOXELS_BLOCK_H_
