#ifndef CUBETEXTURE_CLASS_H
#define CUBETEXTURE_CLASS_H

#include <../include/glad/glad.h>
#include "../include/stb/stb_image.h"
#include <vector>

#include "shaderClass.h"

class CubeTexture {
    public:
        GLuint ID;
        const char* type;
        GLuint unit;

        CubeTexture() = default;
        CubeTexture(std::vector<std::string> faces, const char* texType, GLuint slot);

        void texUnit(Shader& shader, const char* uniform, GLuint unit);
        void Bind();
        void Unbind();
        void Delete();
};

#endif