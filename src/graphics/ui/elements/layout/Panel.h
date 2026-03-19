#ifndef GRAPHICS_UI_ELEMENTS_LAYOUT_PANEL_H_
#define GRAPHICS_UI_ELEMENTS_LAYOUT_PANEL_H_

#include "../commons.h"
#include "Container.h"

namespace gui {
    class Panel : public Container {
    protected:
        Orientation orientation = Orientation::Vertical;
        glm::vec4 padding {2.0f};
        float interval = 2.0f;
        int maxLength = 0;
    public:
        Panel(
            glm::vec2 size, 
            glm::vec4 padding=glm::vec4(2.0f), 
            float interval=2.0f
        );
        virtual ~Panel();

        virtual void cropToContent();

        virtual void setOrientation(Orientation orientation);
        Orientation getOrientation() const;

        virtual void add(std::shared_ptr<UINode> node) override;

        virtual void refresh() override;
        virtual void fullRefresh() override;

        virtual void setMaxLength(int value);
        int getMaxLength() const;

        virtual void setPadding(glm::vec4 padding);
        glm::vec4 getPadding() const;
    };
}

#endif // GRAPHICS_UI_ELEMENTS_LAYOUT_PANEL_H_
