#include <graphics/render/Skybox.h>

#include <cmath>

#include <GL/glew.h>
#include <glm/glm.hpp>

#include <graphics/core/ShaderProgram.h>
#include <graphics/core/Mesh.h>
#include <window/Window.h>
#include <constants.h>
#include <assets/Assets.h>
#include <graphics/core/Batch3D.h>
#include <window/Camera.h>
#include <graphics/core/Texture.h>
#include <graphics/core/Cubemap.h>
#include <graphics/core/Framebuffer.h>
#include <math/UVRegion.h>
#include <graphics/core/DrawContext.h>

namespace SkyboxConsts {
    inline constexpr int STARS_COUNT = 3000;
    inline constexpr int STARS_SEED = 632;
}

Skybox::Skybox(
    uint size,
    ShaderProgram& shader
) : size(size),
    shader(shader),
    batch3d(std::make_unique<Batch3D>(4096))
{
    auto cubemap = std::make_unique<Cubemap>(size, size, ImageFormat::rgb888);

    uint fboid;
    glGenFramebuffers(1, &fboid);
    fbo = std::make_unique<Framebuffer>(fboid, 0, std::move(cubemap));

    SkyboxVertex vertices[]{
        {{-1.0f, -1.0f}},
        {{-1.0f, 1.0f}},
        {{1.0f, 1.0f}},
        {{-1.0f, -1.0f}},
        {{1.0f, 1.0f}},
        {{1.0f, -1.0f}}
    };
    mesh = std::make_unique<Mesh<SkyboxVertex>>(vertices, 6);

    sprites.push_back(skysprite{
        "misc/moon",
        PI * 0.5f,
        4.0f,
        false
    });

    sprites.push_back(skysprite{
        "misc/sun",
        PI * 1.5f,
        4.0f,
        true
    });
}

Skybox::~Skybox() = default;

void Skybox::drawBackground(
    const Camera& camera, const Assets& assets, int width, int height
) {
    auto backShader = assets.get<ShaderProgram>("background");
    backShader->use();
	backShader->uniformMatrix("u_view", camera.getView(false));
	backShader->uniform1f("u_zoom", camera.zoom * camera.getFov() / (PI * 0.5f));
	backShader->uniform1f("u_ar", float(width) / float(height));
    backShader->uniform1i("u_cubemap", 1);
    bind();
    mesh->draw();
    unbind();
}

void Skybox::drawStars(float angle, float opacity) {
    batch3d->texture(nullptr);
    random.setSeed(SkyboxConsts::STARS_SEED);
    for (int i = 0; i < SkyboxConsts::STARS_COUNT; ++i) {
        float rx = (random.randFloat()) - 0.5f;
        float ry = (random.randFloat()) - 0.5f;
        float z = (random.randFloat()) - 0.5f;
        float x = rx * std::sin(angle) + ry * -std::cos(angle);
        float y = rx * std::cos(angle) + ry * std::sin(angle);

        float sopacity = random.randFloat();
        if (y < 0.0f) continue;

        sopacity *= (0.2f + std::sqrt(std::cos(angle)) * 0.5f) - 0.05f;
        glm::vec4 tint (1, 1, 1, sopacity * opacity);
        batch3d->point(glm::vec3(x, y, z), tint);
    }
    batch3d->flushPoints();
}

void Skybox::draw(
    const DrawContext& pctx,
    const Camera& camera, 
    const Assets& assets,
    float daytime,
    float fog
) {
    const glm::uvec2& viewport = pctx.getViewport();

    drawBackground(camera, assets, viewport.x, viewport.y);

    DrawContext ctx = pctx.sub();
    ctx.setBlendMode(BlendMode::Addition);

    auto shader = assets.get<ShaderProgram>("ui3d");
    shader->use();
    shader->uniformMatrix("u_projview", camera.getProjView(false));
    shader->uniformMatrix("u_apply", glm::mat4(1.0f));
    batch3d->begin();

    float angle = daytime * float(PI) * 2.0f;
    float opacity = glm::pow(1.0f - fog, 7.0f);

    for (auto& sprite : sprites) {
        batch3d->texture(assets.get<Texture>(sprite.texture));

        float sangle = daytime * float(PI) * 2.0f + sprite.phase;
        float distance = sprite.distance;

        glm::vec3 pos(-std::cos(sangle) * distance, std::sin(sangle) * distance, 0);
        glm::vec3 up(-std::sin(-sangle), std::cos(-sangle), 0.0f);
        glm::vec4 tint(1, 1, 1, opacity);
        if (!sprite.emissive) tint *= 0.6f + std::cos(angle) * 0.4;

        batch3d->sprite(pos, up, glm::vec3(0, 0, 1), 1, 1, UVRegion(), tint);
    }

    batch3d->flush();
    // drawStars(angle, opacity);
}

