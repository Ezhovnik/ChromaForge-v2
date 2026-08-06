#pragma once

#include <graphics/ui/elements/Container.h>

class UIDocument;

namespace gui {
    class InlineFrame final : public Container {
    public:
        explicit InlineFrame(GUI& gui);
        virtual ~InlineFrame();

        void setSrc(const std::string& src);
        void setDocument(const std::shared_ptr<UIDocument>& document);

        void activate(float delta) override;
        void setSize(const glm::vec2& size) override;

        const std::string& getSrc() const;
    private:
        std::string src;
        std::shared_ptr<UIDocument> document;
        std::shared_ptr<UINode> root;
    };
}
