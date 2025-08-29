#ifndef BLOCKMESH_CLASS_H
#define BLOCKMESH_CLASS_H

#include "VAO.h"
#include "VBO.h"
#include "EBO.h"
#include "CubeTexture.h"
#include "Camera.h"

class BlockMesh {
    public:
        std::vector <Vertex> vertices;
        std::vector <GLuint> indices;
        CubeTexture cubeTexture;

        VAO VAO;

        BlockMesh() = default;
        BlockMesh(std::vector<Vertex>& vertices, std::vector<GLuint>& indices, CubeTexture& cubeTexture);

        void Draw(Shader& shader, Camera& camera);
};

#endif