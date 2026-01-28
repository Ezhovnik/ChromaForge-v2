#include "controls.h"

#include <iostream>

#include "../../assets/Assets.h"
#include "../../graphics/Batch2D.h"
#include "../../graphics/Font.h"

#define KEY_ESCAPE 256
#define KEY_ENTER 257
#define KEY_BACKSPACE 259

using namespace gui;

Label::Label(std::wstring text, std::string fontName) : UINode(glm::vec2(), glm::vec2(text.length() * 8, 15)), text_(text), fontName_(fontName) {
}

Label& Label::text(std::wstring text) {
    this->text_ = text;
    return *this;
}

std::wstring Label::text() const {
    return text_;
}

void Label::draw(Batch2D* batch, Assets* assets) {
    if (supplier) text(supplier());

    batch->color = color_;
    Font* font = assets->getFont(fontName_);
    glm::vec2 size = this->size();
    glm::vec2 newsize = glm::vec2(font->calcWidth(text_), font->lineHeight());
    if (newsize.x > size.x) {
        this->size(newsize);
        size = newsize;
    }
    glm::vec2 coord = calcCoord();
    font->draw(batch, text_, coord.x, coord.y);
}

void Label::textSupplier(wstringsupplier supplier) {
    this->supplier = supplier;
}

Button::Button(std::shared_ptr<UINode> content, glm::vec4 padding) : Panel(glm::vec2(32,32), padding, 0) {
    add(content);
}

Button::Button(std::wstring text, glm::vec4 padding) : Panel(glm::vec2(32,32), padding, 0) {
    Label* label = new Label(text);
    label->align(Align::center);
    add(std::shared_ptr<UINode>(label));
}

void Button::drawBackground(Batch2D* batch, Assets* assets) {
    glm::vec2 coord = calcCoord();
    batch->texture(nullptr);
    batch->color = (ispressed() ? pressedColor : (hover_ ? hoverColor : color_));
    batch->rect(coord.x, coord.y, size_.x, size_.y);
}

std::shared_ptr<UINode> Button::getAt(glm::vec2 pos, std::shared_ptr<UINode> self) {
    return UINode::getAt(pos, self);
}

void Button::mouseRelease(GUI* gui, int x, int y) {
    UINode::mouseRelease(gui, x, y);
    if (isInside(glm::vec2(x, y))) {
        for (auto callback : actions) {
            callback(gui);
        }
    }
}

void Button::listenAction(onaction action) {
    actions.push_back(action);
}

TextBox::TextBox(std::wstring text, glm::vec4 padding) : Panel(glm::vec2(200,32), padding, 0, false) {
    label = new Label(text);
    label->align(Align::center);
    add(std::shared_ptr<UINode>(label));
}

void TextBox::drawBackground(Batch2D* batch, Assets* assets) {
    glm::vec2 coord = calcCoord();
    batch->texture(nullptr);
    batch->color = (isfocused() ? focusedColor : (hover_ ? hoverColor : color_));
    batch->rect(coord.x, coord.y, size_.x, size_.y);
    if (!focused_ && supplier) label->text(supplier());
}

void TextBox::typed(unsigned int codepoint) {
    label->text(label->text() + std::wstring({(wchar_t)codepoint}));    
}

void TextBox::keyPressed(int key) {
    std::wstring src = label->text();
    switch (key) {
        case KEY_BACKSPACE:
            if (src.length()) label->text(src.substr(0, src.length()-1));
            break;
        case KEY_ENTER:
            if (consumer) consumer(label->text());
            defocus();
            break;
    }
}

std::shared_ptr<UINode> TextBox::getAt(glm::vec2 pos, std::shared_ptr<UINode> self) {
    return UINode::getAt(pos, self);
}

void TextBox::textSupplier(wstringsupplier supplier) {
    this->supplier = supplier;
}

void TextBox::textConsumer(wstringconsumer consumer) {
    this->consumer = consumer;
}
