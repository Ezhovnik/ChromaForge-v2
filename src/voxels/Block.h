#pragma once

#include <string>
#include <vector>
#include <array>

#include <math/AABB.h>
#include <typedefs.h>
#include <math/UVRegion.h>
#include <core_content_defs.h>
#include <data/dv.h>
#include <util/EnumMetadata.h>
#include <interfaces/Serializable.h>
#include <util/stack_vector.h>

struct ParticlesPreset;
namespace data {
    class StructLayout;
}

inline const std::string BLOCK_ITEM_SUFFIX = ".item"; 

inline std::string DEFAULT_MATERIAL = CHROMAFORGE_CONTENT_NAMESPACE + ":stone";

enum class BlockModelType {
    None, // Невидимый блок
    Cube, // Полноценный блок
    X, // Крест-накрест (трава, растения)
    AABB, // Форма, повторяющая хитбокс
    Custom
};

struct BlockModel {
    BlockModelType type = BlockModelType::Cube;
    dv::value customRaw = nullptr;
    std::string name = "";
};

CHROMA_ENUM_METADATA(BlockModelType)
    {"none", BlockModelType::None},
    {"cube", BlockModelType::Cube},
    {"X", BlockModelType::X},
    {"aabb", BlockModelType::AABB},
    {"custom", BlockModelType::Custom},
CHROMA_ENUM_END

enum class CullingMode {
    Default,
    Optional,
    Disabled,
};

CHROMA_ENUM_METADATA(CullingMode)
    {"default", CullingMode::Default},
    {"optional", CullingMode::Optional},
    {"disabled", CullingMode::Disabled},
CHROMA_ENUM_END

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

inline constexpr int BLOCK_MAX_VARIANTS = 16;

struct Variant {
    BlockModel model {};
    std::array<std::string, 6> textureFaces;  // -x, x, -y, y, -z, z
    CullingMode culling = CullingMode::Default;
    uint8_t drawGroup = 0;

    struct {
        bool solid = true;
    } rt;
};

struct Variants {
    uint8_t offset;
    uint8_t mask;

    /// First variant is copy of Block::defaults
    util::stack_vector<Variant, BLOCK_MAX_VARIANTS> variants {};
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
    static const BlockRotProfile STAIRS;
    static const BlockRotProfile NONE;

    static inline const std::string NONE_NAME = "none";
    static inline const std::string PIPE_NAME = "pipe";
    static inline const std::string PANE_NAME = "pane";
    static inline std::string STAIRS_NAME = "stairs";
};

struct BlockMaterial : Serializable {
	std::string name;
	std::string stepsSound;
    std::string placeSound;
    std::string breakSound;
    std::string hitSound;

    dv::value toTable() const;
    dv::value serialize() const override;
    void deserialize(const dv::value& src) override;
};

class Block {
public:
    std::string const name;

    std::string caption;

    Variant defaults {};

    dv::value properties = nullptr;

    std::string material = DEFAULT_MATERIAL;

    ubyte emission[4] {0, 0, 0, 0};
    glm::i8vec3 size {1, 1, 1};
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

    std::unique_ptr<Variants> variants;

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

    uint8_t getVariantIndex(uint8_t userbits) const {
        if (variants == nullptr) return 0;
        return (userbits >> variants->offset) & variants->mask;
    }

    const Variant& getVariantByBits(uint8_t userbits) const {
        if (userbits == 0 || variants == nullptr) return defaults;
        return variants->variants[
            (userbits >> variants->offset) & variants->mask
        ];
    }

    const Variant& getVariant(uint8_t index) const {
        if (index == 0) return defaults;
        return variants->variants[index];
    }

    const BlockModel& getModel(uint8_t bits) const {
        return getVariantByBits(bits).model;
    }

    static bool isReservedBlockField(std::string_view view);
};

inline glm::ivec3 get_ground_direction(const Block& def, int rotation) {
    return -def.rotations.variants[rotation].axes[1];
}
