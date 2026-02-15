#include "UINode.h"

#include "../../graphics/Batch2D.h"

using gui::UINode;
using gui::Align;

UINode::UINode(glm::vec2 coord, glm::vec2 size) : coord(coord), size_(size) {
}

UINode::~UINode() {
}

bool UINode::visible() const {
    return isvisible;
}

void UINode::visible(bool flag) {
    isvisible = flag;
}

Align UINode::align() const {
    return align_;
}

void UINode::align(Align align) {
    align_ = align;
}

void UINode::hover(bool flag) {
    hover_ = flag;
}

bool UINode::hover() const {
    return hover_;
}

void UINode::setParent(UINode* node) {
    parent = node;
}

UINode* UINode::getParent() const {
    return parent;
}

void UINode::click(GUI*, int x, int y) {
    pressed_ = true;
}

void UINode::mouseRelease(GUI*, int x, int y) {
    pressed_ = false;
}

bool UINode::ispressed() const {
    return pressed_;
}

void UINode::defocus() {
    focused_ = false;
}

bool UINode::isfocused() const {
    return focused_;
}

bool UINode::isInside(glm::vec2 pos) {
    glm::vec2 coord = calcCoord();
    glm::vec2 size = this->size();
    return (pos.x >= coord.x && pos.y >= coord.y && pos.x < coord.x + size.x && pos.y < coord.y + size.y);
}

std::shared_ptr<UINode> UINode::getAt(glm::vec2 pos, std::shared_ptr<UINode> self) {
    return isInside(pos) ? self : nullptr;
}

glm::vec2 UINode::calcCoord() const {
    if (parent) return coord + parent->calcCoord() + parent->contentOffset();
    return coord;
}

void UINode::scrolled(int value) {
    if (parent) parent->scrolled(value);
}


void UINode::setCoord(glm::vec2 coord) {
    this->coord = coord;
}

glm::vec2 UINode::size() const {
    return size_;
}

void UINode::size(glm::vec2 size) {
    if (sizelock) return;
    this->size_ = size;
}

void UINode::_size(glm::vec2 size) {
    if (sizelock) return;
    this->size_ = size;
}

void UINode::color(glm::vec4 color) {
    this->color_ = color;
}

glm::vec4 UINode::color() const {
    return color_;
}

void UINode::margin(glm::vec4 margin) {
    this->margin_ = margin;
}

glm::vec4 UINode::margin() const {
    return margin_;
}

void UINode::lock() {
}
