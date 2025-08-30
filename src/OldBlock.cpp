#include <fstream>
#include "../Header Files/OldBlock.h"
#include "../Header Files/CubeParams.h"

std::unordered_map<std::string, json> OldBlock::jsonCache;
std::mutex OldBlock::cacheMutex;

OldBlock::OldBlock(std::string jsonKey, std::string jsonPath, std::string texPath){
    // Сохраняем переданные данные в объекте класса
    OldBlock::blockKey = jsonKey;
    OldBlock::jsonFilePath = jsonPath;
    OldBlock::texturesPath = texPath;

    std::vector<std::string> blockTexturesPath = getBlockTextures();
    OldBlock::blockTexture = CubeTexture(blockTexturesPath, "diffuse", 0);

    std::vector <Vertex> verts(cubeVertices, cubeVertices + cubeVerticesCount);
    std::vector <GLuint> ind(cubeIndices, cubeIndices + cubeIndicesCount);

    OldBlock::mesh = BlockMesh(verts, ind, blockTexture);
}

// Читаем JSON файл
json OldBlock::parseJsonFile() {
    // Проверяем, есть ли файл уже в кеше
    {
        std::lock_guard<std::mutex> lock(cacheMutex);
        auto it = jsonCache.find(jsonFilePath);
        if (it != jsonCache.end()) {
            // Файл найден в кеше - возвращаем кешированную версию
            return it->second;
        }
    }

    // Файла нет в кеше - читаем с диска
    std::ifstream file(jsonFilePath);
    if (!file.is_open()) {
        std::cerr << "WARNING::The file on path '" << jsonFilePath << "' was not found" << std::endl;
        OldBlock::texturesPath = "../Resource Files/Textures/Blocks/";
        return json{};
    }
    try {
        json BlocksData = json::parse(file);
        file.close();
        
        // Сохраняем кеш
        {
            std::lock_guard<std::mutex> lock(cacheMutex);
            jsonCache[jsonFilePath] = BlocksData;
        }
        
        return BlocksData;
    } catch (const std::exception& err) {
        std::cerr << "ERROR::Failed to parse JSON file '" << jsonFilePath << "': " << err.what() << std::endl;
        file.close();
        OldBlock::texturesPath = "../Resource Files/Textures/Blocks/";
        return json{};
    }
}

// Читаем данные о блоке из JSON файла
json OldBlock::getBlockData() {
    json BlocksData = parseJsonFile();
    if(!BlocksData.contains("blocks")) {
        std::cerr << "WARNING::The 'blocks' key was not found" << std::endl;
        OldBlock::texturesPath = "../Resource Files/Textures/Blocks/";
        return (json){"unknown", {"name", "Unknown Block"}, {"textures", {"all", "unknown.png"}}};
    }

    BlocksData = BlocksData["blocks"];
    if(!BlocksData.contains(blockKey)) {
        std::cerr << "WARNING::The '" << blockKey << "' block was not found" << std::endl;
        OldBlock::texturesPath = "../Resource Files/Textures/Blocks/";
        return (json){"unknown", {"name", "Unknown Block"}, {"textures", {"all", "unknown.png"}}};
    }

    return BlocksData[blockKey];
}

// Получаем вектор с путями текстур блока
std::vector<std::string> OldBlock::getBlockTextures() {
    json BlockData = getBlockData();
    json TexturesData;
    if (BlockData.contains("textures")) {
        TexturesData = BlockData["textures"];
    } else {
        std::cerr << "WARNING::The 'textures' key was not found" << std::endl;
        TexturesData = (json){"all", "unknown.png"};
    }

    std::vector<std::string> textures(6, "../Resource Files/Textures/Blocks/unknown.png");

    if(TexturesData.contains("all")) {
        std::string allPath = TexturesData["all"];
        allPath = texturesPath + allPath;
        for(int i = 0; i < 6; i++) {
            textures[i] = allPath;
        }
        return textures;
    }

    if(TexturesData.contains("sides")) {
        std::string sidesPath = TexturesData["sides"];
        sidesPath = texturesPath + sidesPath;
        textures[0] = sidesPath;
        textures[1] = sidesPath;
        textures[4] = sidesPath;
        textures[5] = sidesPath;
    } else {
        if(TexturesData.contains("right")) {
            std::string sidePath = TexturesData["right"];
            sidePath = texturesPath +  sidePath;
            textures[0] =  sidePath;
        }
        if(TexturesData.contains("left")) {
            std::string  sidePath = TexturesData["left"];
            sidePath = texturesPath + sidePath;
            textures[1] = sidePath;
        }
        if(TexturesData.contains("front")) {
            std::string sidePath = TexturesData["front"];
            sidePath = texturesPath + sidePath;
            textures[4] = sidePath;
        }
        if(TexturesData.contains("back")) {
            std::string sidePath = TexturesData["back"];
            sidePath = texturesPath + sidePath;
            textures[5] = sidePath;
        }
    }

    if (TexturesData.contains("foundations")) {
        std::string foundationsPath = TexturesData["foundations"];
        foundationsPath = texturesPath + foundationsPath;
        textures[2] = foundationsPath;
        textures[3] = foundationsPath;
    } else {
        if(TexturesData.contains("top")) {
            std::string foundationPath = TexturesData["top"];
            foundationPath = texturesPath + foundationPath;
            textures[2] = foundationPath;
        }
        if(TexturesData.contains("bottom")) {
            std::string foundationPath = TexturesData["bottom"];
            foundationPath = texturesPath + foundationPath;
            textures[3] = foundationPath;
        }
    }

    return textures;
}

// Функция для отрисовки блока
void OldBlock::Draw(Shader& shader, Camera& camera, glm::vec3 position) {
    glm::mat4 cubeModel = glm::mat4(1.0f);
    cubeModel = glm::translate(cubeModel, position);

    shader.setMat4("model", cubeModel);
    mesh.Draw(shader, camera);
}

// Очистка всего кеша
void OldBlock::clearCache() {
    std::lock_guard<std::mutex> lock(cacheMutex);
    jsonCache.clear();
    std::cout << "Block JSON cache cleared" << std::endl;
}

// Удаление конкретного файла из кеша
void OldBlock::removeFromCache(const std::string& filePath) {
    std::lock_guard<std::mutex> lock(cacheMutex);
    auto it = jsonCache.find(filePath);
    if (it != jsonCache.end()) {
        jsonCache.erase(it);
        std::cout << "Removed '" << filePath << "' from Block JSON cache" << std::endl;
    }
}