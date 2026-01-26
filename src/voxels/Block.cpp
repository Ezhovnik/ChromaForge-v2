#include "Block.h"

Block* Block::blocks[256];

Block::Block(uint id, int texture) : id(id), textureFaces{texture,texture,texture,texture,texture,texture}, emission{0, 0, 0}{
}
