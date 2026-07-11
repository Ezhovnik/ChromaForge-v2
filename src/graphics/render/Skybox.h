#pragma once

#include <memory>
#include <string>
#include <vector>

#include <glm/glm.hpp>

#include <typedefs.h>
#include <math/rand.h>
#include <graphics/core/MeshData.h>

template<typename VertexStructure> class Mesh;
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

struct skysprite {
    std::string texture;
    float phase;
    float distance;
    bool emissive;
};

class Skybox {
    std::unique_ptr<Framebuffer> fbo;
    uint size;
    ShaderProgram& shader;
    FastRandom random;
    glm::vec3 lightDir;

    std::unique_ptr<Mesh<SkyboxVertex>> mesh;
    std::unique_ptr<Batch3D> batch3d;
    std::vector<skysprite> sprites;
    int frameID = 0;

    float prevMie = -1.0f;
    float prevT = -1.0f;

    void drawStars(float angle, float opacity);
    void drawBackground(
        const Camera& camera,
        const Assets& assets,
        int width, int height
    );

    void refreshFace(uint face, Cubemap* cubemap);

    bool ready = false;
public:
    Skybox(uint size, ShaderProgram& shader);
    ~Skybox();

    void draw(
        const DrawContext& pctx,
        const Camera& camera,
        const Assets& assets,
        float daytime,
        float fog
    );

    void refresh(const DrawContext& parent_context, float t, float mie, uint quality);
    void bind() const;
    void unbind() const;
    bool isReady() const {return ready;};
    const glm::vec3 getLightDir() const {
        return lightDir;
    }
};
