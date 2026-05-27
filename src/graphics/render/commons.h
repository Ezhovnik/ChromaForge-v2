#pragma once

#include <vector>
#include <memory>

#include <glm/vec3.hpp>

#include <graphics/core/MeshData.h>
#include <util/Buffer.h>

class Mesh;

struct SortingMeshEntry {
    glm::vec3 position;
    util::Buffer<float> vertexData;
};

struct SortingMeshData {
    std::vector<SortingMeshEntry> entries;
};

struct ChunkMeshData {
    MeshData mesh;
    SortingMeshData sortingMesh;
};

struct ChunkMesh {
    std::shared_ptr<Mesh> mesh;
    SortingMeshData sortingMesh;
};
