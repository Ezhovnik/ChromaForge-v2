#ifndef CUBEMESH_CLASS_H
#define CUBEMESH_CLASS_H

#include "VAO.h"
#include "VBO.h"
#include "EBO.h"
#include "CubeTexture.h"
#include "Camera.h"

class CubeMesh {
    public:
        std::vector <Vertex> vertices;
        std::vector <GLuint> indices;
        CubeTexture cubeTexture;

        VAO VAO;

        CubeMesh() = default;
        CubeMesh(std::vector<Vertex>& vertices, std::vector<GLuint>& indices, CubeTexture& cubeTexture);

        void Draw(Shader& shader, Camera& camera);
};

#endif