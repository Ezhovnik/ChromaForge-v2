#ifndef SKYBOXMESH_CLASS_H
#define SKYBOXMESH_CLASS_H

#include "../VAO.h"
#include "../VBO.h"
#include "../EBO.h"
#include "../CubeTexture.h"
#include "../Camera.h"

class SkyboxMesh {
    public:
        std::vector <Vertex> vertices;
        std::vector <GLuint> indices;
        CubeTexture skyboxTextureDay;
        CubeTexture skyboxTextureNight;

        VAO VAO;

        SkyboxMesh() = default;
        SkyboxMesh(std::vector<Vertex>& vertices, std::vector<GLuint>& indices, CubeTexture& skyboxTextureDay, CubeTexture& skyboxTextureNight);

        void Draw(Shader& shader, Camera& camera, float timesOfDay);
};

#endif