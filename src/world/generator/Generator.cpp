#include <world/generator/Generator.h>

#include <world/generator/VoxelFragment.h>
#include <content/Content.h>
#include <util/stringutil.h>
#include <voxels/Block.h>
#include <debug/Logger.h>

VoxelStructure::VoxelStructure(
    VoxelStructureMeta meta,
    std::unique_ptr<VoxelFragment> structure
) : meta(std::move(meta)),
    fragments({std::move(structure)}) {}


Generator::Generator(std::string name) : name(std::move(name)), caption(util::id_to_caption(name)) {}

void Generator::prepare(const Content* content) {
    for (auto& biome : biomes) {
        for (auto& layer : biome.groundLayers.layers) {
            layer.rt.id = content->blocks.require(layer.block).rt.id;
        }
        for (auto& layer : biome.seaLayers.layers) {
            layer.rt.id = content->blocks.require(layer.block).rt.id;
        }
        for (auto& plant : biome.plants.entries) {
            plant.rt.id = content->blocks.require(plant.name).rt.id;
        }
        for (auto& structure : biome.structures.entries) {
            const auto& found = structuresIndices.find(structure.name);
            if (found == structuresIndices.end()) {
                THROW_ERR("No structure {} found", util::quote(structure.name));
            }
            structure.rt.id = found->second;
        }
    }
}
