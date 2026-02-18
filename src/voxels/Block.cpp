#include "Block.h"

#include "../core_defs.h"

void CoordSystem::transform(AABB& aabb) {
	glm::vec3 X(axisX);
	glm::vec3 Y(axisY);
	glm::vec3 Z(axisZ);
	aabb.a = X * aabb.a.x + Y * aabb.a.y + Z * aabb.a.z;
	aabb.b = X * aabb.b.x + Y * aabb.b.y + Z * aabb.b.z;
	aabb.a += fix2;
	aabb.b += fix2;
}

const BlockRotProfile BlockRotProfile::PIPE {"pipe", {
		{ { 1, 0, 0 }, { 0, 0, 1 }, { 0,-1, 0 }, { 0, 0,-1 }, { 0, 1, 0 } }, // North
		{ { 0, 1, 0 }, {-1, 0, 0 }, { 0, 0, 1 }, { 1, 0, 0 }, { 1, 0, 0 } }, // East
		{ { 1, 0, 0 }, { 0, 0,-1 }, { 0, 1, 0 }, { 0, 1, 0 }, { 0, 0, 1 } }, // South
		{ { 0,-1, 0 }, { 1, 0, 0 }, { 0, 0, 1 }, { 0, 1, 0 }, { 0, 1, 0 } }, // West
		{ { 1, 0, 0 }, { 0, 1, 0 }, { 0, 0, 1 }, { 0, 0, 0 }, { 0, 0, 0 } }, // Up
		{ { 1, 0, 0 }, { 0,-1, 0 }, { 0, 0,-1 }, { 0, 1,-1 }, { 0, 1, 1 } }, // Down
}};

const BlockRotProfile BlockRotProfile::PANE {"pane", {
		{ { 1, 0, 0 }, { 0, 1, 0 }, { 0, 0, 1 }, { 0, 0, 0 }, { 0, 0, 0 } }, // North
		{ { 0, 0,-1 }, { 0, 1, 0 }, { 1, 0, 0 }, { 1, 0, 0 }, { 0, 0, 1 } }, // East
		{ {-1, 0, 0 }, { 0, 1, 0 }, { 0, 0,-1 }, { 1, 0,-1 }, { 1, 0, 1 } }, // South
		{ { 0, 0, 1 }, { 0, 1, 0 }, {-1, 0, 0 }, { 0, 0,-1 }, { 1, 0, 0 } }, // West
}};

Block* Block::blocks[256];

Block::Block(std::string name) : name(name), textureFaces{TEXTURE_NOTFOUND, TEXTURE_NOTFOUND, TEXTURE_NOTFOUND, TEXTURE_NOTFOUND, TEXTURE_NOTFOUND, TEXTURE_NOTFOUND} {
    rotations = BlockRotProfile::PIPE;
}

Block::Block(std::string name, std::string texture) : name(name), textureFaces{texture, texture, texture, texture, texture, texture}, emission{0, 0, 0}{
    rotations = BlockRotProfile::PIPE;
}
