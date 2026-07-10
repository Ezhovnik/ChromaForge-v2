#pragma once

#include <string>

#include <glm/vec3.hpp>
#include <glm/vec4.hpp>

#include <interfaces/Serializable.h>
#include <util/EnumMetadata.h>

enum class NoteDisplayMode {
    StaticBillboard,
    YFreeBillboard,
    XYFreeBillboard,
    Projected
};

CHROMA_ENUM_METADATA(NoteDisplayMode)
    {"static_billboard", NoteDisplayMode::StaticBillboard},
    {"y_free_billboard", NoteDisplayMode::YFreeBillboard},
    {"xy_free_billboard", NoteDisplayMode::XYFreeBillboard},
    {"projected", NoteDisplayMode::Projected},
CHROMA_ENUM_END

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
