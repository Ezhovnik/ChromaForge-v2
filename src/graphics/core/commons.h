#pragma once

#include <string>
#include <optional>

enum class DrawPrimitive {
    Point = 0,
    Line,
    Triangle,
};

enum class BlendMode {
    Normal,
    Addition,
    Inversion
};

enum class CursorShape {
    Arrow,
    Text,
    Crosshair,
    Pointer,
    EwResize,
    NsResize,

    // GLFW 3.4+ cursor shapes
    NwseResize,
    NeswResize,
    AllResize,
    NotAllowed,

    Last=NotAllowed
};

std::optional<CursorShape> CursorShape_from(std::string_view name);
std::string to_string(CursorShape shape);

class Flushable {
public:
    virtual ~Flushable() = default;

    virtual void flush() = 0;
};
