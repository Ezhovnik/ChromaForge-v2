#pragma once

#include <graphics/ui/elements/Panel.h>
#include <graphics/ui/elements/Label.h>

class Font;
class ActionsHistory;

namespace gui {
    class TextBoxHistorian;
    class TextBox final : public Container {
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

        size_t selectionStart = 0;
        size_t selectionEnd = 0;
        size_t selectionOrigin = 0;

        bool multiline = false;
        bool editable = true;
        bool autoresize = false;
        bool showLineNumbers = false;
        bool keepLineSelection = false;

        std::string markup;
        std::string syntax;

        void stepCaret(bool shiftPressed, bool breakSelection, bool right);
        void stepDefaultDown(bool shiftPressed, bool breakSelection);
        void stepDefaultUp(bool shiftPressed, bool breakSelection);

        void onTab(bool shiftPressed);

        size_t normalizeIndex(int index);

        void setTextOffset(uint x);
        bool eraseSelected();
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

        ~TextBox();

        void setTextSupplier(wstringsupplier supplier);

        void setTextConsumer(wstringconsumer consumer);

        void setTextSubConsumer(wstringconsumer consumer);

        void setTextValidator(wstringchecker validator);

        void setOnControlCombination(key_handler handler);

        void setFocusedColor(glm::vec4 color);
        const glm::vec4& getFocusedColor() const;

        void setTextColor(glm::vec4 color);
        const glm::vec4& getTextColor() const;

        void setErrorColor(glm::vec4 color);
        glm::vec4 getErrorColor() const;

        const std::wstring& getText() const;
        void setText(const std::wstring& value);

        const std::wstring& getPlaceholder() const;
        void setPlaceholder(const std::wstring& text);

        const std::wstring& getHint() const;
        void setHint(const std::wstring& text);

        std::wstring getSelection() const;

        size_t getCaret() const;
        void setCaret(size_t position);
        void setCaret(ptrdiff_t position);

        void select(int start, int end);

        uint getLineAt(size_t position) const;
        size_t getLinePos(uint line) const;

        int calcIndexAt(int x, int y) const;
        int getLineYOffset(int line) const;

        bool validate();

        void setValid(bool valid);
        bool isValid() const;

        void setMultiline(bool multiline);
        bool isMultiline() const;

        void setEditable(bool editable);
        bool isEditable() const;

        void setPadding(glm::vec4 padding);
        const glm::vec4& getPadding() const;

        void setAutoResize(bool flag);
        bool isAutoResize() const;

        void setShowLineNumbers(bool flag);
        bool isShowLineNumbers() const;

        void setOnEditStart(runnable oneditstart);

        void setTextWrapping(bool flag);
        bool isTextWrapping() const;

        void setOnUpPressed(const runnable& callback);
        void setOnDownPressed(const runnable& callback);

        void setSyntax(std::string_view lang);
        const std::string& getSyntax() const;

        void setMarkup(std::string_view lang);
        const std::string& getMarkup() const;

        std::shared_ptr<Label> getLabel() const;

        bool isEdited() const;
        void setUnedited();

        size_t getSelectionStart() const;
        size_t getSelectionEnd() const;

        void setKeepLineSelection(bool flag);
        bool isKeepLineSelection() const;

        void reposition() override;
        void onFocus() override;
        void refresh() override;
        void click(int, int) override;
        void doubleClick(int x, int y) override;
        void mouseMove(int x, int y) override;
        bool isFocuskeeper() const override {return true;}
        void draw(const DrawContext& pctx, const Assets& assets) override;
        void drawBackground(const DrawContext& pctx, const Assets& assets) override;
        void typed(unsigned int codepoint) override; 
        void paste(const std::wstring& text, bool history=true);
        void erase(size_t start, size_t length);
        void resetSelection();
        void keyPressed(Keycode key) override;
        std::shared_ptr<UINode> getAt(const glm::vec2& pos) override;
    };
}
