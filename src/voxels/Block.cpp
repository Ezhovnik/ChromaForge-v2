#include "Block.h"

std::array<std::unique_ptr<Block>, 256> Block::blocks;

Block::Block(uint id, int texture) : id(id), textureFaces{texture,texture,texture,texture,texture,texture}, emission{0, 0, 0}{
}
