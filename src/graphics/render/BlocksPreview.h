#ifndef FRONTEND_BLOCKS_PREVIEW_H_
#define FRONTEND_BLOCKS_PREVIEW_H_

#include <memory>

#include <glm/glm.hpp>

#include <typedefs.h>

class Atlas;
class Batch3D;
class Block;
class ContentGfxCache;
class Assets;
class Content;
class ImageData;
class Framebuffer;
class ShaderProgram;

class BlocksPreview {
private:
    static std::unique_ptr<ImageData> draw(
        const ContentGfxCache* cache,
        ShaderProgram* shader,
        Framebuffer* framebuffer,
        Batch3D* batch,
        const Block& block, 
        int size
    );
public:
    static std::unique_ptr<Atlas> build(
        const ContentGfxCache* cache,
        Assets* assets, 
        const Content* content
    );
};

#endif // FRONTEND_BLOCKS_PREVIEW_H_
