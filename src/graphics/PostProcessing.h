#ifndef GRAPHICS_POST_PROCESSING_H_
#define GRAPHICS_POST_PROCESSING_H_

#include "Viewport.h"
#include "GfxContext.h"

#include <memory>

class Mesh;
class ShaderProgram;
class Framebuffer;

class PostProcessing {
private:
    std::unique_ptr<Framebuffer> fbo;
    std::unique_ptr<Mesh> quadMesh;
public:
    PostProcessing();
    ~PostProcessing();

    void use(GfxContext& context);

    void render(const GfxContext& context, ShaderProgram* screenShader);
};

#endif // GRAPHICS_POST_PROCESSING_H_
