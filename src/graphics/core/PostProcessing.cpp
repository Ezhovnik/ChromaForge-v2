#include <graphics/core/PostProcessing.h>

#include <stdexcept>
#include <random>

#include <graphics/core/Mesh.h>
#include <graphics/core/ShaderProgram.h>
#include <graphics/core/Texture.h>
#include <graphics/core/Framebuffer.h>
#include <debug/Logger.h>
#include <graphics/core/DrawContext.h>
#include <graphics/core/PostEffect.h>
#include <assets/Assets.h>
#include <window/Camera.h>
#include <graphics/core/GBuffer.h>

PostProcessing::PostProcessing(size_t effectSlotsCount) : effectSlots(effectSlotsCount) {
    PostProcessingVertex meshData[]{
        {{-1.0f, -1.0f}},
        {{-1.0f, 1.0f}},
        {{1.0f, 1.0f}},
        {{-1.0f, -1.0f}},
        {{1.0f, 1.0f}},
        {{1.0f, -1.0f}},
    };
    quadMesh = std::make_unique<Mesh<PostProcessingVertex>>(meshData, 6);

    std::vector<glm::vec3> ssaoNoise;
    for (unsigned int i = 0; i < 16; ++i) {
        glm::vec3 noise(
            (rand() / static_cast<float>(RAND_MAX)) * 2.0 - 1.0, 
            (rand() / static_cast<float>(RAND_MAX)) * 2.0 - 1.0, 
            0.0f); 
        ssaoNoise.push_back(noise);
    }  
    glGenTextures(1, &noiseTexture);
    glBindTexture(GL_TEXTURE_2D, noiseTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, 4, 4, 0, GL_RGB, GL_FLOAT, ssaoNoise.data());
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glBindTexture(GL_TEXTURE_2D, 0);
}

PostProcessing::~PostProcessing() = default;

void PostProcessing::use(DrawContext& context, bool gbufferPipeline) {
    const auto& vp = context.getViewport();
    if (gbufferPipeline) {
        if (gbuffer == nullptr) {
            gbuffer = std::make_unique<GBuffer>(vp.x, vp.y);
        } else {
            gbuffer->resize(vp.x, vp.y);
        }
        context.setFramebuffer(gbuffer.get());
    } else {
        gbuffer.reset();
        refreshFbos(vp.x, vp.y);
        context.setFramebuffer(fbo.get());
    }
}

void PostProcessing::refreshFbos(uint width, uint height) {
    if (fbo) {
        fbo->resize(width, height);
        fboSecond->resize(width, height);
    } else {
        fbo = std::make_unique<Framebuffer>(width, height);
        fboSecond = std::make_unique<Framebuffer>(width, height);
    }
}

void PostProcessing::bindDepthBuffer() {
    if (gbuffer) {
        gbuffer->bindDepthBuffer(fbo->getFBO());
    }
}

void PostProcessing::configureEffect(
    const DrawContext& context,
    PostEffect& effect,
    ShaderProgram& shader,
    float timer,
    const Camera& camera
) {
    const auto& viewport = context.getViewport();

    shader.uniform1i("u_screen", advanced_pipeline::TARGET_COLOR);
    shader.uniform1i("u_skybox", advanced_pipeline::TARGET_SKYBOX);
    if (gbuffer) {
        shader.uniform1i("u_position", advanced_pipeline::TARGET_POSITIONS);
        shader.uniform1i("u_normal", advanced_pipeline::TARGET_NORMALS);
        shader.uniform1i("u_emission", advanced_pipeline::TARGET_EMISSION);
    }
    shader.uniform1i("u_noise", advanced_pipeline::TARGET_SSAO);
    shader.uniform1i("u_ssao", advanced_pipeline::TARGET_SSAO);

    shader.uniform2i("u_screenSize", viewport);
    shader.uniform3f("u_cameraPos", camera.position);
    shader.uniform1f("u_timer", timer);
    shader.uniformMatrix("u_projection", camera.getProjection());
    shader.uniformMatrix("u_view", camera.getView());
    shader.uniformMatrix("u_inverseView", glm::inverse(camera.getView()));
}

void PostProcessing::renderDeferredShading(
    const DrawContext& context,
    const Assets& assets,
    float timer,
    const Camera& camera
) {
    if (gbuffer == nullptr) {
        LOG_ERROR("GBuffer is not initialized");
        throw std::runtime_error("GBuffer is not initialized");
    }

    gbuffer->bindBuffers();

    glActiveTexture(GL_TEXTURE0 + advanced_pipeline::TARGET_SSAO);
    glBindTexture(GL_TEXTURE_2D, noiseTexture);

    glActiveTexture(GL_TEXTURE0);

    auto& ssaoEffect = assets.require<PostEffect>("ssao");
    auto& shader = ssaoEffect.use();
    configureEffect(
        context,
        ssaoEffect,
        shader,
        timer,
        camera
    );
    gbuffer->bindSSAO();
    quadMesh->draw();
    gbuffer->unbind();

    {
        auto viewport = context.getViewport();
        refreshFbos(viewport.x, viewport.y);

        auto ctx = context.sub();
        ctx.setFramebuffer(fbo.get());

        glActiveTexture(GL_TEXTURE0 + advanced_pipeline::TARGET_SSAO);
        gbuffer->bindSSAOBuffer();

        glActiveTexture(GL_TEXTURE0);

        gbuffer->bindBuffers();

        auto& effect = assets.require<PostEffect>("deferred_lighting");
        auto& shader = effect.use();
        configureEffect(
            context,
            effect,
            shader,
            timer,
            camera
        );
        quadMesh->draw();
    }
}

void PostProcessing::render(
    const DrawContext& context,
    const Assets& assets,
    float timer,
    const Camera& camera
) {
    if (fbo == nullptr) {
        LOG_ERROR("'use(...)' was never called");
        throw std::runtime_error("'use(...)' was never called");
    }
    int totalPasses = 0;
    for (const auto& effect : effectSlots) {
        totalPasses +=
            (effect != nullptr && effect->isActive() && !(effect->isAdvanced() && gbuffer == nullptr));
    }

    const auto& vp = context.getViewport();
    refreshFbos(vp.x, vp.y);

    glActiveTexture(GL_TEXTURE0);
    fbo->getTexture()->bind();

    if (totalPasses == 0) {
        auto& effect = assets.require<PostEffect>("default");
        auto& shader = effect.use();
        configureEffect(
            context, effect, shader, timer, camera
        );
        quadMesh->draw();
        return;
    }

    int currentPass = 1;
    for (const auto& effect : effectSlots) {
        if (effect == nullptr || !effect->isActive()) {
            continue;
        }
        if (effect->isAdvanced() && gbuffer == nullptr) {
            continue;
        }
        auto& shader = effect->use();
        configureEffect(
            context,
            *effect,
            shader,
            timer,
            camera
        );

        if (currentPass > 1) {
            fbo->getTexture()->bind();
        }
        if (currentPass < totalPasses) {
            fboSecond->bind();
        }

        quadMesh->draw();
        if (currentPass < totalPasses) {
            fboSecond->unbind();
            std::swap(fbo, fboSecond);
        }
        currentPass++;
    }
}

void PostProcessing::setEffect(size_t slot, std::shared_ptr<PostEffect> effect) {
    effectSlots.at(slot) = std::move(effect);
}

PostEffect* PostProcessing::getEffect(size_t slot) {
    return effectSlots.at(slot).get();
}

std::unique_ptr<ImageData> PostProcessing::toImage() {
    return fbo->getTexture()->readData();
}

Framebuffer* PostProcessing::getFramebuffer() const {
    return fbo.get();
}
