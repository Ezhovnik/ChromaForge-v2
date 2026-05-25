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

TextsRenderer::TextsRenderer(
    Batch3D& batch, const Assets& assets, const Frustum& frustum
) : batch(batch), assets(assets), frustum(frustum) {
}

void TextsRenderer::renderText(
    const TextNote& note,
    const DrawContext& context,
    const Camera& camera,
    const EngineSettings& settings,
    bool hudVisible
) {
    const auto& text = note.getText();
    const auto& preset = note.getPreset();
    const auto& pos = note.getPosition();

    if (util::distance2(pos, camera.position) > util::sqr(preset.renderDistance / camera.zoom)) {
        return;
    }

    if (preset.displayMode == NoteDisplayMode::Projected) return;

    auto& font = assets.require<Font>("normal");

    glm::vec3 xvec {1, 0, 0};
    glm::vec3 yvec {0, 1, 0};

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
    }

    if (preset.displayMode != NoteDisplayMode::Projected) {
        if (!frustum.isBoxVisible(pos - xvec * (width * 0.5f), pos + xvec * (width * 0.5f))) {
            return;
        }
    }

    batch.setColor(preset.color);
    font.draw(
        batch,
        text,
        pos - xvec * (width * 0.5f) * preset.scale,
        xvec * preset.scale,
        yvec * preset.scale
    );
}

void TextsRenderer::renderTexts(
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

    for (const auto& [id, note] : notes) {
        renderText(*note, context, camera, settings, hudVisible);
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
