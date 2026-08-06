#pragma once

#include <functional>

namespace gui {
    enum class Orientation {
        Vertical,
        Horizontal
    };

    using OnTimeOut = std::function<void()>;
    struct IntervalEvent {
        OnTimeOut callback;
        float interval;
        float timer;
        int repeat;
    };
}
