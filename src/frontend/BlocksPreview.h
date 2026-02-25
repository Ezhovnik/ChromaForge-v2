#ifndef FRONTEND_BLOCKS_PREVIEW_H_
#define FRONTEND_BLOCKS_PREVIEW_H_

#include <memory>

#include <glm/glm.hpp>

#include "../typedefs.h"

class ShaderProgram;
class Atlas;
class Batch3D;
class Block;
class ContentGfxCache;
class Viewport;

class BlocksPreview {
private:
    ShaderProgram* shader;
    Atlas* atlas;
    std::unique_ptr<Batch3D> batch;
    const ContentGfxCache* const cache;
    const Viewport* viewport;
public:
    BlocksPreview(ShaderProgram* shader, Atlas* atlas, const ContentGfxCache* cache);
    ~BlocksPreview();

    void begin(const Viewport* viewport);
    void draw(const Block* block, int x, int y, int size, glm::vec4 tint);
};

#endif // FRONTEND_BLOCKS_PREVIEW_H_
