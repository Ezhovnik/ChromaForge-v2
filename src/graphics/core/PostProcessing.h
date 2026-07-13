#pragma once

#include <vector>
#include <memory>

#include <glm/glm.hpp>

#include <graphics/core/MeshData.h>

template<typename VertexStructure> class Mesh;
class Assets;
class Framebuffer;
class DrawContext;
class ImageData;
class PostEffect;
class Camera;
class GBuffer;
class ShaderProgram;

struct PostProcessingVertex {
    glm::vec2 position;

    static constexpr VertexAttribute ATTRIBUTES[] {
        {VertexAttribute::Type::FLOAT, false, 2},
        {{}, 0}
    };
};

class PostProcessing {
private:
public:
    PostProcessing(size_t effectSlotsCount);
    ~PostProcessing();

    void use(DrawContext& context, bool gbufferPipeline);

    void render(
        const DrawContext& context,
        const Assets& assets,
        float timer,
        const Camera& camera,
        uint shadowMap,
        uint shadowMap2,
        const glm::mat4& shadowMatrix,
        const glm::mat4& shadowMatrix2,
        uint shadowMapResolution
    );

    void setEffect(size_t slot, std::shared_ptr<PostEffect> effect);
    PostEffect* getEffect(size_t slot);

    std::unique_ptr<ImageData> toImage();

    Framebuffer* getFramebuffer() const;
    void bindDepthBuffer();
private:
    void configureEffect(
        const DrawContext& context,
        PostEffect& effect,
        ShaderProgram& shader,
        float timer,
        const Camera& camera,
        uint shadowMap,
        uint shadowMap2,
        const glm::mat4& shadowMatrix,
        const glm::mat4& shadowMatrix2,
        uint shadowMapResolution
    );

    void refreshFbos(uint width, uint height);

    std::unique_ptr<Framebuffer> fbo;
    std::unique_ptr<Framebuffer> fboSecond;
    std::unique_ptr<Mesh<PostProcessingVertex>> quadMesh;
    std::vector<std::shared_ptr<PostEffect>> effectSlots;
    std::unique_ptr<GBuffer> gbuffer;

    uint noiseTexture;
};
