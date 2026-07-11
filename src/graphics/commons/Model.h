#pragma once

#include <string>
#include <vector>

#include <glm/glm.hpp>

#include <math/UVRegion.h>

namespace model {
    struct Vertex {
        glm::vec3 coord;
        glm::vec2 uv;
        glm::vec3 normal;
    };

    struct Mesh {
        std::string texture;
        std::vector<Vertex> vertices;

        bool lighting = true;

        void addPlane(
            const glm::vec3& pos,
            const glm::vec3& right,
            const glm::vec3& up,
            const glm::vec3& norm
        );
        void addPlane(
            const glm::vec3& pos,
            const glm::vec3& right,
            const glm::vec3& up,
            const glm::vec3& norm,
            const UVRegion& region
        );
        void addRect(
            const glm::vec3& pos,
            const glm::vec3& right,
            const glm::vec3& up,
            const glm::vec3& norm,
            const UVRegion& region
        );
        void addBox(const glm::vec3& pos, const glm::vec3& size);
        void addBox(
            const glm::vec3& pos,
            const glm::vec3& size,
            const UVRegion (&texfaces)[6],
            const bool enabledSides[6]
        );
        void scale(const glm::vec3& size);
    };

    struct Model {
        std::vector<Mesh> meshes;

        Mesh& addMesh(const std::string& texture) {
            for (auto& mesh : meshes) {
                if (mesh.texture == texture) {
                    return mesh;
                }
            }
            meshes.push_back({texture, {}});
            return meshes[meshes.size() - 1];
        }

        void clean();
    };
}
