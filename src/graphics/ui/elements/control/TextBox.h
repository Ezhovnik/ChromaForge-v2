#ifndef GRAPHICS_UI_ELEMENTS_CONTROL_TEXTBOX_H_
#define GRAPHICS_UI_ELEMENTS_CONTROL_TEXTBOX_H_

#include "../layout/Panel.h"
#include "../display/Label.h"

class Font;

namespace gui {
    class Label;

    class TextBox : public Panel {
    protected:
        glm::vec4 focusedColor {0.0f, 0.0f, 0.0f, 1.0f};
        glm::vec4 invalidColor {0.1f, 0.05f, 0.03f, 1.0f};
        std::shared_ptr<Label> label;
        std::wstring input;
        std::wstring placeholder;
        wstringsupplier supplier = nullptr;
        wstringconsumer consumer = nullptr;
        wstringchecker validator = nullptr;
        runnable onEditStart = nullptr;
        runnable onUpPressed;
        runnable onDownPressed;
        bool valid = true;
        uint caret = 0;
        uint maxLocalCaret = 0;
        uint textOffset = 0;
        int textInitX;
        double caretLastMove = 0.0;
        Font* font = nullptr;

        size_t selectionStart = 0;
        size_t selectionEnd = 0;
        size_t selectionOrigin = 0;

        bool multiline = false;
        bool editable = true;
        bool autoresize = false;

        void stepLeft(bool shiftPressed, bool breakSelection);
        void stepRight(bool shiftPressed, bool breakSelection);
        void stepDefaultDown(bool shiftPressed, bool breakSelection);
        void stepDefaultUp(bool shiftPressed, bool breakSelection);

        size_t normalizeIndex(int index);

        int calcIndexAt(int x, int y) const;
        void setTextOffset(uint x);
        void erase(size_t start, size_t length);
        bool eraseSelected();
        void resetSelection();
        void extendSelection(int index);
        void tokenSelectAt(int index);
        size_t getLineLength(uint line) const;

        size_t getSelectionLength() const;

        void resetMaxLocalCaret();

        void performEditingKeyboardEvents(keycode key);

        void refreshLabel();
    public:
        TextBox(
            std::wstring placeholder, 
            glm::vec4 padding=glm::vec4(4.0f)
        );

        virtual void setTextSupplier(wstringsupplier supplier);

        virtual void setTextConsumer(wstringconsumer consumer);

        virtual void setTextValidator(wstringchecker validator);

        virtual void setFocusedColor(glm::vec4 color);
        virtual glm::vec4 getFocusedColor() const;

        virtual void setErrorColor(glm::vec4 color);
        virtual glm::vec4 getErrorColor() const;

        virtual std::wstring getText() const;
        virtual void setText(std::wstring value);

        virtual std::wstring getPlaceholder() const;
        virtual void setPlaceholder(const std::wstring& text);

        virtual std::wstring getSelection() const;

        virtual uint getCaret() const;
        virtual void setCaret(uint position);

        virtual void select(int start, int end);

        virtual bool validate();

        virtual void setValid(bool valid);
        virtual bool isValid() const;

        virtual void setMultiline(bool multiline);
        virtual bool isMultiline() const;

        virtual void setEditable(bool editable);
        virtual bool isEditable() const;

        virtual void setAutoResize(bool flag);
        virtual bool isAutoResize() const;

        virtual void setOnEditStart(runnable oneditstart);

        virtual void setTextWrapping(bool flag);
        virtual bool isTextWrapping() const;

        virtual void setOnUpPressed(runnable callback);
        virtual void setOnDownPressed(runnable callback);

        virtual void onFocus(GUI*) override;
        virtual void refresh() override;
        virtual void click(GUI*, int, int) override;
        virtual void doubleClick(GUI*, int x, int y) override;
        virtual void mouseMove(GUI*, int x, int y) override;
        virtual bool isFocuskeeper() const override {return true;}
        virtual void draw(const DrawContext* pctx, Assets* assets) override;
        virtual void drawBackground(const DrawContext* pctx, Assets* assets) override;
        virtual void typed(unsigned int codepoint) override; 
        void paste(const std::wstring& text);
        virtual void keyPressed(keycode key) override;
        virtual std::shared_ptr<UINode> getAt(glm::vec2 pos, std::shared_ptr<UINode> self) override;
    };
}

#endif // GRAPHICS_UI_ELEMENTS_CONTROL_TEXTBOX_H_
