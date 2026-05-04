#pragma once

#include <vector>
#include <glm/glm.hpp>

#include <interfaces/Serializable.h>
#include <voxels/voxel.h>

inline constexpr int STRUCTURE_FORMAT_VERSION = 1;

class Level;
class Content;

class VoxelFragment : public Serializable {
    glm::ivec3 size;
    std::vector<voxel> voxels;
    std::vector<std::string> blockNames;

    std::vector<voxel> voxelsRuntime;
public:
    VoxelFragment() : size() {}
    VoxelFragment(
        glm::ivec3 size,
        std::vector<voxel> voxels,
        std::vector<std::string> blockNames
    ) : size(size),
        voxels(std::move(voxels)),
        blockNames(std::move(blockNames)) {}

    dv::value serialize() const override;
    void deserialize(const dv::value& src) override;

    void prepare(const Content& content);

    std::unique_ptr<VoxelFragment> rotated(const Content& content) const;

    static std::unique_ptr<VoxelFragment> create(
        Level* level, const glm::ivec3& a, const glm::ivec3& b, bool entities
    );

    const glm::ivec3& getSize() const {
        return size;
    }

    const std::vector<voxel>& getRuntimeVoxels() {
        assert(!voxelsRuntime.empty());
        return voxelsRuntime;
    }
};
