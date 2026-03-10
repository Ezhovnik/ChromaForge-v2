#include "controls.h"

#include "../../assets/Assets.h"
#include "../../graphics/Batch2D.h"
#include "../../graphics/Font.h"
#include "../../util/stringutil.h"
#include "../../window/Events.h"
#include "GUI.h"
#include "../../graphics/UVRegion.h"
#include "../../graphics/GfxContext.h"
#include "../../graphics/Texture.h"

using namespace gui;

Label::Label(std::string text, std::string fontName) : UINode(glm::vec2(), glm::vec2(text.length() * 8, 15)), text(util::str2wstr_utf8(text)), fontName(fontName) {
    setInteractive(false);
}

Label::Label(std::wstring text, std::string fontName) : UINode(glm::vec2(), glm::vec2(text.length() * 8, 15)), text(text), fontName(fontName) {
    setInteractive(false);
}

void Label::setText(std::wstring text) {
    this->text = text;
}

std::wstring Label::getText() const {
    return text;
}

void Label::draw(const GfxContext* parent_context, Assets* assets) {
    if (supplier) setText(supplier());

    auto batch = parent_context->getBatch2D();
    batch->color = getColor();
    Font* font = assets->getFont(fontName);
    glm::vec2 size = getSize();
    glm::vec2 newsize = glm::vec2(
        font->calcWidth(text), 
        font->getLineHeight()+font->getYOffset()
    );

    glm::vec2 coord = calcCoord();

    switch (align) {
        case Align::left:
            break;
        case Align::center:
            coord.x += (size.x - newsize.x) * 0.5f;
            break;
        case Align::right:
            coord.x += size.x - newsize.x;
            break;
    }

    coord.y += (size.y - newsize.y) * 0.5f;
    font->draw(batch, text, coord.x, coord.y);
}

void Label::textSupplier(wstringsupplier supplier) {
    this->supplier = supplier;
}

Image::Image(std::string texture, glm::vec2 size) : UINode(glm::vec2(), size), texture(texture) {
    setInteractive(false);
}

void Image::draw(const GfxContext* parent_context, Assets* assets) {
    glm::vec2 coord = calcCoord();
    glm::vec4 color = getColor();
    auto batch = parent_context->getBatch2D();

    auto texture = assets->getTexture(this->texture);
    if (texture && autoresize) setSize(glm::vec2(texture->width, texture->height));
    batch->texture(texture);

    batch->color = color;
    batch->rect(coord.x, coord.y, size.x, size.y, 0, 0, 0, UVRegion(), false, true, color);
}

void Image::setAutoResize(bool flag) {
    autoresize = flag;
}
bool Image::isAutoResize() const {
    return autoresize;
}

Button::Button(std::shared_ptr<UINode> content, glm::vec4 padding) : Panel(glm::vec2(), padding, 0) {
    glm::vec4 margin = getMargin();
    setSize(content->getSize() + glm::vec2(padding[0] + padding[2] + margin[0] + margin[2], padding[1] + padding[3] + margin[1] + margin[3]));
    add(content);
    setScrollable(false);
    setHoverColor(glm::vec4(0.05f, 0.1f, 0.15f, 0.75f));
    content->setInteractive(false);
}

Button::Button(std::wstring text, glm::vec4 padding, onaction action, glm::vec2 size) : Panel(size, padding, 0) {
    if (size.y < 0.0f) {
        size = glm::vec2(
            glm::max(padding.x + padding.z + text.length()*8, size.x),
            glm::max(padding.y + padding.w + 16, size.y)
        );
    }
    setSize(size);

    if (action) listenAction(action);
    setScrollable(false);

    label = std::make_shared<Label>(text);
    label->setAlign(Align::center);
    label->setSize(size - glm::vec2(padding.z + padding.x, padding.w + padding.y));
    label->setInteractive(false);
    add(label);

    setHoverColor(glm::vec4(0.05f, 0.1f, 0.15f, 0.75f));
}

void Button::setText(std::wstring text) {
    if (label) label->setText(text);
}

std::wstring Button::getText() const {
    if (label) return label->getText();
    return L"";
}

Button* Button::textSupplier(wstringsupplier supplier) {
    if (label) {
        Label* label = (Label*)(this->label.get());
        label->textSupplier(supplier);
    }
    return this;
}

void Button::drawBackground(const GfxContext* parent_context, Assets* assets) {
    glm::vec2 coord = calcCoord();
    auto batch = parent_context->getBatch2D();
    batch->texture(nullptr);
    batch->color = (isPressed() ? pressedColor : (hover ? hoverColor : color));
    batch->rect(coord.x, coord.y, size.x, size.y);
}

void Button::mouseRelease(GUI* gui, int x, int y) {
    UINode::mouseRelease(gui, x, y);
    if (isInside(glm::vec2(x, y))) {
        for (auto callback : actions) {
            callback(gui);
        }
    }
}

