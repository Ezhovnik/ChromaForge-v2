#include <graphics/core/PostProcessing.h>

#include <stdexcept>

#include <graphics/core/Mesh.h>
#include <graphics/core/ShaderProgram.h>
#include <graphics/core/Texture.h>
#include <graphics/core/Framebuffer.h>
#include <debug/Logger.h>
#include <graphics/core/DrawContext.h>
#include <graphics/core/PostEffect.h>
#include <assets/Assets.h>

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
}

PostProcessing::~PostProcessing() = default;

void PostProcessing::use(DrawContext& context) {
    const auto& vp = context.getViewport();
    if (fbo) {
        fbo->resize(vp.x, vp.y);
        fboSecond->resize(vp.x, vp.y);
    } else {
        fbo = std::make_unique<Framebuffer>(vp.x, vp.y);
        fboSecond = std::make_unique<Framebuffer>(vp.x, vp.y);
    }
    context.setFramebuffer(fbo.get());
}

void PostProcessing::render(
    const DrawContext& context, const Assets& assets, float timer
) {
    if (fbo == nullptr) {
        LOG_ERROR("'use(...)' was never called");
        throw std::runtime_error("'use(...)' was never called");
    }

    int totalPasses = 0;
    for (const auto& effect : effectSlots) {
        totalPasses += (effect != nullptr && effect->isActive());
    }

    if (totalPasses == 0) {
        auto& effect = assets.require<PostEffect>("default");
        effect.use();
        fbo->getTexture()->bind();
        quadMesh->draw();
        return;
    }

    int currentPass = 1;
    for (const auto& effect : effectSlots) {
        if (effect == nullptr || !effect->isActive()) {
            continue;
        }
        auto& shader = effect->use();

        const auto& viewport = context.getViewport();
        shader.uniform1i("u_screen", 0);
        shader.uniform2i("u_screenSize", viewport);
        shader.uniform1f("u_timer", timer);

        fbo->getTexture()->bind();
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
