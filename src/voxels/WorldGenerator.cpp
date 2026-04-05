#include "WorldGenerator.h"

#include "../content/Content.h"
#include "../core_content_defs.h"
#include "Block.h"
#include "voxel.h"

WorldGenerator::WorldGenerator(const Content* content)
                : idStone(content->blocks.require(CHROMAFORGE_CONTENT_NAMESPACE + ":stone").rt.id),
                idDirt(content->blocks.require(CHROMAFORGE_CONTENT_NAMESPACE + ":dirt").rt.id),
				idMoss(content->blocks.require(CHROMAFORGE_CONTENT_NAMESPACE + ":moss").rt.id),
				idSand(content->blocks.require(CHROMAFORGE_CONTENT_NAMESPACE + ":sand").rt.id),
				idWater(content->blocks.require(CHROMAFORGE_CONTENT_NAMESPACE + ":water").rt.id),
				idLog(content->blocks.require(CHROMAFORGE_CONTENT_NAMESPACE + ":oak_log").rt.id),
				idLeaves(content->blocks.require(CHROMAFORGE_CONTENT_NAMESPACE + ":leaves").rt.id),
				idGrass(content->blocks.require(CHROMAFORGE_CONTENT_NAMESPACE + ":grass").rt.id),
				idPoppy(content->blocks.require(CHROMAFORGE_CONTENT_NAMESPACE + ":poppy").rt.id),
				idDandelion(content->blocks.require(CHROMAFORGE_CONTENT_NAMESPACE + ":dandelion").rt.id),
				idDaisy(content->blocks.require(CHROMAFORGE_CONTENT_NAMESPACE + ":daisy").rt.id),
				idMarigold(content->blocks.require(CHROMAFORGE_CONTENT_NAMESPACE + ":marigold").rt.id),
				idBedrock(content->blocks.require(CHROMAFORGE_CONTENT_NAMESPACE + ":bedrock").rt.id) {;
}
