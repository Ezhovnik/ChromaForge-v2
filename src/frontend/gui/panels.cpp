#include "panels.h"

#include <algorithm>

#include "../../window/Window.h"
#include "../../assets/Assets.h"
#include "../../graphics/Batch2D.h"

using gui::UINode;
using gui::Container;
using gui::Panel;
using gui::Orientation;

Container::Container(glm::vec2 coord, glm::vec2 size) : UINode(coord, size) {
}

std::shared_ptr<UINode> Container::getAt(glm::vec2 pos, std::shared_ptr<UINode> self) {
    for (auto node : nodes) {
        if (!node->visible()) continue;
        auto hover = node->getAt(pos, node);
        if (hover != nullptr) return hover;
    }
    return UINode::getAt(pos, self);
}

void Container::activate(float delta) {
    for (IntervalEvent& event : intervalEvents) {
        event.timer += delta;
        if (event.timer > event.interval) {
            event.callback();
            event.timer = fmod(event.timer, event.interval);
            if (event.repeat > 0) event.repeat--;
        }
    }

    intervalEvents.erase(std::remove_if(
        intervalEvents.begin(), intervalEvents.end(),
        [](const IntervalEvent& event) {
            return event.repeat == 0;
        }
    ), intervalEvents.end());

    for (auto node : nodes) {
        if (node->visible()) node->activate(delta);
    }
}

void Container::draw(Batch2D* batch, Assets* assets) {
    glm::vec2 coord = calcCoord();
    glm::vec2 size = this->size();
    drawBackground(batch, assets);
    batch->texture(nullptr);
    batch->render();
    Window::pushScissor(glm::vec4(coord.x, coord.y, size.x, size.y));
    for (auto node : nodes) {
        if (node->visible()) node->draw(batch, assets);
    }
    batch->render();
    Window::popScissor();
}

void Container::add(std::shared_ptr<UINode> node) {
    nodes.push_back(node);
    node->setParent(this);
    refresh();
}

void Container::remove(std::shared_ptr<UINode> selected) {
    selected->setParent(nullptr);
    nodes.erase(std::remove_if(nodes.begin(), nodes.end(), 
        [selected](const std::shared_ptr<UINode> node) {
            return node == selected;
        }
    ), nodes.end());
    refresh();
}

void Container::listenInterval(float interval, std::function<void()> callback, int repeat) {
    intervalEvents.push_back({callback, interval, 0.0f, repeat});
}

Panel::Panel(glm::vec2 size, glm::vec4 padding, float interval, bool resizing)
    : Container(glm::vec2(), size), padding(padding), interval(interval), resizing_(resizing) {
    color_ = glm::vec4(0.0f, 0.0f, 0.0f, 0.75f);
}

Panel::~Panel() {
}

void Panel::drawBackground(Batch2D* batch, Assets* assets) {
    glm::vec2 coord = calcCoord();
    batch->texture(nullptr);
    batch->color = color_;
    batch->rect(coord.x, coord.y, size_.x, size_.y);
}

void Panel::refresh() {
    float x = padding.x;
    float y = padding.y;
    glm::vec2 size = this->size();
    if (orientation_ == Orientation::vertical) {
        float maxw = size.x;
        for (auto& node : nodes) {
            glm::vec2 nodesize = node->size();
            const glm::vec4 margin = node->margin();
            y += margin.y;
            
            float ex;

            float spacex = size.x - margin.z - padding.z;
            switch (node->align()) {
                case Align::center:
                    ex = x + fmax(0.0f, spacex - node->size().x) / 2.0f;
                    break;
                case Align::right:
                    ex = spacex - node->size().x;
                    break;
                default:
                    ex = x + margin.x;
            }
            node->setCoord(glm::vec2(ex, y));
            y += nodesize.y + margin.w + interval;

            float width = size.x - padding.x - padding.z - margin.x - margin.z;
            node->size(glm::vec2(width, nodesize.y));
            maxw = fmax(maxw, ex+node->size().x + margin.z + padding.z);
        }
        if (resizing_) this->size(glm::vec2(size.x, y+padding.w));
    } else {
        float maxh = size.y;
        for (auto& node : nodes) {
            glm::vec2 nodesize = node->size();
            const glm::vec4 margin = node->margin();
            x += margin.x;
            node->setCoord(glm::vec2(x, y+margin.y));
            x += nodesize.x + margin.z + interval;

            float height = size.y - padding.y - padding.w - margin.y - margin.w;
            node->size(glm::vec2(nodesize.x, height));
            maxh = fmax(maxh, y + margin.y + node->size().y + margin.w + padding.w);
        }
        bool increased = maxh > size.y;
        if (resizing_) this->size(glm::vec2(x+padding.z, size.y));
        if (increased) refresh();
    }
}

void Panel::orientation(Orientation orientation) {
    this->orientation_ = orientation;
}

Orientation Panel::orientation() const {
    return orientation_;
}

void Panel::lock(){
    for (auto node : nodes) {
        node->lock();
    }
    resizing_ = false;
}
