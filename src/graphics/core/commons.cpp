#include <graphics/core/commons.h>

#include <map>

std::optional<CursorShape> CursorShape_from(std::string_view name) {
    static std::map<std::string_view, CursorShape> shapes = {
        {"arrow", CursorShape::Arrow},
        {"text", CursorShape::Text},
        {"crosshair", CursorShape::Crosshair},
        {"pointer", CursorShape::Pointer},
        {"ew-resize", CursorShape::EwResize},
        {"ns-resize", CursorShape::NsResize},
        {"nwse-resize", CursorShape::NwseResize},
        {"nesw-resize", CursorShape::NeswResize},
        {"all-resize", CursorShape::AllResize},
        {"not-allowed", CursorShape::NotAllowed}
    };
    const auto& found = shapes.find(name);
    if (found == shapes.end()) return std::nullopt;
    return found->second;
}

std::string to_string(CursorShape shape) {
    static std::string names[] = {
        "arrow",
        "text",
        "crosshair",
        "pointer",
        "ew-resize",
        "ns-resize",
        "nwse-resize",
        "nesw-resize",
        "all-resize",
        "not-allowed"
    };
    return names[static_cast<int>(shape)];
}
