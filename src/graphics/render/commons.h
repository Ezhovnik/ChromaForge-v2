#pragma once

#include <vector>
#include <memory>

#include <glm/vec3.hpp>

#include <graphics/core/MeshData.h>
#include <util/Buffer.h>

inline const VertexAttribute CHUNK_VATTRS[]{{3}, {2}, {1}, {0}};
inline constexpr int CHUNK_VERTEX_SIZE = 6;

class Mesh;

struct SortingMeshEntry {
    glm::vec3 position;
    util::Buffer<float> vertexData;
    long long distance;

    inline bool operator<(const SortingMeshEntry& o) const noexcept {
        return distance > o.distance;
    }
};

struct SortingMeshData {
    std::vector<SortingMeshEntry> entries;
};

struct ChunkMeshData {
    MeshData mesh;
    SortingMeshData sortingMesh;
};

struct ChunkMesh {
    std::unique_ptr<Mesh> mesh;
    SortingMeshData sortingMeshData;
    std::unique_ptr<Mesh> sortedMesh = nullptr;
};
