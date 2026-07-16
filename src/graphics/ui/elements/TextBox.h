#pragma once

#include <graphics/ui/elements/Panel.h>
#include <graphics/ui/elements/Label.h>

class Font;
class ActionsHistory;

namespace gui {
    class TextBoxHistorian;
    class TextBox : public Container {
        const Input& inputEvents;
        LabelCache rawTextCache;
        std::shared_ptr<ActionsHistory> history;
        std::unique_ptr<TextBoxHistorian> historian;
        int editedHistorySize = 0;
    protected:
        glm::vec4 focusedColor {0.0f, 0.0f, 0.0f, 1.0f};
        glm::vec4 invalidColor {0.1f, 0.05f, 0.03f, 1.0f};
        glm::vec4 textColor {1.0f, 1.0f, 1.0f, 1.0f};
        glm::vec4 padding {2};
        std::shared_ptr<Label> label;
        std::shared_ptr<Label> lineNumbersLabel;
        std::wstring input;
        std::wstring placeholder;
        std::wstring hint;
        wstringsupplier supplier = nullptr;
        wstringconsumer consumer = nullptr;
        wstringconsumer subconsumer = nullptr;
        wstringchecker validator = nullptr;
        key_handler controlCombinationsHandler = nullptr;
        runnable onEditStart = nullptr;
        runnable onUpPressed;
        runnable onDownPressed;
        bool valid = true;
        size_t caret = 0;
        size_t maxLocalCaret = 0;
        size_t textOffset = 0;
        int textInitX = 0;
        double caretLastMove = 0.0;
        Font* font = nullptr;

        size_t selectionStart = 0;
        size_t selectionEnd = 0;
        size_t selectionOrigin = 0;

        bool multiline = false;
        bool editable = true;
        bool autoresize = false;
        bool showLineNumbers = false;

        std::string markup;
        std::string syntax;

        void stepLeft(bool shiftPressed, bool breakSelection);
        void stepRight(bool shiftPressed, bool breakSelection);
        void stepDefaultDown(bool shiftPressed, bool breakSelection);
        void stepDefaultUp(bool shiftPressed, bool breakSelection);

        void onTab(bool shiftPressed);

        size_t normalizeIndex(int index);

        int calcIndexAt(int x, int y) const;
        void setTextOffset(uint x);
        bool eraseSelected();
        void resetSelection();
        void extendSelection(int index);
        void tokenSelectAt(int index);
        size_t getLineLength(uint line) const;

        size_t getSelectionLength() const;

        void resetMaxLocalCaret();

        void performEditingKeyboardEvents(Keycode key);

        void refreshLabel();

        void onInput();

        void refreshSyntax();
    public:
        explicit TextBox(
            GUI& gui,
            std::wstring placeholder, 
            glm::vec4 padding=glm::vec4(4.0f)
        );

        virtual ~TextBox();

        virtual void setTextSupplier(wstringsupplier supplier);

        virtual void setTextConsumer(wstringconsumer consumer);

        virtual void setTextSubConsumer(wstringconsumer consumer);

        virtual void setTextValidator(wstringchecker validator);

        virtual void setOnControlCombination(key_handler handler);

        virtual void setFocusedColor(glm::vec4 color);
        virtual glm::vec4 getFocusedColor() const;

        virtual void setTextColor(glm::vec4 color);
        virtual glm::vec4 getTextColor() const;

        virtual void setErrorColor(glm::vec4 color);
        virtual glm::vec4 getErrorColor() const;

        virtual const std::wstring& getText() const;
        virtual void setText(const std::wstring& value);

        virtual const std::wstring& getPlaceholder() const;
        virtual void setPlaceholder(const std::wstring& text);

        virtual const std::wstring& getHint() const;
        virtual void setHint(const std::wstring& text);

        virtual std::wstring getSelection() const;

        virtual size_t getCaret() const;
        virtual void setCaret(size_t position);
        virtual void setCaret(ptrdiff_t position);

        virtual void select(int start, int end);

        virtual uint getLineAt(size_t position) const;

        virtual size_t getLinePos(uint line) const;

        virtual bool validate();

        virtual void setValid(bool valid);
        virtual bool isValid() const;

        virtual void setMultiline(bool multiline);
        virtual bool isMultiline() const;

        virtual void setEditable(bool editable);
        virtual bool isEditable() const;

        virtual void setPadding(glm::vec4 padding);
        glm::vec4 getPadding() const;

        virtual void setAutoResize(bool flag);
        virtual bool isAutoResize() const;

        virtual void setShowLineNumbers(bool flag);
        virtual bool isShowLineNumbers() const;

        virtual void setOnEditStart(runnable oneditstart);

        virtual void setTextWrapping(bool flag);
        virtual bool isTextWrapping() const;

        virtual void setOnUpPressed(const runnable& callback);
        virtual void setOnDownPressed(const runnable& callback);

        virtual void setSyntax(std::string_view lang);
        virtual const std::string& getSyntax() const;

        virtual void setMarkup(std::string_view lang);
        virtual const std::string& getMarkup() const;

        virtual bool isEdited() const;
        virtual void setUnedited();

        virtual void reposition() override;
        virtual void onFocus() override;
        virtual void refresh() override;
        virtual void click(int, int) override;
        virtual void doubleClick(int x, int y) override;
        virtual void mouseMove(int x, int y) override;
        virtual bool isFocuskeeper() const override {return true;}
        virtual void draw(const DrawContext& pctx, const Assets& assets) override;
        virtual void drawBackground(const DrawContext& pctx, const Assets& assets) override;
        virtual void typed(unsigned int codepoint) override; 
        void paste(const std::wstring& text, bool history=true);
        void erase(size_t start, size_t length);
        size_t getSelectionStart() const;
        size_t getSelectionEnd() const;
        virtual void keyPressed(Keycode key) override;
        virtual std::shared_ptr<UINode> getAt(const glm::vec2& pos) override;
    };
}
