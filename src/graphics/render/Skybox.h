#pragma once

#include <memory>
#include <string>
#include <vector>

#include <glm/glm.hpp>

#include <typedefs.h>
#include <math/rand.h>
#include <graphics/core/MeshData.h>
#include <world/Environment.h>

template<typename VertexStructure>
class Mesh;
class Assets;
class Camera;
class Batch3D;
class Framebuffer;
class Cubemap;
class DrawContext;
class ShaderProgram;

struct SkyboxVertex {
    glm::vec2 position;

    static constexpr VertexAttribute ATTRIBUTES[] {
        {VertexAttribute::Type::FLOAT, false, 2},
        {{}, 0}
    };
};

struct SkySprite {
    std::string texture;
    float phase;
    float distance;
    bool emissive;
    float altitude;
};

class Skybox {
    SkyMode mode = SkyMode::Solid;
    uint size;
    std::unique_ptr<Framebuffer> fbo;
    const Assets& assets;
    ShaderProgram& shader;
    util::FastRandom random;
    glm::vec3 lightDir;

    std::unique_ptr<Mesh<SkyboxVertex>> mesh;
    std::unique_ptr<Batch3D> batch3d;
    std::vector<SkySprite> sprites;
    int frameID = 0;

    float prevMie = -1.0f;
    float prevT = -1.0f;
    float sunAltitude = 45.0f;
    glm::vec3 prevHighlight {1.0f};
    glm::mat4 rotation;

    void drawStars(float angle, float opacity);
    void drawBackground(const Camera& camera, int width, int height);
    void drawSkySprites(float daytime, float angle, float opacity);

    void refreshFace(uint face, Cubemap& cubemap);
public:
    Skybox(uint size, const Assets& assets);
    ~Skybox();

    void setMode(SkyMode mode);

    void draw(
        const Environment& environment,
        const DrawContext& pctx,
        const Camera& camera,
        float daytime,
        float fog
    );

    void refresh(
        const Environment& environment,
        const DrawContext& pctx,
        float t,
        float mie,
        const glm::vec3& tint,
        const glm::vec3& hightlight,
        uint quality
    );

    const Cubemap* getCubemap() const;

    const glm::vec3& getLightDir() const {
        return lightDir;
    }
};
