#ifndef GRAPHICS_UI_ELEMENTS_LABEL_H_
#define GRAPHICS_UI_ELEMENTS_LABEL_H_

#include "../UINode.h"

class Font;

namespace gui {
    struct LineScheme {
        size_t offset;
        bool fake;
    };

    struct LabelCache {
        Font* font = nullptr;
        std::vector<LineScheme> lines;
        bool resetFlag = true;
        size_t wrapWidth = -1;

        void prepare(Font* font, size_t wrapWidth);
        void update(const std::wstring& text, bool multiline, bool wrap);
    };

    class Label : public UINode {
    private:
        LabelCache cache;

        glm::vec2 calcSize();
    protected:
        std::wstring text;
        std::string fontName;
        wstringsupplier supplier = nullptr;
        float lineInterval = 1.5f;
        Align valign = Align::center;

        bool multiline = false;

        bool textWrap = true;

        bool autoresize = false;

        int textYOffset = 0;

        int totalLineHeight = 1;
    public:
        Label(const std::string& text, std::string fontName="normal");
        Label(const std::wstring& text, std::string fontName="normal");

        virtual void setText(const std::wstring& text);
        const std::wstring& getText() const;

        virtual void setFontName(std::string name);
        virtual const std::string& getFontName() const;

        virtual void setVerticalAlign(Align align);
        virtual Align getVerticalAlign() const;

        virtual float getLineInterval() const;

        virtual void setLineInterval(float interval);

        virtual int getTextYOffset() const;

        virtual int getLineYOffset(uint line) const;

        virtual size_t getTextLineOffset(size_t line) const;

        virtual uint getLineByYOffset(int offset) const;
        virtual uint getLineByTextIndex(size_t index) const;
        virtual uint getLinesNumber() const;

        virtual bool isFakeLine(size_t line) const;

        virtual void draw(const DrawContext* pctx, Assets* assets) override;

        virtual void textSupplier(wstringsupplier supplier);

        virtual void setMultiline(bool multiline);
        virtual bool isMultiline() const;

        virtual void setAutoResize(bool flag);
        virtual bool isAutoResize() const;

        virtual void setTextWrapping(bool flag);
        virtual bool isTextWrapping() const;
    };
}

#endif // GRAPHICS_UI_ELEMENTS_LABEL_H_
