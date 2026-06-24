#pragma once

#include <graphics/ui/elements/commons.h>
#include <graphics/ui/elements/layout/Container.h>

namespace gui {
    class Panel : public Container {
    protected:
        Orientation orientation = Orientation::Vertical;
        glm::vec4 padding;
        float interval = 2.0f;
        int minLength = 0;
        int maxLength = 0;
    public:
        Panel(
            glm::vec2 size, 
            glm::vec4 padding=glm::vec4(0.0f), 
            float interval=2.0f
        );
        virtual ~Panel();

        virtual void cropToContent();

        virtual void setOrientation(Orientation orientation);
        Orientation getOrientation() const;

        virtual void add(const std::shared_ptr<UINode>& node) override;
        virtual void remove(UINode* node) override;

        virtual void refresh() override;
        virtual void fullRefresh() override;

        virtual void setMinLength(int value);
        int getMinLength() const;

        virtual void setMaxLength(int value);
        int getMaxLength() const;

        virtual void setPadding(glm::vec4 padding);
        glm::vec4 getPadding() const;
    };
}
