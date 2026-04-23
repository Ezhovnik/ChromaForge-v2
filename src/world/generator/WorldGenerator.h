#pragma once

#include <string>

#include <typedefs.h>

#include <core_content_defs.h>

struct voxel;
class PseudoRandom;
class Map2D;
class fnl_state;
class Content;
struct Generator;

// Класс для генерации воксельного мира
class WorldGenerator {
	const Generator& def;
    const Content* content;
public:
	WorldGenerator(
        const Generator& def,
        const Content* content
    );

	virtual void generate(voxel* voxels, int x, int z, int seed);

    inline static std::string DEFAULT = BUILTIN_CONTENT_NAMESPACE + ":default";
};
