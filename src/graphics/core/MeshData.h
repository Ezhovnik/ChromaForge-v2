#pragma once

#include <vector>

#include <typedefs.h>
#include <util/Buffer.h>

struct vattr {
    ubyte size;
};

struct MeshData {
    util::Buffer<float> vertices;
    util::Buffer<int> indices;
    util::Buffer<vattr> attrs;

    MeshData() = default;

    MeshData(
        util::Buffer<float> vertices, 
        util::Buffer<int> indices,
        util::Buffer<vattr> attrs
    ) : vertices(std::move(vertices)),
        indices(std::move(indices)),
        attrs(std::move(attrs)
    ) {}
};
