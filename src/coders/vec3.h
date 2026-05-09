#pragma once

#include <memory>
#include <string>
#include <vector>
#include <glm/glm.hpp>
#include <unordered_map>

#include <typedefs.h>
#include <util/Buffer.h>
#include <graphics/core/Model.h>

namespace vec3 {
    struct Material {
        int flags;
        std::string name;
    };

    struct Model {
        std::string name;
        model::Model model;
        glm::vec3 origin;

        Model& operator=(Model&&) = default;

        ~Model();
    };

    struct File {
        std::unordered_map<std::string, Model> models;
        std::vector<Material> materials;

        File(File&&) = default;

        File& operator=(File&&) = default;
    };

    File load(const std::string_view file, const util::Buffer<ubyte>& src);
}
