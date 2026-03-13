#ifndef FRONTEND_GUI_CONTROLS_H_
#define FRONTEND_GUI_CONTROLS_H_

#include <string>
#include <memory>
#include <vector>
#include <functional>
#include <glm/glm.hpp>

#include "UINode.h"
#include "containers.h"
#include "../../window/input.h"
#include "../../typedefs.h"
#include "GUI.h"
#include "../../delegates.h"

class Batch2D;
class Assets;
class Font;

namespace gui {
    class Label : public UINode {
    protected:
        std::wstring text;
        std::string fontName;
        wstringsupplier supplier = nullptr;

        uint lines = 1;
        float lineInterval = 1.5f;
        Align valign = Align::center;

        bool multiline = false;

        /// Смещение относительно оси Y
        int textYOffset = 0;

        // Высота строки текста, умноженная на межстрочный интервал
        int totalLineHeight = 1;
    public:
        Label(std::string text, std::string fontName="normal");
        Label(std::wstring text, std::string fontName="normal");

        virtual void setText(std::wstring text);
        const std::wstring& getText() const;

        virtual void setFontName(std::string name);
        virtual const std::string& getFontName() const;

        /**
         * @brief Устанавливает вертикальное выравнивание текста (по умолчанию — по центру)
         * @param Выравнивание (Align::top / Align::center / Align::bottom).
         */
        virtual void setVerticalAlign(Align align);

        /**
         * @brief Возвращает текущее вертикальное выравнивание текста.
         * @return Текущее выравнивание текста.
         */
        virtual Align getVerticalAlign() const;

        /**
         * @brief Возвращает коэффициент высоты строки для многострочных Label-ов
         * 
         * Значение по умолчанию: 1.5
         */
        virtual float getLineInterval() const;

        /**
         * @brief Установит коэффициент высоты строки для многострочных Label-ов
         */
        virtual void setLineInterval(float interval);

        virtual int getTextYOffset() const;
        virtual int getLineYOffset(uint line) const;

        virtual size_t getTextLineOffset(uint line) const;

        virtual uint getLineByYOffset(int offset) const;
        virtual uint getLineByTextIndex(size_t index) const;
        virtual uint getLinesNumber() const;

        virtual void setMultiline(bool multiline);
        virtual bool isMultiline() const;

        virtual void draw(const GfxContext* parent_context, Assets* assets) override;

        virtual void textSupplier(wstringsupplier supplier);
    };

    class Image : public UINode {
    protected:
        std::string texture;
        bool autoresize = false;
    public:
        Image(std::string texture, glm::vec2 size=glm::vec2(32,32));

        virtual void draw(const GfxContext* parent_context, Assets* assets) override;

        virtual void setAutoResize(bool flag);
        virtual bool isAutoResize() const;
    };

    class Button : public Panel {
    protected:
        glm::vec4 pressedColor {0.0f, 0.0f, 0.0f, 0.95f};

        std::vector<onaction> actions;

        std::shared_ptr<Label> label = nullptr;
    public:
        Button(std::shared_ptr<UINode> content, glm::vec4 padding = glm::vec4(2.0f));
        Button(std::wstring text, glm::vec4 padding, onaction action, glm::vec2 size=glm::vec2(-1));

        virtual void drawBackground(const GfxContext* parent_context, Assets* assets) override;

        virtual void mouseRelease(GUI*, int x, int y) override;
        virtual Button* listenAction(onaction action);

        virtual void setTextAlign(Align align);
        virtual Align getTextAlign() const;

        virtual void setText(std::wstring text);
        virtual std::wstring getText() const;

        virtual glm::vec4 getPressedColor() const;
        virtual void setPressedColor(glm::vec4 color);

        virtual Button* textSupplier(wstringsupplier supplier);

        virtual void refresh() override;
    };

    class RichButton : public Container {
    protected:
        glm::vec4 pressedColor {0.0f, 0.0f, 0.0f, 0.95f};
        std::vector<onaction> actions;
    public:
        RichButton(glm::vec2 size);

        virtual void drawBackground(const GfxContext* parent_context, Assets* assets) override;

        virtual void mouseRelease(GUI*, int x, int y) override;
        virtual RichButton* listenAction(onaction action);
    };

    class InputBindBox : public Panel {
    protected:
        glm::vec4 hoverColor {0.05f, 0.1f, 0.2f, 0.75f};
        glm::vec4 focusedColor {0.0f, 0.0f, 0.0f, 1.0f};

        std::shared_ptr<Label> label;
        Binding& binding;
    public:
        InputBindBox(Binding& binding, glm::vec4 padding = glm::vec4(6.0f));
        virtual void drawBackground(const GfxContext* parent_context, Assets* assets) override;

        virtual void clicked(GUI*, mousecode button) override;
        virtual void keyPressed(keycode key) override;
        virtual bool isFocuskeeper() const override {return true;}
    };


    class TextBox : public Panel {
    protected:
        glm::vec4 focusedColor {0.0f, 0.0f, 0.0f, 1.0f};
        glm::vec4 invalidColor {0.3f, 0.0f, 0.0f, 0.5f};

        runnable onEditStart = nullptr;

        std::shared_ptr<Label> label;

        std::wstring input;
        std::wstring placeholder;

        Font* font = nullptr;

