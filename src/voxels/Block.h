#ifndef VOXELS_BLOCK_H_
#define VOXELS_BLOCK_H_

typedef unsigned int uint;

class Block {
public:
    static Block* blocks[256];

    const uint id;
    int textureFaces[6]; // -x, +x, -y, +y, -z, +z
    unsigned char emission[3];
    unsigned char drawGroup = 0;
    bool lightPassing = false;
    bool obstacle = true;

    Block(uint id, int texture);
};

#endif // VOXELS_BLOCK_H_
