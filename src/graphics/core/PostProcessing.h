#ifndef GRAPHICS_CORE_POST_PROCESSING_H_
#define GRAPHICS_CORE_POST_PROCESSING_H_

#include <memory>

#include "Viewport.h"
#include "DrawContext.h"

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

    void use(DrawContext& context);

    void render(const DrawContext& context, ShaderProgram* screenShader);

    std::unique_ptr<ImageData> toImage();

    Framebuffer* getFramebuffer() const;
};

#endif // GRAPHICS_CORE_POST_PROCESSING_H_
