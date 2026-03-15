#include "controls.h"

#include <queue>
#include <sstream>
#include <algorithm>

#include "../../../assets/Assets.h"
#include "../../../graphics/core/Batch2D.h"
#include "../../../graphics/core/Font.h"
#include "../../../util/stringutil.h"
#include "../../../window/Events.h"
#include "../GUI.h"
#include "../../../math/UVRegion.h"
#include "../../../graphics/core/GfxContext.h"
#include "../../../graphics/core/Texture.h"

using namespace gui;

Label::Label(std::string text, std::string fontName) : UINode(glm::vec2(text.length() * 8, 15)), text(util::str2wstr_utf8(text)), fontName(fontName) {
    setInteractive(false);
}

Label::Label(std::wstring text, std::string fontName) : UINode(glm::vec2(text.length() * 8, 15)), text(text), fontName(fontName) {
    setInteractive(false);
}

void Label::setText(std::wstring text) {
    this->text = text;
    lines = 1;
    for (size_t i = 0; i < text.length(); ++i) {
        if (text[i] == L'\n') lines++;
    }
    lines = std::max(lines, 1U);
}

const std::wstring& Label::getText() const {
    return text;
}

void Label::setVerticalAlign(Align align) {
    this->valign = align;
}

Align Label::getVerticalAlign() const {
    return valign;
}

float Label::getLineInterval() const {
    return lineInterval;
}

void Label::setLineInterval(float interval) {
    lineInterval = interval;
}

int Label::getTextYOffset() const {
    return textYOffset;
}

int Label::getLineYOffset(uint line) const {
    return line * totalLineHeight + textYOffset;
}

void Label::draw(const GfxContext* parent_context, Assets* assets) {
    if (supplier) setText(supplier());

    auto batch = parent_context->getBatch2D();
    Font* font = assets->getFont(fontName);

    batch->setColor(getColor());

    uint lineHeight = font->getLineHeight();
    glm::vec2 size = getSize();
    glm::vec2 newsize = glm::vec2(
        font->calcWidth(text), 
        (lines == 1 ? lineHeight : lineHeight * lineInterval) * lines + font->getYOffset()
    );

    glm::vec2 pos = calcPos();

    switch (align) {
        case Align::left:
            break;
        case Align::center:
            pos.x += (size.x - newsize.x) * 0.5f;
            break;
        case Align::right:
            pos.x += size.x - newsize.x;
            break;
    }

    switch (valign) {
        case Align::top:
            break;
        case Align::center:
            pos.y += (size.y - newsize.y) * 0.5f;
            break;
        case Align::bottom:
            pos.y += size.y - newsize.y;
            break;
    }
    textYOffset = pos.y - calcPos().y;
    totalLineHeight = lineHeight * lineInterval;

    if (multiline) {
        size_t offset = 0;
        for (uint i = 0; i < lines; ++i) {
            std::wstring_view view(text.c_str() + offset, text.length() - offset);
            size_t end = view.find(L'\n');
            if (end != std::wstring::npos) {
                view = std::wstring_view(text.c_str() + offset, end);
                offset += end + 1;
            }
            font->draw(batch, view, pos.x, pos.y + i * totalLineHeight, FontStyle::None);
        }
    } else {
        font->draw(batch, text, pos.x, pos.y, FontStyle::None);
    }
}

void Label::textSupplier(wstringsupplier supplier) {
    this->supplier = supplier;
}

void Label::setFontName(std::string name) {
    this->fontName = name;
}

const std::string& Label::getFontName() const {
    return fontName;
}

void Label::setMultiline(bool multiline) {
    this->multiline = multiline;
}

bool Label::isMultiline() const {
    return multiline;
}

size_t Label::getTextLineOffset(uint line) const {
    size_t offset = 0;
    size_t linesCount = 0;
    while (linesCount < line && offset < text.length()) {
        size_t endline = text.find(L'\n', offset);
        if (endline == std::wstring::npos) break;
        offset = endline + 1;
        linesCount++;
    }
    return offset;
}

uint Label::getLineByYOffset(int offset) const {
    if (offset < textYOffset) return 0;
    return (offset - textYOffset) / totalLineHeight;
}

uint Label::getLineByTextIndex(size_t index) const {
    size_t offset = 0;
    size_t linesCount = 0;
    while (offset < index && offset < text.length()) {
        size_t endline = text.find(L'\n', offset);
        if (endline == std::wstring::npos) break;
        if (endline+1 > index) break;
        offset = endline + 1;
        linesCount++;
    }
    return linesCount;
}

uint Label::getLinesNumber() const {
    return lines;
}

