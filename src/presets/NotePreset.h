#pragma once

#include <glm/vec3.hpp>
#include <optional>

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

    dv::value serialize() const override;
    void deserialize(const dv::value& src) override;
};
