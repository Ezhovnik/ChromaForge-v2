#pragma once

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

class Flushable {
public:
    virtual ~Flushable() = default;

    virtual void flush() = 0;
};
