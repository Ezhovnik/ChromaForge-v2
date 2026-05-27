#pragma once

#include <unordered_map>
#include <memory>

#include <typedefs.h>

class DrawContext;
class Camera;
class Assets;
class Batch3D;
class Frustum;
class TextNote;
struct EngineSettings;

class TextsRenderer {
    Batch3D& batch;
    const Assets& assets;
    const Frustum& frustum;

    std::unordered_map<uint64_t, std::unique_ptr<TextNote>> notes;
    uint64_t nextNote = 1;

    void renderNote(
        const TextNote& note,
        const DrawContext& context,
        const Camera& camera,
        const EngineSettings& settings,
        bool hudVisible,
        bool frontLayer,
        bool projected
    );
public:
    TextsRenderer(Batch3D& batch, const Assets& assets, const Frustum& frustum);

    void render(
        const DrawContext& context,
        const Camera& camera,
        const EngineSettings& settings,
        bool hudVisible,
        bool frontLayer
    );

    uint64_t add(std::unique_ptr<TextNote> note);
    TextNote* get(uint64_t id) const;
    void remove(uint64_t id);
};
