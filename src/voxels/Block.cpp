#include "Block.h"

void CoordSystem::transform(AABB& aabb) {
	glm::vec3 X(axisX);
	glm::vec3 Y(axisY);
	glm::vec3 Z(axisZ);
	aabb.a = X * aabb.a.x + Y * aabb.a.y + Z * aabb.a.z;
	aabb.b = X * aabb.b.x + Y * aabb.b.y + Z * aabb.b.z;
	aabb.a += fix2;
	aabb.b += fix2;
}

const BlockRotProfile BlockRotProfile::PIPE {{
		// Vertical
		{{1, 0, 0}, {0, 1, 0}, {0, 0, 1}, {0, 0, 0}, {0, 0, 0}},
		// X-Aligned
		{{0, -1, 0}, {1, 0, 0}, {0, 0, 1}, {0, 1, 0}, {0, 1, 0}},
		// Z-Aligned
		{{1, 0, 0}, {0, 0, 1}, {0, -1, 0}, {0, 0, -1}, {0, 1, 0}},
}};

Block* Block::blocks[256];

Block::Block(std::string name) : name(name), textureFaces{"notfound", "notfound", "notfound", "notfound", "notfound", "notfound"} {
    rotations = BlockRotProfile::PIPE;
}

Block::Block(std::string name, std::string texture) : name(name), textureFaces{texture, texture, texture, texture, texture, texture}, emission{0, 0, 0}{
    rotations = BlockRotProfile::PIPE;
}
