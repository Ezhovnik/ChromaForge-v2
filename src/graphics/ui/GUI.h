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

class DrawContext;
class Assets;
class Camera;
class Viewport;

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

        std::shared_ptr<UINode> hover;
        std::shared_ptr<UINode> pressed;
        std::shared_ptr<UINode> focus;
        std::shared_ptr<UINode> tooltip;

        std::unordered_map<std::string, std::shared_ptr<UINode>> storage;

        std::unique_ptr<Camera> uicamera;
        std::shared_ptr<Menu> menu;

        std::queue<runnable> postRunnables;

        float doubleClickTimer = 0.0f;
        float doubleClickDelay = 0.5f;
        bool doubleClicked = false;

        float tooltipTimer = 0.0f;

        void activateMouse(float deltaTIme);
        void activateFocused();
        void updateTooltip(float deltaTime);
        void resetTooltip();
    public:
        GUI();
        ~GUI();

        std::shared_ptr<Menu> getMenu();

        std::shared_ptr<UINode> getFocused() const;
        bool isFocusCaught() const;
        void setFocus(std::shared_ptr<UINode> node);

        void activate(float deltaTime, const Viewport& viewport);
        void draw(const DrawContext* parent_context, Assets* assets);
        void add(std::shared_ptr<UINode> panel);
        void remove(std::shared_ptr<UINode> panel) noexcept;
        void store(const std::string& name, std::shared_ptr<UINode> node);
        std::shared_ptr<UINode> get(const std::string& name) noexcept;
        void remove(const std::string& name) noexcept;

        std::shared_ptr<Container> getContainer() const;

        void onAssetsLoad(Assets* assets);

        void postRunnable(const runnable& callback);

        void setDoubleClickDelay(float delay);
        float getDoubleClickDelay() const;
    };
}

#endif // GRAPHICS_UI_GUI_H_
