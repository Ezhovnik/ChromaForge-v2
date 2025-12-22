#ifndef GRAPHICS_TEXTURE_H_
#define GRAPHICS_TEXTURE_H_

#include <string>

typedef unsigned int uint;

class Texture {
public:
    uint id;
    int width, height;
    Texture (uint id, int width, int height);
    ~Texture();

    void bind();
};

extern Texture* loadTexture(std::string filename);

#endif