void Skybox::refresh(const DrawContext& parent_context, float t, float mie, uint quality) {
    frameID++;
    float dayTime = t;

    DrawContext context = parent_context.sub();
    context.setDepthMask(false);
    context.setDepthTest(false);
    context.setFramebuffer(fbo.get());
    context.setViewport({size, size});

    auto cubemap = dynamic_cast<Cubemap*>(fbo->getTexture());
    assert(cubemap != nullptr);

    ready = true;
    glActiveTexture(GL_TEXTURE1);
    cubemap->bind();
    shader.use();
    t *= PI * 2.0f;

    lightDir = glm::normalize(glm::vec3(sin(t), -cos(t), 0.0f));
    shader.uniform1i("u_quality", quality);
    shader.uniform1f("u_mie", mie);
    shader.uniform1f("u_fog", mie - 1.0f);
    shader.uniform3f("u_lightDir", lightDir);
    shader.uniform1f("u_dayTime", dayTime);

    if (glm::abs(mie - prevMie) + glm::abs(t - prevT) >= 0.01) {
        for (uint face = 0; face < 6; ++face) {
            refreshFace(face, cubemap);
        }
    } else {
        uint face = frameID % 6;
        refreshFace(face, cubemap);
    }
    prevMie = mie;
    prevT = t;

    cubemap->unbind();
    glActiveTexture(GL_TEXTURE0);
}

void Skybox::refreshFace(uint face, Cubemap* cubemap) {
    const glm::vec3 xaxs[] = {
        {0.0f, 0.0f, -1.0f},
        {0.0f, 0.0f, 1.0f},
        {-1.0f, 0.0f, 0.0f},

        {-1.0f, 0.0f, 0.0f},
        {-1.0f, 0.0f, 0.0f},
        {1.0f, 0.0f, 0.0f},
    };
    const glm::vec3 yaxs[] = {
        {0.0f, 1.0f, 0.0f},
        {0.0f, 1.0f, 0.0f},
        {0.0f, 0.0f, -1.0f},

        {0.0f, 0.0f, 1.0f},
        {0.0f, 1.0f, 0.0f},
        {0.0f, 1.0f, 0.0f},
    };

    const glm::vec3 zaxs[] = {
        {1.0f, 0.0f, 0.0f},
        {-1.0f, 0.0f, 0.0f},
        {0.0f, -1.0f, 0.0f},

        {0.0f, 1.0f, 0.0f},
        {0.0f, 0.0f, -1.0f},
        {0.0f, 0.0f, 1.0f},
    };

    glFramebufferTexture2D(
        GL_FRAMEBUFFER,
        GL_COLOR_ATTACHMENT0,
        GL_TEXTURE_CUBE_MAP_POSITIVE_X + face,
        cubemap->getId(),
        0
    );
    shader.uniform3f("u_xaxis", xaxs[face]);
    shader.uniform3f("u_yaxis", yaxs[face]);
    shader.uniform3f("u_zaxis", zaxs[face]);
    mesh->draw();
}

void Skybox::bind() const {
    glActiveTexture(GL_TEXTURE1);
    fbo->getTexture()->bind();
    glActiveTexture(GL_TEXTURE0);
}

void Skybox::unbind() const {
    glActiveTexture(GL_TEXTURE1);
    fbo->getTexture()->unbind();
    glActiveTexture(GL_TEXTURE0);
}
