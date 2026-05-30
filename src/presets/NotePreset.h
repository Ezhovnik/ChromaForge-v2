#pragma once

#include <optional>

#include <glm/vec3.hpp>
#include <glm/vec4.hpp>

#include <interfaces/Serializable.h>

enum class NoteDisplayMode {
    StaticBillboard,
    YFreeBillboard,
    XYFreeBillboard,
    Projected
};

std::string to_string(NoteDisplayMode mode);
std::optional<NoteDisplayMode> NoteDisplayMode_from(std::string_view s);

struct NotePreset : public Serializable {
    NoteDisplayMode displayMode = NoteDisplayMode::StaticBillboard;
    glm::vec4 color {1.0f};
    float scale = 1.0f;
    float renderDistance = 32.0f;
    float xrayOpacity = 0.0f;
    float perspective = 1.0f;

    dv::value serialize() const override;
    void deserialize(const dv::value& src) override;
};
