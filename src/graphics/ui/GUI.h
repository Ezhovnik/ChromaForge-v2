#ifndef GRAPHICS_UI_GUI_H_
#define GRAPHICS_UI_GUI_H_

#include <memory>
#include <vector>
#include <string>
#include <unordered_map>
#include <functional>
#include <queue>

#include <glm/glm.hpp>

#include "../../delegates.h"

class GfxContext;
class Assets;
class Camera;

/*
Padding is element inner space, margin is outer
    glm::vec4 usage:
    x - left
    y - top
    z - right
    w - bottom

Outer element
 +======================================================================+
 |            .           .                    .          .             |
 |            .padding.y  .                    .          .             |
 | padding.x  .           .                    .          .   padding.z |
 |- - - - - - + - - - - - + - - - - - - - - - -+- - - - - + - - - - - - |
 |            .           .                    .          .             |
 |            .           .margin.y            .          .             |
 |            .margin.x   .                    .  margin.z.             |
 |- - - - - - + - - - - - +====================+- - - - - + - - - - - - |
 |            .           |    Inner element   |          .             |
 |- - - - - - + - - - - - +====================+- - - - - + - - - - - - |
 |            .           .                    .          .             |
 |            .           .margin.w            .          .             |
 |            .           .                    .          .             |
 |- - - - - - + - - - - - + - - - - - - - - - -+- - - - - + - - - - - - |
 |            .           .                    .          .             |
 |            .padding.w  .                    .          .             |
 |            .           .                    .          .             |
 +======================================================================+
*/

namespace gui {
    class UINode;
    class Container;
    class Menu;

    class GUI {
        std::shared_ptr<Container> container;

        std::shared_ptr<UINode> hover = nullptr;
        std::shared_ptr<UINode> pressed = nullptr;
        std::shared_ptr<UINode> focus = nullptr;

        std::unordered_map<std::string, std::shared_ptr<UINode>> storage;

        std::unique_ptr<Camera> uicamera;
        std::shared_ptr<Menu> menu;

        std::queue<runnable> postRunnables;

        void activateMouse(float delta);
        void activateFocused();
    public:
        GUI();
        ~GUI();

        std::shared_ptr<Menu> getMenu();

        std::shared_ptr<UINode> getFocused() const;
        bool isFocusCaught() const;
        void setFocus(std::shared_ptr<UINode> node);

        void activate(float delta);
        void draw(const GfxContext* parent_context, Assets* assets);
        void add(std::shared_ptr<UINode> panel);
        void add(std::shared_ptr<UINode> node, glm::vec2 coord);
        void remove(std::shared_ptr<UINode> panel) noexcept;
        void store(std::string name, std::shared_ptr<UINode> node);
        std::shared_ptr<UINode> get(std::string name) noexcept;
        void remove(std::string name) noexcept;

        std::shared_ptr<Container> getContainer() const;

        void postRunnable(runnable callback);
    };
}

#endif // GRAPHICS_UI_GUI_H_