Image::Image(std::string texture, glm::vec2 size) : UINode(size), texture(texture) {
    setInteractive(false);
}

void Image::draw(const GfxContext* parent_context, Assets* assets) {
    glm::vec2 pos = calcPos();
    glm::vec4 color = getColor();
    auto batch = parent_context->getBatch2D();

    auto texture = assets->getTexture(this->texture);
    if (texture && autoresize) setSize(glm::vec2(texture->getWidth(), texture->getHeight()));
    batch->texture(texture);

    batch->setColor(color);
    batch->rect(pos.x, pos.y, size.x, size.y, 0, 0, 0, UVRegion(), false, true, color);
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
    setPressedColor(glm::vec4(0.0f, 0.0f, 0.0f, 0.95f));
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
    setPressedColor(glm::vec4(0.0f, 0.0f, 0.0f, 0.95f));
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
    glm::vec2 pos = calcPos();
    auto batch = parent_context->getBatch2D();
    batch->untexture();
    batch->setColor(isPressed() ? pressedColor : (hover ? hoverColor : color));
    batch->rect(pos.x, pos.y, size.x, size.y);
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

TextBox::TextBox(std::wstring placeholder, glm::vec4 padding) : Panel(glm::vec2(200, 32), padding, 0), input(L""), placeholder(placeholder) {
    label = std::make_shared<Label>(L"");
    label->setSize(size - glm::vec2(padding.z + padding.x, padding.w + padding.y));
    add(label);
    setHoverColor(glm::vec4(0.05f, 0.1f, 0.2f, 0.75f));

    textInitX = label->getPos().x;
}

void TextBox::draw(const GfxContext* pctx, Assets* assets) {
    Panel::draw(pctx, assets);

    font = assets->getFont(label->getFontName());

    if (!isFocused()) return;

    glm::vec2 pos = calcPos();
    glm::vec2 size = getSize();

    auto subctx = pctx->sub();
    subctx.setScissors(glm::vec4(pos.x, pos.y, size.x, size.y));

    const int lineHeight = font->getLineHeight() * label->getLineInterval();
    glm::vec2 lcoord = label->calcPos();
    lcoord.y -= 2;
    auto batch = pctx->getBatch2D();
    batch->untexture();
    if (editable && int((Window::time() - caretLastMove) * 2) % 2 == 0) {
        uint line = label->getLineByTextIndex(caret);
        uint lcaret = caret - label->getTextLineOffset(line);
        batch->setColor(glm::vec4(1.0f));

        int width = font->calcWidth(input, lcaret);
        batch->rect(lcoord.x + width, lcoord.y + label->getLineYOffset(line), 2, lineHeight);
    }
    if (selectionStart != selectionEnd) {
        uint startLine = label->getLineByTextIndex(selectionStart);
        uint endLine = label->getLineByTextIndex(selectionEnd);

        batch->setColor(glm::vec4(0.8f, 0.9f, 1.0f, 0.25f));
        int start = font->calcWidth(input, selectionStart-label->getTextLineOffset(startLine));
        int end = font->calcWidth(input, selectionEnd-label->getTextLineOffset(endLine));
        int startY = label->getLineYOffset(startLine);
        int endY = label->getLineYOffset(startLine);

        if (startLine == endLine) {
            batch->rect(lcoord.x + start, lcoord.y + startY, end - start, lineHeight);
        } else {
            batch->rect(lcoord.x + start, lcoord.y + endY, label->getSize().x - start - padding.z - padding.x - 2, lineHeight);
            for (uint i = startLine + 1; i < endLine; ++i) {
                batch->rect(lcoord.x, lcoord.y + label->getLineYOffset(i), label->getSize().x - padding.z - padding.x - 2, lineHeight);
            }
            batch->rect(lcoord.x, lcoord.y + label->getLineYOffset(endLine), end, lineHeight);
        }
    }
}

void TextBox::drawBackground(const GfxContext* parent_context, Assets* assets) {
    glm::vec2 pos = calcPos();

    auto batch = parent_context->getBatch2D();
    batch->untexture();

    auto sub_context = parent_context->sub();
    sub_context.setScissors(glm::vec4(pos.x, pos.y, size.x, size.y));

    if (valid) {
        if (isFocused() && !multiline) batch->setColor(focusedColor);
        else if (hover && !multiline) batch->setColor(hoverColor);
        else batch->setColor(color);
    } else {
        batch->setColor(invalidColor);
    }

    batch->rect(pos.x, pos.y, size.x, size.y);
    if (!isFocused() && supplier) input = supplier();

    if (isFocused() && multiline) {
        batch->setColor(glm::vec4(1, 1, 1, 0.1f));
        glm::vec2 lcoord = label->calcPos();
        lcoord.y -= 2;
        uint line = label->getLineByTextIndex(caret);
        int lineY = label->getLineYOffset(line);
        int lineHeight = font->getLineHeight() * label->getLineInterval();
        batch->rect(lcoord.x, lcoord.y + lineY, label->getSize().x, 1);
        batch->rect(lcoord.x, lcoord.y + lineY + lineHeight - 2, label->getSize().x, 1);
    }

    label->setColor(glm::vec4(input.empty() ? 0.5f : 1.0f));
    label->setText(getText());

    if (multiline && font) {
        setScrollable(true);
        uint height = label->getLinesNumber() * font->getLineHeight() * label->getLineInterval();
        label->setSize(glm::vec2(label->getSize().x, height));
        actualLength = height;
    } else {
        setScrollable(false);
    }
}

void TextBox::paste(const std::wstring& text) {
    eraseSelected();
    if (caret >= input.length()) {
        input += text;
    } else {
        auto left = input.substr(0, caret);
        auto right = input.substr(caret);
        input = left + text + right;
    }
    input.erase(std::remove(input.begin(), input.end(), '\r'), input.end());
    label->setText(input);

    setCaret(caret + text.length());
    validate();
}

void TextBox::erase(size_t start, size_t length) {
    size_t end = start + length;
    if (caret > start) setCaret(caret - length);
    auto left = input.substr(0, start);
    auto right = input.substr(end);
    input = left + right;
}

bool TextBox::eraseSelected() {
    if (selectionStart == selectionEnd) return false;
    erase(selectionStart, selectionEnd - selectionStart);
    resetSelection();
    return true;
}

void TextBox::resetSelection() {
    selectionOrigin = 0;
    selectionStart = 0;
    selectionEnd = 0;
}

void TextBox::extendSelection(int index) {
    size_t normalized = normalizeIndex(index);
    selectionStart = std::min(selectionOrigin, normalized);
    selectionEnd = std::max(selectionOrigin, normalized);
}

void TextBox::setTextOffset(uint x) {
    label->setPos(glm::vec2(textInitX - int(x), label->getPos().y));
    textOffset = x;
}

size_t TextBox::normalizeIndex(int index) {
    return std::min(input.length(), static_cast<size_t>(std::max(0, index)));
}

void TextBox::typed(unsigned int codepoint) {
    paste(std::wstring({(wchar_t)codepoint}));
}

bool TextBox::validate() {
    if (validator) valid = validator(getText());
    else valid = true;

    return valid;
}

void TextBox::setValid(bool valid) {
    this->valid = valid;
}

bool TextBox::isValid() const {
    return valid;
}

void TextBox::click(GUI*, int x, int y) {
    int index = normalizeIndex(calcIndexAt(x, y));
    selectionStart = index;
    selectionEnd = index;
    selectionOrigin = index;
}

void TextBox::mouseMove(GUI*, int x, int y) {
    int index = calcIndexAt(x, y);
    setCaret(index);
    extendSelection(index);
    resetMaxLocalCaret();
}

void TextBox::resetMaxLocalCaret() {
    maxLocalCaret = caret - label->getTextLineOffset(label->getLineByTextIndex(caret));
}

int TextBox::calcIndexAt(int x, int y) const {
    if (font == nullptr) return 0;
    glm::vec2 lcoord = label->calcPos();
    uint line = label->getLineByYOffset(y - lcoord.y);
    line = std::min(line, label->getLinesNumber() - 1);
    size_t lineLength = getLineLength(line);
    uint offset = 0;
    while (lcoord.x + font->calcWidth(input, offset) < x && offset < lineLength - 1) {
        offset++;
    }
    return std::min(offset + label->getTextLineOffset(line), input.length());
}

void TextBox::performEditingKeyboardEvents(keycode key) {
    bool shiftPressed = Events::isPressed(keycode::LEFT_SHIFT);
    bool breakSelection = getSelectionLength() != 0 && !shiftPressed;
    uint previousCaret = caret;
    if (key == keycode::BACKSPACE) {
        if (!eraseSelected() && caret > 0 && input.length() > 0) {
            if (caret > input.length()) caret = input.length();
            input = input.substr(0, caret - 1) + input.substr(caret);
            setCaret(caret - 1);
            validate();
        }
    } else if (key == keycode::ENTER) {
        if (multiline) {
            paste(L"\n");
        } else {
            if (validate() && consumer) consumer(label->getText());
            defocus();
        }
    } else if (key == keycode::TAB) {
        paste(L"    ");
    } else if (key == keycode::DEL) {
        if (!eraseSelected() && caret < input.length()) {
            input = input.substr(0, caret) + input.substr(caret + 1);
            validate();
        }
    } else if (key == keycode::LEFT) {
        uint caret = breakSelection ? selectionStart : this->caret;
        if (caret > 0) {
            if (caret > input.length()) setCaret(input.length() - 1);
            else setCaret(caret - 1);
            if (shiftPressed) {
                if (selectionStart == selectionEnd) selectionOrigin = previousCaret;
                extendSelection(this->caret);
            } else {
                resetSelection();
            }
        } else {
            setCaret(caret);
            resetSelection();
        }
        resetMaxLocalCaret();
    } else if (key == keycode::RIGHT) {
        uint caret = breakSelection ? selectionEnd : this->caret;
        if (caret < input.length()) {
            setCaret(caret + 1);
            if (shiftPressed) {
                if (selectionStart == selectionEnd) selectionOrigin = previousCaret;
                extendSelection(this->caret);
            } else {
                resetSelection();
            }
        } else {
            setCaret(caret);
            resetSelection();
        }
        resetMaxLocalCaret();
    } else if (key == keycode::UP) {
        uint caret = breakSelection ? selectionStart : this->caret;
        uint caretLine = label->getLineByTextIndex(caret);
        if (caretLine > 0) {
            uint offset = std::min(size_t(maxLocalCaret), getLineLength(caretLine - 1) - 1);
            setCaret(label->getTextLineOffset(caretLine - 1) + offset);
        } else {
            setCaret(0);
        }
        if (shiftPressed) {
            if (selectionStart == selectionEnd) selectionOrigin = previousCaret;
            extendSelection(this->caret);
        } else {
            resetSelection();
        }
    } else if (key == keycode::DOWN) {
        uint caret = breakSelection ? selectionEnd : this->caret;
        uint caretLine = label->getLineByTextIndex(caret);
        if (caretLine < label->getLinesNumber()-1) {
            uint offset = std::min(size_t(maxLocalCaret), getLineLength(caretLine + 1) - 1);
            setCaret(label->getTextLineOffset(caretLine + 1) + offset);
        } else {
            setCaret(input.length());
        }
        if (shiftPressed) {
            if (selectionStart == selectionEnd) selectionOrigin = previousCaret;
            extendSelection(this->caret);
        } else {
            resetSelection();
        }
    }
}

void TextBox::keyPressed(keycode key) {
    if (editable) performEditingKeyboardEvents(key);

    if (Events::isPressed(keycode::LEFT_CONTROL)) {
        if (key == keycode::C || key == keycode::X) {
            std::string text = util::wstr2str_utf8(getSelection());
            if (!text.empty()) Window::setClipboardText(text.c_str());
            if (editable && key == keycode::X) eraseSelected();
        }
        if (editable && key == keycode::V) {
            const char* text = Window::getClipboardText();
            if (text) paste(util::str2wstr_utf8(text));
        }
        if (key == keycode::A) {
            if (selectionStart == selectionEnd) select(0, input.length());
            else resetSelection();
        }
    }
}

void TextBox::setEditable(bool editable) {
    this->editable = editable;
}

bool TextBox::isEditable() const {
    return editable;
}

void TextBox::setOnEditStart(runnable oneditstart) {
    onEditStart = oneditstart;
}

void TextBox::onFocus(GUI* gui) {
    Panel::onFocus(gui);
    if (onEditStart) {
        setCaret(input.size());
        onEditStart();
        resetSelection();
    }
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

void TextBox::setFocusedColor(glm::vec4 color) {
    this->focusedColor = color;
}

glm::vec4 TextBox::getFocusedColor() const {
    return focusedColor;
}

void TextBox::setErrorColor(glm::vec4 color) {
    this->invalidColor = color;
}

glm::vec4 TextBox::getErrorColor() const {
    return invalidColor;
}

std::wstring TextBox::getText() const {
    if (input.empty()) return placeholder;
    return input;
}

void TextBox::setText(const std::wstring value) {
    this->input = value;
    input.erase(std::remove(input.begin(), input.end(), '\r'), input.end());
}

std::wstring TextBox::getPlaceholder() const {
    return placeholder;
}

void TextBox::setPlaceholder(const std::wstring& placeholder) {
    this->placeholder = placeholder;
}

void TextBox::refresh() {
    Panel::refresh();
    label->setSize(size - glm::vec2(padding.z + padding.x, padding.w + padding.y));
}

std::wstring TextBox::getSelection() const {
    return input.substr(selectionStart, selectionEnd - selectionStart);
}

void TextBox::select(int start, int end) {
    if (end < start) std::swap(start, end);
    start = normalizeIndex(start);
    end = normalizeIndex(end);

    selectionStart = start;
    selectionEnd = end;
    selectionOrigin = start;
    setCaret(selectionEnd);
}

uint TextBox::getCaret() const {
    return caret;
}

void TextBox::setCaret(uint position) {
    this->caret = std::min(size_t(position), input.length());
    caretLastMove = Window::time();

    int width = label->getSize().x;
    uint line = label->getLineByTextIndex(caret);
    int offset = label->getLineYOffset(line) + contentOffset().y;
    uint lineHeight = font->getLineHeight() * label->getLineInterval();
    scrollStep = lineHeight;
    if (offset < 0) scrolled(1);
    else if (offset >= getSize().y) scrolled(-1);
    uint lcaret = caret - label->getTextLineOffset(line);
    int realoffset = font->calcWidth(input, lcaret) - int(textOffset) + 2;
    if (realoffset - width > 0) {
        setTextOffset(textOffset + realoffset - width);
    } else if (realoffset < 0) {
        setTextOffset(std::max(textOffset + realoffset, 0U));
    }
}

void TextBox::setMultiline(bool multiline) {
    this->multiline = multiline;
    label->setMultiline(multiline);
    label->setVerticalAlign(multiline ? Align::top : Align::center);
}

bool TextBox::isMultiline() const {
    return multiline;
}

size_t TextBox::getLineLength(uint line) const {
    size_t position = label->getTextLineOffset(line);
    size_t lineLength = label->getTextLineOffset(line+1)-position;
    if (lineLength == 0) lineLength = input.length() - position + 1;
    return lineLength;
}

size_t TextBox::getSelectionLength() const {
    return selectionEnd - selectionStart;
}

InputBindBox::InputBindBox(Binding& binding, glm::vec4 padding) : Panel(glm::vec2(100, 32), padding, 0), binding(binding) {
    label = std::make_shared<Label>(L"");
    add(label);
    setScrollable(false);
}

void InputBindBox::drawBackground(const GfxContext* parent_context, Assets* assets) {
    glm::vec2 pos = calcPos();
    auto batch = parent_context->getBatch2D();
    batch->untexture();
    batch->setColor(isFocused() ? focusedColor : (hover ? hoverColor : color));
    batch->rect(pos.x, pos.y, size.x, size.y);
    label->setText(util::str2wstr_utf8(binding.text()));
}

void InputBindBox::clicked(GUI*, mousecode button) {
    binding.reset(button);
    defocus();
}

void InputBindBox::keyPressed(keycode key) {
    if (key != keycode::ESCAPE) binding.reset(key);
    defocus();
}

TrackBar::TrackBar(double min, double max, double value, double step, int trackWidth)
    : UINode(glm::vec2(26)), min(min), max(max), value(value), step(step), trackWidth(trackWidth) {
    setColor(glm::vec4(0.0f, 0.0f, 0.0f, 0.4f));
    setHoverColor(glm::vec4(0.01f, 0.02f, 0.03f, 0.5f));
}

void TrackBar::draw(const GfxContext* parent_context, Assets* assets) {
    if (supplier) value = supplier();

    glm::vec2 pos = calcPos();
    auto batch = parent_context->getBatch2D();
    batch->untexture();
    batch->setColor(hover ? hoverColor : color);
    batch->rect(pos.x, pos.y, size.x, size.y);

    float width = size.x;
    float t = (value - min) / (max - min + trackWidth * step);

    batch->setColor(trackColor);
    int actualWidth = size.x * (trackWidth / (max - min + trackWidth * step) * step);
    batch->rect(pos.x + width * t, pos.y, actualWidth, size.y);
}

void TrackBar::setSupplier(doublesupplier supplier_) {
    supplier = supplier_;
}

void TrackBar::setConsumer(doubleconsumer consumer_) {
    consumer = consumer_;
}

void TrackBar::mouseMove(GUI*, int x, int y) {
    glm::vec2 pos = calcPos();
    value = x;
    value -= pos.x;
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

CheckBox::CheckBox(bool checked) : UINode(glm::vec2(32.0f)), checked(checked) {
    setColor(glm::vec4(0.0f, 0.0f, 0.0f, 0.5f));
}

void CheckBox::draw(const GfxContext* parent_context, Assets* assets) {
    if (supplier) checked = supplier();

    glm::vec2 pos = calcPos();
    auto batch = parent_context->getBatch2D();
    batch->untexture();
    batch->setColor(checked ? checkColor : (hover ? hoverColor : color));
    batch->rect(pos.x, pos.y, size.x, size.y);
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
