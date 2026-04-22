#pragma once

#include <memory>

class Mesh;
class ShaderProgram;
class Framebuffer;
class DrawContext;
class ImageData;

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
