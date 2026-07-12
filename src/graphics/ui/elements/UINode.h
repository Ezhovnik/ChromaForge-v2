#pragma once

#include <string>
#include <vector>
#include <memory>
#include <functional>
#include <unordered_map>

#include <glm/glm.hpp>

#include <typedefs.h>
#include <delegates.h>
#include <window/input.h>
#include <graphics/core/commons.h>

class DrawContext;
class Assets;

namespace gui {
    class UINode;
    class GUI;
    class Container;

    using onaction = std::function<void(GUI&)>;
    using onnumberchange = std::function<void(GUI&, double)>;
    using onstringchange = std::function<void(GUI&, const std::string&)>;

    template<typename... Args>
    class CallbacksSet {
    public:
        using Func = std::function<void(Args...)>;
    private:
        std::unique_ptr<std::vector<Func>> callbacks;
    public:
        void listen(const Func& callback) {
            if (callbacks == nullptr) {
                callbacks = std::make_unique<std::vector<Func>>();
            }
            callbacks->push_back(callback);
        }

        void notify(Args&&... args) {
            if (callbacks) {
                for (auto& callback : *callbacks) {
                    callback(std::forward<Args>(args)...);
                }
            }
        }
    };

    using ActionsSet = CallbacksSet<GUI&>;
    using StringCallbacksSet = CallbacksSet<GUI&, const std::string&>;

    enum class Align {
        left,
        center,
        right,
        top = left,
        bottom = right
    };

    enum class Gravity {
        none,

        top_left,
        top_center,
        top_right,

        center_left,
        center_center,
        center_right,

        bottom_left,
        bottom_center,
        bottom_right
    };

    class UINode : public std::enable_shared_from_this<UINode> {
    protected:
        GUI& gui;
        bool mustRefresh = true;
    private:
        std::string id = "";

        bool enabled = true;
    protected:
        glm::vec2 pos {0.0f};
        glm::vec2 size;
        glm::vec2 minSize {1.0f};
        glm::vec2 maxSize {1e6f};
        glm::vec4 color {1.0f};
        glm::vec4 hoverColor {1.0f};
        glm::vec4 pressedColor {1.0f};
        glm::vec4 margin {0.0f};
        int zindex = 0;
        bool visible = true;
        bool hover = false;
        bool pressed = false;
        bool focused = false;
        bool interactive = true;
        bool resizing = true;
        Align align = Align::left;
        vec2supplier positionfunc = nullptr;
        vec2supplier sizefunc = nullptr;
        UINode* parent = nullptr;
        ActionsSet actions;
        ActionsSet doubleClickCallbacks;
        ActionsSet focusCallbacks;
        ActionsSet defocusCallbacks;
        std::wstring tooltip;
        float tooltipDelay = 0.5f;
        CursorShape cursor = CursorShape::Arrow;

        UINode(GUI& gui, glm::vec2 size);
    public:
        virtual ~UINode();
        virtual void activate(float deltaTime) {
            if (mustRefresh) {
                mustRefresh = false;
                refresh();
            }
        };
        virtual void draw(const DrawContext& parent_context, const Assets& assets) = 0;

        virtual void setVisible(bool flag);
        bool isVisible() const;

        virtual void setAlign(Align align);
        Align getAlign() const;

        virtual void setHover(bool flag);
        bool isHover() const;

        virtual void setTooltip(const std::wstring& text);
        virtual const std::wstring& getTooltip() const;

        virtual void setTooltipDelay(float delay);
        virtual float getTooltipDelay() const;

        virtual void setCursor(CursorShape shape);
        virtual CursorShape getCursor() const;

        virtual void setParent(UINode* node);
        UINode* getParent() const;

        virtual void setEnabled(bool flag);
        bool isEnabled() const;

        virtual void setColor(glm::vec4 newColor);
        glm::vec4 getColor() const;

        virtual void setHoverColor(glm::vec4 newColor);
        glm::vec4 getHoverColor() const;

        virtual glm::vec4 getPressedColor() const;
        virtual void setPressedColor(glm::vec4 color);

        virtual glm::vec4 calcColor() const;

        virtual void setResizing(bool flag);
        virtual bool isResizing() const;

        virtual void setMargin(glm::vec4 margin);
        glm::vec4 getMargin() const;

        virtual void setZIndex(int idx);
        int getZIndex() const;

        virtual UINode* listenAction(const onaction& action);
        virtual UINode* listenDoubleClick(const onaction& action);
        virtual UINode* listenFocus(const onaction& action);
        virtual UINode* listenDefocus(const onaction& action);

        virtual void onFocus();
        virtual void click(int x, int y);
        virtual void doubleClick(int x, int y);
        virtual void clicked(Mousecode button) {}
        virtual void mouseMove(int x, int y) {};
        virtual void mouseRelease(int x, int y);
        virtual void scrolled(int value);

        bool isPressed() const;
        void defocus();
        bool isFocused() const; 
        virtual bool isFocuskeeper() const {return false;}

        virtual void typed(uint codepoint) {};
        virtual void keyPressed(Keycode key) {};

        virtual bool isInside(glm::vec2 pos);
        virtual std::shared_ptr<UINode> getAt(const glm::vec2& pos);

        virtual bool isInteractive() const;
        virtual void setInteractive(bool flag);

        virtual glm::vec2 getContentOffset() {return glm::vec2(0.0f);};

        virtual glm::vec2 calcPos() const;
        virtual void setPos(glm::vec2 pos);
        virtual glm::vec2 getPos() const;

        glm::vec2 getSize() const;
        virtual void setSize(glm::vec2 size);
        glm::vec2 getMinSize() const;
        virtual void setMinSize(glm::vec2 size);
        glm::vec2 getMaxSize() const;
        virtual void setMaxSize(glm::vec2 size);

        virtual vec2supplier getPositionFunc() const;
        virtual void setPositionFunc(vec2supplier);

        virtual vec2supplier getSizeFunc() const;
        virtual void setSizeFunc(vec2supplier);

        virtual void setGravity(Gravity gravity);

        void setMustRefresh() {
            mustRefresh = true;
        }

        bool isSubnodeOf(const UINode* node);

        virtual void refresh() {};
        virtual void fullRefresh() {
            if (parent) parent->fullRefresh();
        };
        static void moveInto(
            const std::shared_ptr<UINode>& node,
            const std::shared_ptr<Container>& dest
        );

        void setId(const std::string& id);
        const std::string& getId() const;

        virtual void reposition();

        static void getIndices(
            const std::shared_ptr<UINode>& node,
            std::unordered_map<std::string, std::shared_ptr<UINode>>& map
        );

        static std::shared_ptr<UINode> find(
            const std::shared_ptr<UINode>& node,
            const std::string& id
        );
    };
}
