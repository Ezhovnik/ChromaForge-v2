#ifndef GRAPHICS_UI_ELEMENTS_DISPLAY_PLOTTER_H_
#define GRAPHICS_UI_ELEMENTS_DISPLAY_PLOTTER_H_

#include <memory>
#include <glm/glm.hpp>

#include "../UINode.h"
#include "../../../../typedefs.h"

class Assets;
class GfxContext;

namespace gui {
    class Plotter : public gui::UINode {
    private:
        std::unique_ptr<int[]> points;
        float multiplier;
        int index = 0;
        int dmwidth;
        int dmheight;
        int labelsInterval;
    public:
        Plotter(uint width, uint height, float multiplier, int labelsInterval) 
        : gui::UINode(glm::vec2(width, height)), 
            multiplier(multiplier),
            dmwidth(width - 50),
            dmheight(height),
            labelsInterval(labelsInterval)
        {
            points = std::make_unique<int[]>(dmwidth);
        }

        void activate(float delta) override;
        void draw(const GfxContext* pctx, Assets* assets) override;
    };
}

#endif // GRAPHICS_UI_ELEMENTS_DISPLAY_PLOTTER_H_
