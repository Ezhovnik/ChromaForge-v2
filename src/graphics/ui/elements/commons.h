#pragma once

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
