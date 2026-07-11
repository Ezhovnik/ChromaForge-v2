#pragma once

#include <memory>
#include <vector>
#include <string>
#include <unordered_map>
#include <functional>
#include <queue>

#include <glm/glm.hpp>

#include <delegates.h>

class DrawContext;
class Assets;
class Camera;
class Batch2D;
struct CursorState;
class Engine;
class Input;
class Window;
namespace devtools {
    class Editor;
}

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

    using PageLoaderFunc = std::function<std::shared_ptr<UINode>(const std::string&)>;

    class GUI {
        Engine& engine;
        Input& input;
        std::unique_ptr<Batch2D> batch2D;
        std::shared_ptr<Container> container;

        std::shared_ptr<UINode> hover;
        std::shared_ptr<UINode> pressed;
        std::shared_ptr<UINode> focus;
        std::shared_ptr<UINode> tooltip;

        std::unordered_map<std::string, std::shared_ptr<UINode>> storage;

        std::unique_ptr<Camera> uicamera;
        std::shared_ptr<Menu> menu;

        std::queue<runnable> postRunnables;

        PageLoaderFunc pagesLoader;

        float doubleClickTimer = 0.0f;
        float doubleClickDelay = 0.5f;
        bool doubleClicked = false;
        bool debug = false;

        float tooltipTimer = 0.0f;

        void activateMouse(float deltaTime, const CursorState& cursor);
        void activateFocused();
        void updateTooltip(float deltaTime);
        void resetTooltip();
    public:
        GUI(Engine& engine);
        ~GUI();

        void setPageLoader(PageLoaderFunc pageLoader);
        PageLoaderFunc getPagesLoader();

        std::shared_ptr<Menu> getMenu();

        std::shared_ptr<UINode> getFocused() const;
        bool isFocusCaught() const;
        void setFocus(std::shared_ptr<UINode> node);

        void activate(float deltaTime, const glm::uvec2& viewport);
        void postActivate();

        void draw(const DrawContext& parent_context, const Assets& assets);
        void add(std::shared_ptr<UINode> panel);
        void remove(UINode* node) noexcept;
        void remove(const std::shared_ptr<UINode>& node) noexcept {
            return remove(node.get());
        }
        void store(const std::string& name, std::shared_ptr<UINode> node);
        std::shared_ptr<UINode> get(const std::string& name) noexcept;
        void remove(const std::string& name) noexcept;

        std::shared_ptr<Container> getContainer() const;

        void onAssetsLoad(Assets* assets);

        void postRunnable(const runnable& callback);

        void setDoubleClickDelay(float delay);
        float getDoubleClickDelay() const;

        void toggleDebug();
        const Input& getInput() const;
        Input& getInput();
        Window& getWindow();
        devtools::Editor& getEditor();
        Engine& getEngine();
    };
}