Button* Button::listenAction(onaction action) {
    actions.push_back(action);
    return this;
}

void Button::setTextAlign(Align align) {
    if (label) {
        label->setAlign(align);
        refresh();
    }
}

Align Button::getTextAlign() const {
    if (label) return label->getAlign();
    return Align::left;
}

void Button::refresh() {
    Panel::refresh();
    if (label) label->setSize(size - glm::vec2(padding.z + padding.x, padding.w + padding.y));
}

glm::vec4 Button::getPressedColor() const {
    return pressedColor;
}

void Button::setPressedColor(glm::vec4 color) {
    pressedColor = color;
}

RichButton::RichButton(glm::vec2 size) : Container(glm::vec2(), size) {
    setHoverColor(glm::vec4(0.05f, 0.1f, 0.15f, 0.75f));
}

void RichButton::mouseRelease(GUI* gui, int x, int y) {
    UINode::mouseRelease(gui, x, y);
    if (isInside(glm::vec2(x, y))) {
        for (auto callback : actions) {
            callback(gui);
        }
    }
}

RichButton* RichButton::listenAction(onaction action) {
    actions.push_back(action);
    return this;
}

void RichButton::drawBackground(const GfxContext* parent_context, Assets* assets) {
    glm::vec2 coord = calcCoord();
    auto batch = parent_context->getBatch2D();
    batch->texture(nullptr);
    batch->color = (isPressed() ? pressedColor : (hover ? hoverColor : color));
    batch->rect(coord.x, coord.y, size.x, size.y);
}

TextBox::TextBox(std::wstring placeholder, glm::vec4 padding) : Panel(glm::vec2(200, 32), padding, 0), input(L""), placeholder(placeholder) {
    label = std::make_shared<Label>(L"");
    label->setSize(size - glm::vec2(padding.z + padding.x, padding.w + padding.y));
    add(label);
    setHoverColor(glm::vec4(0.05f, 0.1f, 0.2f, 0.75f));
}

void TextBox::drawBackground(const GfxContext* parent_context, Assets* assets) {
    glm::vec2 coord = calcCoord();

    auto batch = parent_context->getBatch2D();
    batch->texture(nullptr);

    if (valid) {
        if (isFocused()) batch->color = focusedColor;
        else if (hover) batch->color = hoverColor;
        else batch->color = color;
    } else {
        batch->color = invalidColor;
    }

    batch->rect(coord.x, coord.y, size.x, size.y);
    if (!isFocused() && supplier) input = supplier();

    if (input.empty()) {
        label->setColor(glm::vec4(0.5f));
        label->setText(placeholder);
    } else {
        label->setColor(glm::vec4(1.0f));
        label->setText(input);
    }

    setScrollable(false);
}

void TextBox::typed(unsigned int codepoint) {
    input += std::wstring({(wchar_t)codepoint});
    validate();
}

bool TextBox::validate() {
    if (validator) {
        valid = validator(getText());
    } else {
        valid = true;
    }

    return valid;
}

void TextBox::setValid(bool valid) {
    this->valid = valid;
}

bool TextBox::isValid() const {
    return valid;
}

void TextBox::keyPressed(int key) {
    if (key == keycode::BACKSPACE) {
        if (!input.empty()) {
            input = input.substr(0, input.length() - 1);
            validate();
        }
    } else if (key == keycode::ENTER) {
        if (validate() && consumer) consumer(label->getText());
        defocus();
    }

    if (key == keycode::V && Events::isPressed(keycode::LEFT_CONTROL)) {
        const char* text = Window::getClipboardText();
        if (text) {
            input += util::str2wstr_utf8(text);
            validate();
        }
    }
}

void TextBox::setOnEditStart(runnable oneditstart) {
    onEditStart = oneditstart;
}

void TextBox::focus(GUI* gui) {
    Panel::focus(gui);
    if (onEditStart) onEditStart();
}

std::shared_ptr<UINode> TextBox::getAt(glm::vec2 pos, std::shared_ptr<UINode> self) {
    return UINode::getAt(pos, self);
}

void TextBox::setTextSupplier(wstringsupplier supplier) {
    this->supplier = supplier;
}

void TextBox::setTextConsumer(wstringconsumer consumer) {
    this->consumer = consumer;
}

void TextBox::setTextValidator(wstringchecker validator) {
    this->validator = validator;
}

std::wstring TextBox::getText() const {
    if (input.empty()) return placeholder;
    return input;
}

void TextBox::setText(std::wstring value) {
    this->input = value;
}

void TextBox::refresh() {
    Panel::refresh();
    label->setSize(size - glm::vec2(padding.z + padding.x, padding.w + padding.y));
}

InputBindBox::InputBindBox(Binding& binding, glm::vec4 padding) : Panel(glm::vec2(100, 32), padding, 0), binding(binding) {
    label = std::make_shared<Label>(L"");
    add(label);
    setScrollable(false);
}

