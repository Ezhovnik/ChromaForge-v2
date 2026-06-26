#pragma once

#include <string>
#include <vector>
#include <optional>
#include <array>

#include <math/AABB.h>
#include <typedefs.h>
#include <math/UVRegion.h>
#include <core_content_defs.h>
#include <data/dv.h>

struct ParticlesPreset;
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

enum class CullingMode {
    Default,
    Optional,
    Disabled,
};

std::string to_string(CullingMode mode);
std::optional<CullingMode> CullingMode_from(std::string_view str);

inline constexpr int FACE_MX = 0;
inline constexpr int FACE_PX = 1;
inline constexpr int FACE_MY = 2;
inline constexpr int FACE_PY = 3;
inline constexpr int FACE_MZ = 4;
inline constexpr int FACE_PZ = 5;

inline constexpr int BLOCK_AABB_GRID = 16;

inline constexpr size_t MAX_USER_BLOCK_FIELDS_SIZE = 240;

struct BlockFuncsSet {
	bool init : 1;
	bool update : 1;
    bool onplaced : 1;
    bool onbreaking : 1;
    bool onbroken : 1;
    bool onreplaced : 1;
    bool oninteract : 1;
    bool randupdate : 1;
    bool onblocksspark : 1;
};

struct CoordSystem {
	std::array<glm::ivec3, 3> axes;
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
    int variantsCount;

	static const BlockRotProfile PIPE;
    static const BlockRotProfile PANE;
    static const BlockRotProfile NONE;

    static inline const std::string NONE_NAME = "none";
    static inline const std::string PIPE_NAME = "pipe";
    static inline const std::string PANE_NAME = "pane";
};

struct BlockMaterial {
	std::string name;
	std::string stepsSound;
    std::string placeSound;
    std::string breakSound;
    std::string hitSound;

    dv::value serialize() const;
};

class Block {
public:
    std::string const name;

    std::string caption;

    std::array<std::string, 6> textureFaces; // -x, +x, -y, +y, -z, +z

    dv::value properties = nullptr;

    std::string material = DEFAULT_MATERIAL;

    ubyte emission[4] {0, 0, 0, 0};
    glm::i8vec3 size {1, 1, 1};
    ubyte drawGroup = 0;
    BlockModel model = BlockModel::Cube;
    dv::value customModelRaw = nullptr;
    std::string modelName = "";
    CullingMode culling = CullingMode::Default;
    std::string pickingItem = name + BLOCK_ITEM_SUFFIX;
    std::string scriptName = name.substr(name.find(':') + 1);
    std::string scriptFile;
    uint inventorySize = 0;
    std::string surfaceReplacement = name;
    std::string overlayTexture;
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
    bool translucent = false; // TODO

    std::vector<AABB> hitboxes {AABB()};

    BlockRotProfile rotations = BlockRotProfile::NONE;

    std::unique_ptr<data::StructLayout> dataStruct;

    std::unique_ptr<ParticlesPreset> particles;

	struct {
        blockid_t id;
		bool solid = true;
		bool emissive = false;
        bool extended = false;
		std::vector<AABB> hitboxes[BlockRotProfile::MAX_COUNT];
        BlockFuncsSet funcsset {};
        itemid_t pickingItem = 0;
        blockid_t surfaceReplacement = 0;
	} rt {};

    Block(const std::string& name);
    Block(std::string name, const std::string& texture);
    Block(const Block&) = delete;
    ~Block();

    void cloneTo(Block& dst);

    static bool isReservedBlockField(std::string_view view);
};

inline glm::ivec3 get_ground_direction(const Block& def, int rotation) {
    return -def.rotations.variants[rotation].axes[1];
}
