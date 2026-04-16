#ifndef GRAPHICS_CORE_COMMONS_H_
#define GRAPHICS_CORE_COMMONS_H_

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

#endif // GRAPHICS_CORE_COMMONS_H_
