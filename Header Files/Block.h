#ifndef BLOCK_CLASS_H
#define BLOCK_CLASS_H

#include "CubeMesh.h"
#include "../include/json/json.h"

using json = nlohmann::json;

class Block {
    public:
        std::string blockKey;
        CubeTexture blockTexture;

        Block(
            std::string jsonKey, 
            std::string jsonPath = "../Resource Files/blocks.json", 
            std::string texPath = "../Resource Files/Textures/Blocks/"
        );

        void Draw(Shader& shader, Camera& camera, glm::vec3 position);
    private:
        CubeMesh mesh;
        std::string jsonFilePath;
        std::string texturesPath;

        json parseJsonFile();
        json getBlockData();
        std::vector<std::string> getBlockTextures();
};

#endif