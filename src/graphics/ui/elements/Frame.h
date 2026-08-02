#pragma once

#include <graphics/ui/elements/Container.h>

class Framebuffer;
class UIDocument;

namespace gui {
    class Frame final : public Container {
    public:
        Frame(
            GUI& gui,
            std::string id,
            std::string outputTexture
        );
        virtual ~Frame();

        void draw(
            const DrawContext& pctx,
            const Assets& assets
        ) override;

        void updateOutput(Assets& assets);

        const std::string& getOutputTexture() const;

        const std::string& getFrameId() const;
    private:
        std::string frameId;
        std::unique_ptr<Framebuffer> fbo;
        std::string outputTexture;
    };
}
