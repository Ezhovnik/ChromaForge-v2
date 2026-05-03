#include <world/generator/Generator.h>

#include <world/generator/VoxelFragment.h>
#include <content/Content.h>
#include <util/stringutil.h>
#include <voxels/Block.h>
#include <debug/Logger.h>

VoxelStructure::VoxelStructure(
    VoxelStructureMeta meta,
    std::unique_ptr<VoxelFragment> structure
) : fragments({std::move(structure)}),
    meta(std::move(meta)) {}


Generator::Generator(std::string name) : name(std::move(name)) {}

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
                LOG_ERROR("No structure {} found", util::quote(structure.name));
                throw std::runtime_error(
                    "No structure " + util::quote(structure.name) + " found"
                );
            }
            structure.rt.id = found->second;
        }
    }
}
