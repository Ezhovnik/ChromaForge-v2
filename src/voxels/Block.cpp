#include <voxels/Block.h>

#include <utility>
#include <set>

#include <core_content_defs.h>
#include <util/stringutil.h>
#include <data/StructLayout.h>

std::string to_string(BlockModel model) {
    switch (model) {
        case BlockModel::None: return "none";
        case BlockModel::Cube: return "cube";
        case BlockModel::X: return "X";
        case BlockModel::AABB: return "aabb";
        case BlockModel::Custom: return "custom";
		default: return "unknown";
    }
}

std::optional<BlockModel> BlockModel_from(std::string_view str) {
    if (str == "none") {
        return BlockModel::None;
    } else if (str == "cube") {
        return BlockModel::Cube;
    } else if (str == "X") {
        return BlockModel::X;
    } else if (str == "aabb") {
        return BlockModel::AABB;
    } else if (str == "custom") {
        return BlockModel::Custom;
    }
    return std::nullopt;
}

CoordSystem::CoordSystem(glm::ivec3 axisX, glm::ivec3 axisY, glm::ivec3 axisZ) : axisX(axisX), axisY(axisY), axisZ(axisZ) {
	fix = glm::ivec3(0);
	if (isVectorHasNegatives(axisX)) fix -= axisX;
	if (isVectorHasNegatives(axisY)) fix -= axisY;
	if (isVectorHasNegatives(axisZ)) fix -= axisZ;
}

void CoordSystem::transform(AABB& aabb) const {
	glm::vec3 X(axisX);
	glm::vec3 Y(axisY);
	glm::vec3 Z(axisZ);
	aabb.a = X * aabb.a.x + Y * aabb.a.y + Z * aabb.a.z;
	aabb.b = X * aabb.b.x + Y * aabb.b.y + Z * aabb.b.z;
	aabb.a += fix;
	aabb.b += fix;
}

const BlockRotProfile BlockRotProfile::NONE {"none", {
        {{1, 0, 0}, {0, 1, 0}, {0, 0, 1}}, // North
        {{1, 0, 0}, {0, 1, 0}, {0, 0, 1}}, // East
        {{1, 0, 0}, {0, 1, 0}, {0, 0, 1}}, // South
        {{1, 0, 0}, {0, 1, 0}, {0, 0, 1}}, // West
        {{1, 0, 0}, {0, 1, 0}, {0, 0, 1}}, // Up
        {{1, 0, 0}, {0, 1, 0}, {0, 0, 1}}, // Down
	}
};

const BlockRotProfile BlockRotProfile::PIPE {"pipe", {
		{{ 1, 0, 0 }, { 0, 0, 1 }, { 0, -1, 0 }}, // North
		{{ 0, 0, 1 }, {-1, 0, 0 }, { 0, -1, 0 }}, // East
		{{ -1, 0, 0 }, { 0, 0,-1 }, { 0, -1, 0 }}, // South
		{{ 0, 0, -1 }, { 1, 0, 0 }, { 0, -1, 0 }}, // West
		{{ 1, 0, 0 }, { 0, 1, 0 }, { 0, 0, 1 }}, // Up
		{{ 1, 0, 0 }, { 0,-1, 0 }, { 0, 0,-1 }}, // Down
	}
};

const BlockRotProfile BlockRotProfile::PANE {"pane", {
		{{ 1, 0, 0 }, { 0, 1, 0 }, { 0, 0, 1 }}, // North
		{{ 0, 0,-1 }, { 0, 1, 0 }, { 1, 0, 0 }}, // East
		{{-1, 0, 0 }, { 0, 1, 0 }, { 0, 0,-1 }}, // South
		{{ 0, 0, 1 }, { 0, 1, 0 }, {-1, 0, 0 }}, // West
	}
};

Block::Block(const std::string& name) : 
	name(name), 
	textureFaces{TEXTURE_NOTFOUND, TEXTURE_NOTFOUND, TEXTURE_NOTFOUND, TEXTURE_NOTFOUND, TEXTURE_NOTFOUND, TEXTURE_NOTFOUND},
	caption(util::id_to_caption(name))
{}

Block::~Block() {}

Block::Block(
	std::string name, 
	const std::string& texture
) : name(std::move(name)), 
	textureFaces{texture, texture, texture, texture, texture, texture}, 
	caption(util::id_to_caption(name))
{}

void Block::cloneTo(Block& dst) {
    dst.caption = caption;

    for (int i = 0; i < 6; ++i) {
        dst.textureFaces[i] = textureFaces[i];
    }

    dst.modelTextures = modelTextures;
    dst.modelBoxes = modelBoxes;
    dst.modelExtraPoints = modelExtraPoints;
    dst.modelUVs = modelUVs;
    dst.material = material;
    std::copy(&emission[0], &emission[3], dst.emission);
    dst.size = size;
    dst.model = model;
    dst.lightPassing = lightPassing;
    dst.skyLightPassing = skyLightPassing;
    dst.shadeless = shadeless;
    dst.ambientOcclusion = ambientOcclusion;
    dst.obstacle = obstacle;
    dst.selectable = selectable;
    dst.replaceable = replaceable;
    dst.breakable = breakable;
    dst.rotatable = rotatable;
    dst.grounded = grounded;
    dst.hidden = hidden;
    dst.hitboxes = hitboxes;
    dst.rotations = rotations;
    dst.pickingItem = pickingItem;
    dst.scriptName = scriptName;
    dst.uiLayout = uiLayout;
    dst.inventorySize = inventorySize;
    dst.sparkInterval = sparkInterval;
}

static std::set<std::string, std::less<>> RESERVED_BLOCK_FIELDS {
};

bool Block::isReservedBlockField(std::string_view view) {
    return RESERVED_BLOCK_FIELDS.find(view) != RESERVED_BLOCK_FIELDS.end();
}
