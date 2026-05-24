#include <presets/NotePreset.h>

#include <map>
#include <vector>

std::string to_string(NoteDisplayMode mode) {
    static std::vector<std::string> names = {
        "static_billboard",
        "y_free_billboard",
        "xy_free_billboard",
        "projected"
    };
    return names.at(static_cast<int>(mode));
}

std::optional<NoteDisplayMode> NoteDisplayMode_from(std::string_view s) {
    static std::map<std::string_view, NoteDisplayMode, std::less<>> map {
        {"static_billboard", NoteDisplayMode::StaticBillboard},
        {"y_free_billboard", NoteDisplayMode::YFreeBillboard},
        {"xy_free_billboard", NoteDisplayMode::XYFreeBillboard},
        {"projected", NoteDisplayMode::Projected}
    };
    const auto& found = map.find(s);
    if (found == map.end()) {
        return std::nullopt;
    }
    return found->second;
}

dv::value NotePreset::serialize() const {
    return dv::object({
        {"display", to_string(displayMode)}
    });
}

void NotePreset::deserialize(const dv::value& src) {
    if (src.has("display")) {
        displayMode = NoteDisplayMode_from(src["display"].asString()).value();
    }
}