        wstringsupplier supplier = nullptr;
        wstringconsumer consumer = nullptr;

        wstringchecker validator = nullptr;
        bool valid = true;

        uint caret = 0;
        double caretLastMove = 0.0;
        uint textOffset = 0;
        int textInitX;
        uint maxLocalCaret = 0;

        size_t selectionStart = 0;
        size_t selectionEnd = 0;
        size_t selectionOrigin = 0;

        bool multiline = false;
        bool editable = true;

        size_t normalizeIndex(int index);

        int calcIndexAt(int x, int y) const;
        void paste(const std::wstring& text);
        void setTextOffset(uint x);
        void erase(size_t start, size_t length);
        bool eraseSelected();
        void resetSelection();
        void extendSelection(int index);

        void performEditingKeyboardEvents(keycode key);
    public:
        TextBox(std::wstring placeholder, glm::vec4 padding = glm::vec4(4.0f));

        virtual void setTextSupplier(wstringsupplier supplier);
        virtual void setTextConsumer(wstringconsumer consumer);
        virtual void setTextValidator(wstringchecker validator);

        virtual void setFocusedColor(glm::vec4 color);
        virtual glm::vec4 getFocusedColor() const;

        virtual void setErrorColor(glm::vec4 color);
        virtual glm::vec4 getErrorColor() const;

        virtual std::wstring getSelection() const;
        virtual void select(int start, int end);

        virtual uint getCaret() const;
        virtual void setCaret(uint position);

        void resetMaxLocalCaret();

        virtual std::wstring getText() const;
        virtual void setText(std::wstring value);

        virtual std::wstring getPlaceholder() const;
        virtual void setPlaceholder(const std::wstring& text);

        virtual void setMultiline(bool multiline);
        virtual bool isMultiline() const;

        virtual void setEditable(bool editable);
        virtual bool isEditable() const;

        virtual void draw(const GfxContext* pctx, Assets* assets) override;
        virtual void drawBackground(const GfxContext* pctx, Assets* assets) override;

        virtual void typed(unsigned int codepoint) override; 
        virtual void keyPressed(keycode key) override;

        virtual bool isFocuskeeper() const override {return true;}
        virtual std::shared_ptr<UINode> getAt(glm::vec2 pos, std::shared_ptr<UINode> self) override;

        virtual void setOnEditStart(runnable oneditstart);
        virtual void focus(GUI*) override;
        virtual void refresh() override;

        virtual bool validate();
        virtual void setValid(bool valid);
        virtual bool isValid() const;

        virtual void click(GUI*, int, int) override;
        virtual void mouseMove(GUI*, int x, int y) override;

        size_t getLineLength(uint line) const;
        size_t getSelectionLength() const;
    };

    class TrackBar : public UINode {
    protected:
        glm::vec4 trackColor {1.0f, 1.0f, 1.0f, 0.4f};
        doublesupplier supplier = nullptr;
        doubleconsumer consumer = nullptr;
        double min;
        double max;
        double value;
        double step;
        int trackWidth;
    public:
        TrackBar(double min, double max, double value, double step = 1.0, int trackWidth = 1);
        virtual void draw(const GfxContext* parent_context, Assets* assets) override;

        virtual void setSupplier(doublesupplier supplier);
        virtual void setConsumer(doubleconsumer consumer);

        virtual void mouseMove(GUI*, int x, int y) override;

        virtual double getValue() const;
        virtual void setValue(double value);

        virtual double getMin() const;
        virtual void setMin(double min);

        virtual double getMax() const;
        virtual void setMax(double max);

        virtual double getStep() const;
        virtual void setStep(double step);

        virtual int getTrackWidth() const;
        virtual void setTrackWidth(int trackWidth);

        virtual glm::vec4 getTrackColor() const;
        virtual void setTrackColor(glm::vec4 trackColor);
    };

    class CheckBox : public UINode {
    protected:
        glm::vec4 hoverColor {0.05f, 0.1f, 0.2f, 0.75f};
        glm::vec4 checkColor {1.0f, 1.0f, 1.0f, 0.4f};

        boolsupplier supplier = nullptr;
        boolconsumer consumer = nullptr;

        bool checked = false;
    public:
        CheckBox(bool checked=false);

        virtual void draw(const GfxContext* parent_context, Assets* assets) override;

        virtual void mouseRelease(GUI*, int x, int y) override;

        virtual void setSupplier(boolsupplier supplier);
        virtual void setConsumer(boolconsumer consumer);

        virtual CheckBox* setChecked(bool flag);
        virtual bool isChecked() const {
            if (supplier) return supplier();
            return checked;
        }
    };

    class FullCheckBox : public Panel {
    protected:
        std::shared_ptr<CheckBox> checkbox;
    public:
        FullCheckBox(std::wstring text, glm::vec2 size, bool checked=false);

        virtual void setSupplier(boolsupplier supplier) {
            checkbox->setSupplier(supplier);
        }

        virtual void setConsumer(boolconsumer consumer) {
            checkbox->setConsumer(consumer);
        }

        virtual void setChecked(bool flag) {
            checkbox->setChecked(flag);
        }
        virtual bool isChecked() const {
            return checkbox->isChecked();
        }
    };
}

#endif // FRONTEND_GUI_CONTROLS_H_
