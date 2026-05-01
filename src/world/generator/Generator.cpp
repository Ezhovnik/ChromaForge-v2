#include <world/generator/Generator.h>

#include <world/generator/VoxelFragment.h>

GeneratingVoxelStructure::GeneratingVoxelStructure(
    VoxelStructureMeta meta,
    std::unique_ptr<VoxelFragment> structure
) : fragments({std::move(structure)}),
    meta(std::move(meta)) {}


Generator::Generator(std::string name) : name(std::move(name)) {}
