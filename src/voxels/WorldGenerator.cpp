#include "WorldGenerator.h"

#include "../content/Content.h"
#include "../core_defs.h"
#include "Block.h"
#include "voxel.h"

WorldGenerator::WorldGenerator(const Content* content)
                : idStone(content->requireBlock(CHROMAFORGE_CONTENT_NAMESPACE":stone").rt.id),
                idDirt(content->requireBlock(CHROMAFORGE_CONTENT_NAMESPACE":dirt").rt.id),
				idMoss(content->requireBlock(CHROMAFORGE_CONTENT_NAMESPACE":moss").rt.id),
				idSand(content->requireBlock(CHROMAFORGE_CONTENT_NAMESPACE":sand").rt.id),
				idWater(content->requireBlock(CHROMAFORGE_CONTENT_NAMESPACE":water").rt.id),
				idLog(content->requireBlock(CHROMAFORGE_CONTENT_NAMESPACE":oak_log").rt.id),
				idLeaves(content->requireBlock(CHROMAFORGE_CONTENT_NAMESPACE":leaves").rt.id),
				idGrass(content->requireBlock(CHROMAFORGE_CONTENT_NAMESPACE":grass").rt.id),
				idPoppy(content->requireBlock(CHROMAFORGE_CONTENT_NAMESPACE":poppy").rt.id),
				idDandelion(content->requireBlock(CHROMAFORGE_CONTENT_NAMESPACE":dandelion").rt.id),
				idDaisy(content->requireBlock(CHROMAFORGE_CONTENT_NAMESPACE":daisy").rt.id),
				idMarigold(content->requireBlock(CHROMAFORGE_CONTENT_NAMESPACE":marigold").rt.id),
				idBedrock(content->requireBlock(CHROMAFORGE_CONTENT_NAMESPACE":bedrock").rt.id) {;
}
