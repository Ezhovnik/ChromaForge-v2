#pragma once

#include <vector>
#include <memory>

class Mesh;
class Assets;
class Framebuffer;
class DrawContext;
class ImageData;
class PostEffect;

class PostProcessing {
private:
    std::unique_ptr<Framebuffer> fbo;
    std::unique_ptr<Framebuffer> fboSecond;
    std::unique_ptr<Mesh> quadMesh;
    std::vector<std::shared_ptr<PostEffect>> effectSlots;
public:
    PostProcessing();
    ~PostProcessing();

    void use(DrawContext& context);

    void render(const DrawContext& context, const Assets& assets, float timer);

    std::unique_ptr<ImageData> toImage();

    Framebuffer* getFramebuffer() const;
};
