#include "../Header Files/Block.h"
#include "../include/json/json.h"
#include <fstream>
#include <sstream>

using json = nlohmann::json;

Block::Block(const std::string& key) {
    std::ifstream file("../Resource Files/blocks.json");
    if (!file.is_open()) {
        throw "Failed to open blocks.json";
    }

    json blocksData;
    file >> blocksData;
    file.close();

    BlockInfo info;
    info.name = blocksData["blocks"][key]["name"];
    if (blocksData["blocks"][key]["textures"].contains("all")) {
        std::string texPath = blocksData["blocks"][key]["textures"]["all"];
        info.textures["top"] = texPath;
        info.textures["bottom"] = texPath;
        info.textures["front"] = texPath;
        info.textures["back"] = texPath;
        info.textures["left"] = texPath;
        info.textures["right"] = texPath;

    } else if (blocksData["blocks"][key]["textures"].contains("sides")) {
        std::string sidesPath = blocksData["blocks"][key]["textures"]["sides"];

        info.textures["top"] = blocksData["blocks"][key]["textures"]["top"];
        info.textures["bottom"] = blocksData["blocks"][key]["textures"]["bottom"];
        info.textures["front"] = sidesPath;
        info.textures["back"] = sidesPath;
        info.textures["left"] = sidesPath;
        info.textures["right"] = sidesPath;
    } else {
        std::vector<std::string> sides = {"top", "bottom", "front", "back", "left", "right"};
        for(const auto side: sides) {
            info.textures[side] = blocksData["blocks"][key]["textures"][side];
        }
    }
}

