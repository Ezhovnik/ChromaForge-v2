#ifndef OBJECTS_SKELETON_H_
#define OBJECTS_SKELETON_H_

#include <vector>
#include <memory>
#include <string>
#include <unordered_map>

#include <glm/glm.hpp>

#include "../typedefs.h"

namespace rigging {
    struct Rig;

    struct Pose {
        std::vector<glm::mat4> matrices;
    };

    class RigNode {
    private:
        size_t index;
        std::string name;
        std::vector<std::unique_ptr<RigNode>> subnodes;
    public:
        RigNode(size_t index, std::string name, std::vector<std::unique_ptr<RigNode>> subnodes);

        size_t getIndex() const {
            return index;
        }

        const auto& getSubnodes() const {
            return subnodes;
        }
    };

    class RigConfig {
    private:
        std::unique_ptr<RigNode> root;
        std::unordered_map<std::string, size_t> indices;
        std::vector<RigNode*> nodes;
    public:
        RigConfig(std::unique_ptr<RigNode> root);
    };

    struct Rig {
        RigConfig* config;
        Pose pose;
        std::vector<std::string> textures;
    };
};

#endif // OBJECTS_SKELETON_H_
