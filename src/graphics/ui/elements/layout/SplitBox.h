#pragma once

#include <graphics/ui/elements/layout/BasePanel.h>

namespace gui {
    class SplitBox : public BasePanel {
    public:
        SplitBox(const glm::vec2& size, float splitPos, Orientation orientation);

        virtual void mouseMove(GUI*, int x, int y) override;
        virtual void refresh() override;
        virtual void fullRefresh() override;
    private:
        float splitPos;
    };
}
