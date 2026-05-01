#pragma once

#include <vector>
#include <glm/glm.hpp>

#include <interfaces/Serializable.h>
#include <voxels/voxel.h>

inline constexpr int STRUCTURE_FORMAT_VERSION = 1;

class Level;
class Content;

struct VoxelStructure : public Serializable {
    glm::ivec3 size;
    std::vector<voxel> voxels;
    std::vector<std::string> blockNames;

    VoxelStructure() : size() {}
    VoxelStructure(
        glm::ivec3 size,
        std::vector<voxel> voxels,
        std::vector<std::string> blockNames
    ) : size(size),
        voxels(std::move(voxels)),
        blockNames(std::move(blockNames)) {}

    dv::value serialize() const override;
    void deserialize(const dv::value& src) override;

    static std::unique_ptr<VoxelStructure> create(
        Level* level, const glm::ivec3& a, const glm::ivec3& b, bool entities
    );
};
