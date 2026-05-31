#include <graphics/render/TextsRenderer.h>

#include <graphics/render/TextNote.h>
#include <math/util.h>
#include <assets/Assets.h>
#include <window/Camera.h>
#include <math/FrustumCulling.h>
#include <graphics/core/Font.h>
#include <graphics/core/Batch3D.h>
#include <graphics/core/ShaderProgram.h>
#include <presets/NotePreset.h>
#include <window/Window.h>

TextsRenderer::TextsRenderer(
    Batch3D& batch, const Assets& assets, const Frustum& frustum
) : batch(batch), assets(assets), frustum(frustum) {
}

void TextsRenderer::renderNote(
    const TextNote& note,
    const DrawContext& context,
    const Camera& camera,
    const EngineSettings& settings,
    bool hudVisible,
    bool frontLayer,
    bool projected
) {
    const auto& text = note.getText();
    const auto& preset = note.getPreset();
    auto pos = note.getPosition();

    if (util::distance2(pos, camera.position) > util::sqr(preset.renderDistance / camera.zoom)) {
        return;
    }

    if ((preset.displayMode == NoteDisplayMode::Projected) != projected) return;

    float opacity = 1.0f;
    if (frontLayer) {
        if (preset.xrayOpacity <= 0.0001f) return;
        opacity = preset.xrayOpacity;
    }
    const auto& font = assets.require<Font>("normal");

    glm::vec3 xvec = note.getAxisX();
    glm::vec3 yvec = note.getAxisY();

    int width = font.calcWidth(text, text.length());
    if (preset.displayMode == NoteDisplayMode::YFreeBillboard || preset.displayMode == NoteDisplayMode::XYFreeBillboard) {
        xvec = camera.position - pos;
        xvec.y = 0;
        std::swap(xvec.x, xvec.z);
        xvec.z *= -1;
        xvec = glm::normalize(xvec);
        if (preset.displayMode == NoteDisplayMode::XYFreeBillboard) {
            yvec = camera.up;
        }
        float scale = (1.0f - preset.perspective) * glm::pow(glm::distance(camera.position, pos), 1.0f-preset.perspective);
        xvec *= 1.0f + scale;
        yvec *= 1.0f + scale;
    }

    if (preset.displayMode == NoteDisplayMode::Projected) {
        float scale = 1.0f;
        if (glm::abs(preset.perspective) > 0.0001f) {
            float scale2 = scale / (glm::distance(camera.position, pos) * util::sqr(camera.zoom) * glm::sqrt(glm::tan(camera.getFov() * 0.5f)));
            scale = scale2 * preset.perspective + scale * (1.0f - preset.perspective);
        }
        if (frontLayer) {
            auto projpos = camera.getProjView() * glm::vec4(pos, 1.0f);
            pos = projpos;
            if (pos.z < 0.0f) return;
            pos /= projpos.w;
            pos.z = 0;
            xvec = {2.0f/Window::width * scale, 0, 0};
            yvec = {0, 2.0f/Window::height * scale, 0};
        } else {
            auto matrix = camera.getProjView();
            auto screenPos = matrix * glm::vec4(pos, 1.0f);

            xvec = glm::vec3(2.0f/Window::width * scale, 0, 0);
            yvec = glm::vec3(0, 2.0f/Window::height * scale, 0);

            pos = screenPos / screenPos.w;
        }
    } else if (!frustum.isBoxVisible(pos - xvec * (width * 0.5f * preset.scale), pos + xvec * (width * 0.5f * preset.scale))) {
        return;
    }

    static FontStylesScheme styles {};
    auto color = preset.color;
    batch.setColor(glm::vec4(color.r, color.g, color.b, color.a * opacity));
    font.draw(
        batch,
        text,
        &styles,
        pos - xvec * (width * 0.5f) * preset.scale,
        xvec * preset.scale,
        yvec * preset.scale
    );
}

void TextsRenderer::render(
    const DrawContext& context,
    const Camera& camera,
    const EngineSettings& settings,
    bool hudVisible,
    bool frontLayer
) {
    auto& shader = assets.require<ShaderProgram>("ui3d");

    shader.use();
    shader.uniformMatrix("u_projview", camera.getProjView());
    shader.uniformMatrix("u_apply", glm::mat4(1.0f));
    batch.begin();

    for (const auto& [_, note] : notes) {
        renderNote(*note, context, camera, settings, hudVisible, frontLayer, false);
    }

    batch.flush();
    shader.uniformMatrix("u_projview", glm::mat4(1.0f));
    for (const auto& [_, note] : notes) {
        renderNote(*note, context, camera, settings, hudVisible, frontLayer, true);
    }
    batch.flush();
}

uint64_t TextsRenderer::add(std::unique_ptr<TextNote> note) {
    uint64_t uid = nextNote++;
    notes[uid] = std::move(note);
    return uid;
}

TextNote* TextsRenderer::get(uint64_t id) const {
    const auto& found = notes.find(id);
    if (found == notes.end()) {
        return nullptr;
    }
    return found->second.get();
}

void TextsRenderer::remove(uint64_t id) {
    notes.erase(id);
}
