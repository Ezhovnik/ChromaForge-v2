#ifndef BLOCK_CLASS_H
#define BLOCK_CLASS_H

#include <unordered_map>
#include <mutex>
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

        static void clearCache(); // Очистка кеша
        static void removeFromCache(const std::string& filePath); // Очистка конкретного файла из кеша
    private:
        CubeMesh mesh;
        std::string jsonFilePath;
        std::string texturesPath;

        // Статическое кеширование JSON данных
        static std::unordered_map<std::string, json> jsonCache;
        static std::mutex cacheMutex;

        json parseJsonFile();
        json getBlockData();
        std::vector<std::string> getBlockTextures();
};

#endif