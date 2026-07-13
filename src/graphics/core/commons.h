#pragma once

#include <string>
#include <optional>

#include <util/EnumMetadata.h>

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
    EWResize,
    NSResize,

    // GLFW 3.4+ cursor shapes
    NWSEResize,
    NESWResize,
    AllResize,
    NotAllowed,

    Last=NotAllowed
};

namespace advanced_pipeline {
    inline constexpr int TARGET_COLOR = 0;
    inline constexpr int TARGET_SKYBOX = 1;
    inline constexpr int TARGET_POSITIONS = 2;
    inline constexpr int TARGET_NORMALS = 3;
    inline constexpr int TARGET_SSAO = 4;
    inline constexpr int TARGET_SHADOWS0 = 5;
    inline constexpr int TARGET_SHADOWS1 = 6;
}

CHROMA_ENUM_METADATA(CursorShape)
    {"arrow", CursorShape::Arrow},
    {"text", CursorShape::Text},
    {"crosshair", CursorShape::Crosshair},
    {"pointer", CursorShape::Pointer},
    {"ew-resize", CursorShape::EWResize},
    {"ns-resize", CursorShape::NSResize},
    {"nwse-resize", CursorShape::NWSEResize},
    {"nesw-resize", CursorShape::NESWResize},
    {"all-resize", CursorShape::AllResize},
    {"not-allowed", CursorShape::NotAllowed},
CHROMA_ENUM_END

class Flushable {
public:
    virtual ~Flushable() = default;

    virtual void flush() = 0;
};

class Bindable {
public:
    virtual ~Bindable() = default;

    virtual void bind() = 0;
    virtual void unbind() = 0;
};
