#pragma once

#include <string>
#include <vector>
#include <optional>

#include <math/AABB.h>
#include <typedefs.h>
#include <math/UVRegion.h>
#include <core_content_defs.h>

namespace data {
    class StructLayout;
}

inline const std::string BLOCK_ITEM_SUFFIX = ".item"; 

inline std::string DEFAULT_MATERIAL = CHROMAFORGE_CONTENT_NAMESPACE + ":stone";

enum class BlockModel {
    None, // Невидимый блок
    Cube, // Полноценный блок
    X, // Крест-накрест (трава, растения)
    AABB, // Форма, повторяющая хитбокс
    Custom
};

std::string to_string(BlockModel model);
std::optional<BlockModel> BlockModel_from(std::string_view str);

inline constexpr int FACE_MX = 0;
inline constexpr int FACE_PX = 1;
inline constexpr int FACE_MY = 2;
inline constexpr int FACE_PY = 3;
inline constexpr int FACE_MZ = 4;
inline constexpr int FACE_PZ = 5;

inline constexpr int BLOCK_AABB_GRID = 16;

inline constexpr size_t MAX_USER_BLOCK_FIELDS_SIZE = 240;

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
    static const BlockRotProfile NONE;

    static inline const std::string NONE_NAME = "none";
    static inline const std::string PIPE_NAME = "pipe";
    static inline const std::string PANE_NAME = "pane";
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
    glm::i8vec3 size {1, 1, 1};
    ubyte drawGroup = 0;
    BlockModel model = BlockModel::Cube;
    std::string pickingItem = name + BLOCK_ITEM_SUFFIX;
    std::string scriptName = name.substr(name.find(':') + 1);
    uint inventorySize = 0;
    std::string uiLayout = name;
    uint sparkInterval = 1;

    bool lightPassing = false;
    bool skyLightPassing = false;
    bool obstacle = true;
    bool selectable = true;
    bool breakable = true;
    bool rotatable = false;
    bool replaceable = false;
    bool grounded = false;
    bool hidden = false;
    bool shadeless = false;
    bool ambientOcclusion = true;

    std::vector<AABB> hitboxes;

    BlockRotProfile rotations = BlockRotProfile::NONE;

    std::unique_ptr<data::StructLayout> dataStruct;

	struct {
        blockid_t id;
		bool solid = true;
		bool emissive = false;
        bool extended = false;
		std::vector<AABB> hitboxes[BlockRotProfile::MAX_COUNT];
        block_funcs_set funcsset {};
        itemid_t pickingItem = 0;
	} rt {};

    Block(const std::string& name);
    Block(std::string name, const std::string& texture);
    Block(const Block&) = delete;
    ~Block();

    void cloneTo(Block& dst);

    static bool isReservedBlockField(std::string_view view);
};

inline glm::ivec3 get_ground_direction(const Block& def, int rotation) {
    return -def.rotations.variants[rotation].axisY;
}