void Block::Draw(Shader& shader, Camera& camera, const glm::vec3& position) {
    // Vertex vertices[] = { // Координаты вершин
    //     // Передняя сторона
    //     Vertex{glm::vec3(-0.5f, -0.5f, 0.5f), glm::vec3(0.83f, 0.70f, 0.44f), glm::vec2(0.0f, 0.0f), 0, glm::vec3(0.0f, 0.0f, 1.0f)}, // 0
    //     Vertex{glm::vec3(0.5f, -0.5f, 0.5f), glm::vec3(0.83f, 0.70f, 0.44f), glm::vec2(1.0f, 0.0f), 0, glm::vec3(0.0f, 0.0f, 1.0f)}, // 1
    //     Vertex{glm::vec3(0.5f, 0.5f, 0.5f), glm::vec3(0.83f, 0.70f, 0.44f), glm::vec2(1.0f, 1.0f), 0, glm::vec3(0.0f, 0.0f, 1.0f)}, // 2
    //     Vertex{glm::vec3(-0.5f, 0.5f, 0.5f), glm::vec3(0.83f, 0.70f, 0.44f), glm::vec2(0.0f, 1.0f), 0, glm::vec3(0.0f, 0.0f, 1.0f)}, // 3

    //     // Задняя сторона
    //     Vertex{glm::vec3(-0.5f, -0.5f, -0.5f), glm::vec3(0.83f, 0.70f, 0.44f), glm::vec2(1.0f, 0.0f), 1, glm::vec3(0.0f, 0.0f, -1.0f)}, // 4
    //     Vertex{glm::vec3(0.5f, -0.5f, -0.5f), glm::vec3(0.83f, 0.70f, 0.44f), glm::vec2(0.0f, 0.0f), 1, glm::vec3(0.0f, 0.0f, -1.0f)}, // 5
    //     Vertex{glm::vec3(0.5f, 0.5f, -0.5f), glm::vec3(0.83f, 0.70f, 0.44f), glm::vec2(0.0f, 1.0f), 1, glm::vec3(0.0f, 0.0f, -1.0f)}, // 6
    //     Vertex{glm::vec3(-0.5f, 0.5f, -0.5f), glm::vec3(0.83f, 0.70f, 0.44f), glm::vec2(1.0f, 1.0f), 1, glm::vec3(0.0f, 0.0f, -1.0f)}, // 7

    //     // Верхняя сторона
    //     Vertex{glm::vec3(-0.5f, 0.5f, 0.5f), glm::vec3(0.83f, 0.70f, 0.44f), glm::vec2(0.0f, 0.0f), 2, glm::vec3(0.0f, 1.0f, 0.0f)}, // 8
    //     Vertex{glm::vec3(0.5f, 0.5f, 0.5f), glm::vec3(0.83f, 0.70f, 0.44f), glm::vec2(1.0f, 0.0f), 2, glm::vec3(0.0f, 1.0f, 0.0f)}, // 9
    //     Vertex{glm::vec3(0.5f, 0.5f, -0.5f), glm::vec3(0.83f, 0.70f, 0.44f), glm::vec2(1.0f, 1.0f), 2, glm::vec3(0.0f, 1.0f, 0.0f)}, // 10
    //     Vertex{glm::vec3(-0.5f, 0.5f, -0.5f), glm::vec3(0.83f, 0.70f, 0.44f), glm::vec2(0.0f, 1.0f), 2, glm::vec3(0.0f, 1.0f, 0.0f)}, // 11

    //     // Нижняя сторона
    //     Vertex{glm::vec3(-0.5f, -0.5f, 0.5f), glm::vec3(0.83f, 0.70f, 0.44f), glm::vec2(1.0f, 1.0f), 3, glm::vec3(0.0f, -1.0f, 0.0f)}, // 12
    //     Vertex{glm::vec3(0.5f, -0.5f, 0.5f), glm::vec3(0.83f, 0.70f, 0.44f), glm::vec2(0.0f, 1.0f), 3, glm::vec3(0.0f, -1.0f, 0.0f)}, // 13
    //     Vertex{glm::vec3(0.5f, -0.5f, -0.5f), glm::vec3(0.83f, 0.70f, 0.44f), glm::vec2(0.0f, 0.0f), 3, glm::vec3(0.0f, -1.0f, 0.0f)}, // 14
    //     Vertex{glm::vec3(-0.5f, -0.5f, -0.5f), glm::vec3(0.83f, 0.70f, 0.44f), glm::vec2(1.0f, 0.0f), 3, glm::vec3(0.0f, -1.0f, 0.0f)}, // 15

    //     // Правая сторона
    //     Vertex{glm::vec3(0.5f, -0.5f, 0.5f), glm::vec3(0.83f, 0.70f, 0.44f), glm::vec2(0.0f, 0.0f), 4, glm::vec3(1.0f, 0.0f, 0.0f)}, // 16
    //     Vertex{glm::vec3(0.5f, -0.5f, -0.5f), glm::vec3(0.83f, 0.70f, 0.44f), glm::vec2(1.0f, 0.0f), 4, glm::vec3(1.0f, 0.0f, 0.0f)}, // 17
    //     Vertex{glm::vec3(0.5f, 0.5f, -0.5f), glm::vec3(0.83f, 0.70f, 0.44f), glm::vec2(1.0f, 1.0f), 4, glm::vec3(1.0f, 0.0f, 0.0f)}, // 18
    //     Vertex{glm::vec3(0.5f, 0.5f, 0.5f), glm::vec3(0.83f, 0.70f, 0.44f), glm::vec2(0.0f, 1.0f), 4, glm::vec3(1.0f, 0.0f, 0.0f)}, // 19

    //     // Правая сторона
    //     Vertex{glm::vec3(-0.5f, -0.5f, 0.5f), glm::vec3(0.83f, 0.70f, 0.44f), glm::vec2(1.0f, 0.0f), 5, glm::vec3(-1.0f, 0.0f, 0.0f)}, // 20
    //     Vertex{glm::vec3(-0.5f, -0.5f, -0.5f), glm::vec3(0.83f, 0.70f, 0.44f), glm::vec2(0.0f, 0.0f), 5, glm::vec3(-1.0f, 0.0f, 0.0f)}, // 21
    //     Vertex{glm::vec3(-0.5f, 0.5f, -0.5f), glm::vec3(0.83f, 0.70f, 0.44f), glm::vec2(0.0f, 1.0f), 5, glm::vec3(-1.0f, 0.0f, 0.0f)}, // 22
    //     Vertex{glm::vec3(-0.5f, 0.5f, 0.5f), glm::vec3(0.83f, 0.70f, 0.44f), glm::vec2(1.0f, 1.0f), 5, glm::vec3(-1.0f, 0.0f, 0.0f)} // 23
    // };

    GLuint indices[] =
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
}