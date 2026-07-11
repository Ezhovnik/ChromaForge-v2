#include <graphics/ui/elements/InlineFrame.h>

#include <frontend/UIDocument.h>
#include <logic/scripting/scripting.h>
#include <assets/Assets.h>
#include <engine/Engine.h>
#include <graphics/ui/GUI.h>

using namespace gui;

InlineFrame::InlineFrame(GUI& gui) : Container(gui, glm::vec2(1)) {}
InlineFrame::~InlineFrame() = default;

void InlineFrame::setSrc(const std::string& src) {
    this->src = src;
    if (document) {
        scripting::on_ui_close(document.get(), nullptr);
        document = nullptr;
        root = nullptr;
    }
}

void InlineFrame::setDocument(const std::shared_ptr<UIDocument>& document) {
    clear();
    if (document == nullptr) return;
    this->document = document;
    this->root = document->getRoot();
    add(root);
    root->setSize(size);
    gui.postRunnable([this]() {
        scripting::on_ui_open(this->document.get(), {});
    });
}

void InlineFrame::activate(float delta) {
    if (document || src.empty()) return;

    const auto& assets = *gui.getEngine().getAssets();
    setDocument(assets.getShared<UIDocument>(src));
}

void InlineFrame::setSize(glm::vec2 size) {
    Container::setSize(size);
    if (root) {
        root->setSize(size);
    }
}

const std::string& InlineFrame::getSrc() const {
    return src;
}
