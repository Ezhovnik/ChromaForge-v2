#pragma once

#include <memory>

#include <glm/glm.hpp>

#include <typedefs.h>

class Atlas;
class Batch3D;
class Block;
class ContentGfxCache;
class Assets;
class ContentIndices;
class ImageData;
class Framebuffer;
class ShaderProgram;
class Window;

class BlocksPreview {
private:
    static std::unique_ptr<ImageData> draw(
        const ContentGfxCache& cache,
        ShaderProgram& shader,
        const Framebuffer& framebuffer,
        Batch3D& batch,
        const Block& block, 
        int size
    );
public:
    static std::unique_ptr<Atlas> build(
        Window& window,
        const ContentGfxCache& cache,
        const Assets& assets, 
        const ContentIndices& indices
    );
};
