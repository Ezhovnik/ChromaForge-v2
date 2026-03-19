#ifndef GRAPHICS_UI_ELEMENTS_COMMONS_H_
#define GRAPHICS_UI_ELEMENTS_COMMONS_H_

#include <functional>

namespace gui {
    enum class Orientation {
        Vertical,
        Horizontal
    };

    using ontimeout = std::function<void()>;
    struct IntervalEvent {
        ontimeout callback;
        float interval;
        float timer;
        int repeat;
    };
}

#endif // GRAPHICS_UI_ELEMENTS_COMMONS_H_
