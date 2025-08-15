#include <fstream>
#include "../Header Files/Block.h"

const Vertex Block::vertices[] = {
    // Передняя сторона
    Vertex{glm::vec3(-0.5f, -0.5f, 0.5f), glm::vec2(0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f)}, // 0
    Vertex{glm::vec3(0.5f, -0.5f, 0.5f), glm::vec2(1.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f)}, // 1
    Vertex{glm::vec3(0.5f, 0.5f, 0.5f), glm::vec2(1.0f, 1.0f), glm::vec3(0.0f, 0.0f, 1.0f)}, // 2
    Vertex{glm::vec3(-0.5f, 0.5f, 0.5f), glm::vec2(0.0f, 1.0f), glm::vec3(0.0f, 0.0f, 1.0f)}, // 3

    // Задняя сторона
    Vertex{glm::vec3(-0.5f, -0.5f, -0.5f), glm::vec2(1.0f, 0.0f), glm::vec3(0.0f, 0.0f, -1.0f)}, // 4
    Vertex{glm::vec3(0.5f, -0.5f, -0.5f), glm::vec2(0.0f, 0.0f), glm::vec3(0.0f, 0.0f, -1.0f)}, // 5
    Vertex{glm::vec3(0.5f, 0.5f, -0.5f), glm::vec2(0.0f, 1.0f), glm::vec3(0.0f, 0.0f, -1.0f)}, // 6
    Vertex{glm::vec3(-0.5f, 0.5f, -0.5f), glm::vec2(1.0f, 1.0f), glm::vec3(0.0f, 0.0f, -1.0f)}, // 7

    // Верхняя сторона
    Vertex{glm::vec3(-0.5f, 0.5f, 0.5f), glm::vec2(0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f)}, // 8
    Vertex{glm::vec3(0.5f, 0.5f, 0.5f), glm::vec2(1.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f)}, // 9
    Vertex{glm::vec3(0.5f, 0.5f, -0.5f), glm::vec2(1.0f, 1.0f), glm::vec3(0.0f, 1.0f, 0.0f)}, // 10
    Vertex{glm::vec3(-0.5f, 0.5f, -0.5f), glm::vec2(0.0f, 1.0f), glm::vec3(0.0f, 1.0f, 0.0f)}, // 11

    // Нижняя сторона
    Vertex{glm::vec3(-0.5f, -0.5f, 0.5f), glm::vec2(1.0f, 1.0f), glm::vec3(0.0f, -1.0f, 0.0f)}, // 12
    Vertex{glm::vec3(0.5f, -0.5f, 0.5f), glm::vec2(0.0f, 1.0f), glm::vec3(0.0f, -1.0f, 0.0f)}, // 13
    Vertex{glm::vec3(0.5f, -0.5f, -0.5f), glm::vec2(0.0f, 0.0f), glm::vec3(0.0f, -1.0f, 0.0f)}, // 14
    Vertex{glm::vec3(-0.5f, -0.5f, -0.5f), glm::vec2(1.0f, 0.0f), glm::vec3(0.0f, -1.0f, 0.0f)}, // 15

    // Правая сторона
    Vertex{glm::vec3(0.5f, -0.5f, 0.5f), glm::vec2(0.0f, 0.0f), glm::vec3(1.0f, 0.0f, 0.0f)}, // 16
    Vertex{glm::vec3(0.5f, -0.5f, -0.5f), glm::vec2(1.0f, 0.0f), glm::vec3(1.0f, 0.0f, 0.0f)}, // 17
    Vertex{glm::vec3(0.5f, 0.5f, -0.5f), glm::vec2(1.0f, 1.0f), glm::vec3(1.0f, 0.0f, 0.0f)}, // 18
    Vertex{glm::vec3(0.5f, 0.5f, 0.5f), glm::vec2(0.0f, 1.0f), glm::vec3(1.0f, 0.0f, 0.0f)}, // 19

    // Правая сторона
    Vertex{glm::vec3(-0.5f, -0.5f, 0.5f), glm::vec2(1.0f, 0.0f), glm::vec3(-1.0f, 0.0f, 0.0f)}, // 20
    Vertex{glm::vec3(-0.5f, -0.5f, -0.5f), glm::vec2(0.0f, 0.0f), glm::vec3(-1.0f, 0.0f, 0.0f)}, // 21
    Vertex{glm::vec3(-0.5f, 0.5f, -0.5f), glm::vec2(0.0f, 1.0f), glm::vec3(-1.0f, 0.0f, 0.0f)}, // 22
    Vertex{glm::vec3(-0.5f, 0.5f, 0.5f), glm::vec2(1.0f, 1.0f), glm::vec3(-1.0f, 0.0f, 0.0f)} // 23
};

const GLuint Block::indices[] =
{
    // Передняя сторона
    0, 1, 2,
    2, 3, 0,
    
    // Задняя сторона
    5, 4, 7,
    7, 6, 5,
    
    // Верхняя сторона
    8, 9, 10,
    10, 11, 8,
    
    // Нижняя сторона
    14, 13, 12,
    12, 15, 14,
    
    // Правая сторона
    16, 17, 18,
    18, 19, 16,
    
    // Левая сторона
    21, 20, 23,
    23, 22, 21
};

Block::Block(std::string jsonKey, std::string jsonPath, std::string texPath){
        Block::blockKey = jsonKey;
        Block::jsonFilePath = jsonPath;
        Block::texturesPath = texPath;

        std::vector<std::string> blockTexturesPath = getBlockTextures();
        std::cout << jsonKey << " textures path:" << std::endl;
        for(int i = 0; i < 6; i++) {
            std::cout << blockTexturesPath[i] << std::endl;
        }
        Block::blockTexture = CubeTexture(blockTexturesPath, "diffuse", 0);

        std::vector <Vertex> verts(vertices, vertices + sizeof(vertices) / sizeof(Vertex));
        std::vector <GLuint> ind(indices, indices + sizeof(indices) / sizeof(GLuint));

        Block::mesh = CubeMesh(verts, ind, blockTexture);
    }

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

void Block::Draw(Shader& shader, Camera& camera, glm::vec3 position) {
    glm::mat4 cubeModel = glm::mat4(1.0f);
    cubeModel = glm::translate(cubeModel, position);

    glUniformMatrix4fv(glGetUniformLocation(shader.ID, "model"), 1, GL_FALSE, glm::value_ptr(cubeModel));
    mesh.Draw(shader, camera);
}