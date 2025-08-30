#ifndef BLOCK_CLASS_H
#define BLOCK_CLASS_H

#include <unordered_map>
#include <mutex>
#include "../include/json/json.h"
#include "CubeTexture.h"

using json = nlohmann::json;

class Block {
    public:
        std::string blockKey;
        CubeTexture blockTexture;

        bool isTransparent = false;

        Block(
            std::string jsonKey, 
            std::string jsonPath = "../Resource Files/blocks.json", 
            std::string texPath = "../Resource Files/Textures/Blocks/"
        );

        static void clearCache(); // Очистка кеша
        static void removeFromCache(const std::string& filePath); // Очистка конкретного файла из кеша
    private:
        std::string jsonFilePath;
        std::string texturesPath;

        // Статическое кеширование JSON данных
        static std::unordered_map<std::string, json> jsonCache;
        static std::mutex cacheMutex;

        json parseJsonFile();
        json getBlockData();
        std::vector<std::string> getBlockTextures(json BlockData);
};

#endif