#include <fstream>
#include "../Header Files/Block.h"
#include "../Header Files/CubeParams.h"

Block::Block(std::string jsonKey, std::string jsonPath, std::string texPath){
    // Сохраняем переданные данные в объекте класса
    Block::blockKey = jsonKey;
    Block::jsonFilePath = jsonPath;
    Block::texturesPath = texPath;

    std::vector<std::string> blockTexturesPath = getBlockTextures();
    Block::blockTexture = CubeTexture(blockTexturesPath, "diffuse", 0);

    std::vector <Vertex> verts(cubeVertices, cubeVertices + cubeVerticesCount);
    std::vector <GLuint> ind(cubeIndices, cubeIndices + cubeIndicesCount);

    Block::mesh = CubeMesh(verts, ind, blockTexture);
}

// Читаем JSON файл
json Block::parseJsonFile() {
    std::ifstream file(jsonFilePath);
    if (!file.is_open()) {
        std::cerr << "WARNING::The file is on the path '" << jsonFilePath << "' was not found" << std::endl;
        Block::texturesPath = "../Resource Files/Textures/Blocks/";
        return (json){};
    }

    json BlocksData = json::parse(file);
    return BlocksData;
}

// Читаем данные о блоке из JSON файла
json Block::getBlockData() {
    json BlocksData = parseJsonFile();
    if(!BlocksData.contains("blocks")) {
        std::cerr << "WARNING::The 'blocks' key was not found" << std::endl;
        Block::texturesPath = "../Resource Files/Textures/Blocks/";
        return (json){"unknown", {"name", "Unknown Block"}, {"textures", {"all", "unknown.png"}}};
    }

    BlocksData = BlocksData["blocks"];
    if(!BlocksData.contains(blockKey)) {
        std::cerr << "WARNING::The '" << blockKey << "' block was not found" << std::endl;
        Block::texturesPath = "../Resource Files/Textures/Blocks/";
        return (json){"unknown", {"name", "Unknown Block"}, {"textures", {"all", "unknown.png"}}};
    }

    return BlocksData[blockKey];
}

// Получаем вектор с путями текстур блока
std::vector<std::string> Block::getBlockTextures() {
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
void Block::Draw(Shader& shader, Camera& camera, glm::vec3 position) {
    glm::mat4 cubeModel = glm::mat4(1.0f);
    cubeModel = glm::translate(cubeModel, position);

    shader.setMat4("model", cubeModel);
    mesh.Draw(shader, camera);
}