void InputBindBox::drawBackground(const GfxContext* parent_context, Assets* assets) {
    glm::vec2 coord = calcCoord();
    auto batch = parent_context->getBatch2D();
    batch->texture(nullptr);
    batch->color = (isFocused() ? focusedColor : (hover ? hoverColor : color));
    batch->rect(coord.x, coord.y, size.x, size.y);
    label->setText(util::str2wstr_utf8(binding.text()));
}

void InputBindBox::clicked(GUI*, int button) {
    binding.type = inputType::mouse;
    binding.code = button;
    defocus();
}

void InputBindBox::keyPressed(int key) {
    if (key != keycode::ESCAPE) {
        binding.type = inputType::keyboard;
        binding.code = key;
    }
    defocus();
}

TrackBar::TrackBar(double min, double max, double value, double step, int trackWidth)
    : UINode(glm::vec2(), glm::vec2(26)), min(min), max(max), value(value), step(step), trackWidth(trackWidth) {
    setColor(glm::vec4(0.0f, 0.0f, 0.0f, 0.4f));
    setHoverColor(glm::vec4(0.01f, 0.02f, 0.03f, 0.5f));
}

void TrackBar::draw(const GfxContext* parent_context, Assets* assets) {
    if (supplier) value = supplier();

    glm::vec2 coord = calcCoord();
    auto batch = parent_context->getBatch2D();
    batch->texture(nullptr);
    batch->color = (hover ? hoverColor : color);
    batch->rect(coord.x, coord.y, size.x, size.y);

    float width = size.x;
    float t = (value - min) / (max - min + trackWidth * step);

    batch->color = trackColor;
    int actualWidth = size.x * (trackWidth / (max - min + trackWidth * step) * step);
    batch->rect(coord.x + width * t, coord.y, actualWidth, size.y);
}

void TrackBar::setSupplier(doublesupplier supplier_) {
    supplier = supplier_;
}

void TrackBar::setConsumer(doubleconsumer consumer_) {
    consumer = consumer_;
}

void TrackBar::mouseMove(GUI*, int x, int y) {
    glm::vec2 coord = calcCoord();
    value = x;
    value -= coord.x;
    value = (value) / size.x * (max - min + trackWidth * step);
    value += min;
    value = (value > max) ? max : value;
    value = (value < min) ? min : value;
    value = (int)(value / step) * step;
    if (consumer) consumer(value);
}

double TrackBar::getValue() const {
    return value;
}

double TrackBar::getMin() const {
    return min;
}

double TrackBar::getMax() const {
    return max;
}

double TrackBar::getStep() const {
    return step;
}

int TrackBar::getTrackWidth() const {
    return trackWidth;
}

glm::vec4 TrackBar::getTrackColor() const {
    return trackColor;
}

void TrackBar::setValue(double value_) {
    value = value_;
}

void TrackBar::setMin(double min_) {
    min = min_;
}

void TrackBar::setMax(double max_) {
    max = max_;
}

void TrackBar::setStep(double step_) {
    step = step_;
}

void TrackBar::setTrackWidth(int trackWidth_) {
    trackWidth = trackWidth_;
}

void TrackBar::setTrackColor(glm::vec4 trackColor_) {
    trackColor = trackColor_;
}

CheckBox::CheckBox(bool checked) : UINode(glm::vec2(), glm::vec2(32.0f)), checked(checked) {
    setColor(glm::vec4(0.0f, 0.0f, 0.0f, 0.5f));
}

void CheckBox::draw(const GfxContext* parent_context, Assets* assets) {
    if (supplier) checked = supplier();

    glm::vec2 coord = calcCoord();
    auto batch = parent_context->getBatch2D();
    batch->texture(nullptr);
    batch->color = checked ? checkColor : (hover ? hoverColor : color);
    batch->rect(coord.x, coord.y, size.x, size.y);
}

void CheckBox::mouseRelease(GUI*, int x, int y) {
    checked = !checked;
    if (consumer) consumer(checked);
}

void CheckBox::setSupplier(boolsupplier supplier_) {
    supplier = supplier_;
}

void CheckBox::setConsumer(boolconsumer consumer_) {
    consumer = consumer_;
}

CheckBox* CheckBox::setChecked(bool flag) {
    checked = flag;
    return this;
}

FullCheckBox::FullCheckBox(std::wstring text, glm::vec2 size, bool checked) : Panel(size), checkbox(std::make_shared<CheckBox>(checked)){
    setColor(glm::vec4(0.0f));
    setOrientation(Orientation::horizontal);

    add(checkbox);

    auto label = std::make_shared<Label>(text); 
    label->setMargin(glm::vec4(5.0f, 5.0f, 0.0f, 0.0f));
    add(label);
}
