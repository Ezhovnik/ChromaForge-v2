#include "Block.h"

Block* Block::blocks[256];

Block::Block(std::string name, std::string texture) : name(name), textureFaces{texture,texture,texture,texture,texture,texture}, emission{0, 0, 0}{
}
