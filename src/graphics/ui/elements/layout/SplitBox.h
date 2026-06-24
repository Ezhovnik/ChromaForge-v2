#pragma once

#include <graphics/ui/elements/layout/Container.h>

namespace gui {
    class SplitBox : public Container {
    public:
        SplitBox(const glm::vec2& size, float splitPos, Orientation orientation);

        virtual void mouseMove(GUI*, int x, int y) override;
        virtual void refresh() override;
        virtual void fullRefresh() override;
    private:
        float splitPos;
        int splitRadius = 2;
        Orientation orientation;
    };
}
