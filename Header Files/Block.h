#ifndef BLOCK_CLASS_H
#define BLOCK_CLASS_H

#include <../include/glm/glm.hpp>
#include "Texture.h"
#include "Mesh.h"
#include <map>

struct BlockInfo {
    std::string name;
    std::map<std::string, std::string> textures;
};

class Block {
    public:
        BlockInfo info;

        Block(const std::string& key);
        
        void Draw(Shader& shader, Camera& camera, const glm::vec3& position);

};

#endif