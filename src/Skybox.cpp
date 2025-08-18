#include "../Header Files/Skybox.h"
#include "../Header Files/SkyboxParams.h"

Skybox::Skybox(std::string path) {
    std::string pathDayTexture = path + "Day/";
    std::string pathNightTexture = path + "Night/";

    Skybox::dayTexture = loadTexture(pathDayTexture, 0);
    Skybox::nightTexture = loadTexture(pathNightTexture, 1);

    std::vector <Vertex> verts(skyboxVertices, skyboxVertices + skyboxVerticesCount);
    std::vector <GLuint> ind(skyboxIndices, skyboxIndices + skyboxIndicesCount);

    Skybox::mesh = SkyboxMesh(verts, ind, dayTexture, nightTexture);
}

CubeTexture Skybox::loadTexture(std::string path, int slot) {
    std::vector<std::string> faces = {
        "right.png",
        "left.png",
        "top.png",
        "bottom.png",
        "front.png",
        "back.png"
    };

    std::vector<std::string> pathToFaces;
    for(auto face: faces) {
        std::string pathToFace = path + face;
        pathToFaces.push_back(pathToFace);
    }

    return CubeTexture(pathToFaces, "cubeMap", slot);
}

void Skybox::setDayDuration(int dayDurationInSparks) {
    Skybox::dayDurationInSparks = dayDurationInSparks;
}

void Skybox::Draw(Shader& shader, Camera& camera, float timesOfDayInSparks) {
    float timeFactor = timesOfDayInSparks / dayDurationInSparks;

    glDepthFunc(GL_LEQUAL);
    mesh.Draw(shader, camera, timeFactor);
    glDepthFunc(GL_LESS);
}